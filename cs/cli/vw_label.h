// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw.h"
#include "vw_clr.h"
#include "cb.h"
#include "best_constant.h"
#include "constant.h"
#include "multiclass.h"

namespace VW
{
namespace Labels
{
// The label classes are a replication of the parse_label function pointers provided by individual
// modules. Main reason for creation is thread-saftey. The C++ version use a shared v_array in parser
// and thus need to be synchronized.
// These label classes are thread-safe and even more efficient as they avoid marshalling.

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Globalization;
using namespace System::Text;
using namespace CB;
using namespace MULTICLASS;
using namespace Newtonsoft::Json;

public interface class ILabel
{ void UpdateExample(vw* vw, example* ex);
  void ReadFromExample(example* ex);
};

public ref class ContextualBanditLabel sealed : ILabel
{
private:
  uint32_t m_action;
  float m_cost;
  float m_probability;

public:
  ContextualBanditLabel()
    : m_action(0), m_cost(0), m_probability(0)
  { }

  ContextualBanditLabel(uint32_t action, float cost, float probability)
    : m_action(action), m_cost(cost), m_probability(0)
  { Probability = probability;
  }

  [JsonProperty]
  property uint32_t Action
  { uint32_t get()
    { return m_action;
    }

    void set(uint32_t value)
    { m_action = value;
    }
  }

  [JsonProperty]
  property float Probability
  { float get()
    { return m_probability;
    }

    void set(float value)
    { if (value < 0 || value >1)
      {
        if (value > 1 && value - 1 < probability_tolerance)
          m_probability = 1.0f;
        else
          throw gcnew ArgumentOutOfRangeException("invalid probability: " + value);
      }
      else
        m_probability = value;
    }
  }

  [JsonProperty]
  property float Cost
  { float get()
    { return m_cost;
    }

    void set(float value)
    { m_cost = value;
    }
  }

  [JsonIgnore]
  property bool IsShared
  { bool get()
    { return m_cost == FLT_MAX && m_probability == -1.f;
    }
  }

  virtual void ReadFromExample(example* ex)
  { CB::label* ld = (CB::label*)&ex->l;
    if (ld->costs.size() > 0)
    { cb_class& f = ld->costs[0];

      m_action = f.action;
      m_cost = f.cost;
      m_probability = f.probability;
    }
  }

  virtual void UpdateExample(vw* vw, example* ex)
  { CB::label* ld = (CB::label*)&ex->l;
    cb_class f;

    f.partial_prediction = 0.;
    f.action = m_action;
    f.cost = m_cost;
    f.probability = m_probability;

    ld->costs.push_back(f);
  }

  virtual String^ ToString() override
  { auto sb = gcnew StringBuilder;

    sb->Append(m_action.ToString(CultureInfo::InvariantCulture));
    sb->Append(L':');
    sb->Append(m_cost.ToString(CultureInfo::InvariantCulture));
    sb->Append(L':');
    sb->Append(m_probability.ToString(CultureInfo::InvariantCulture));

    return sb->ToString();
  }
};

/// <summary>
/// In multi-line scenarios the first example can contain a set of shared features. This first example must be
/// marked using a 'shared' label.
/// </summary>
public ref class SharedLabel sealed : ILabel
{
private:
  uint32_t m_action;

  SharedLabel() : m_action((uint32_t)uniform_hash("shared", 6, 0))
  { }

public:
  static SharedLabel^ Instance = gcnew SharedLabel;

  virtual void UpdateExample(vw* vw, example* ex)
  { CB::label* ld = (CB::label*)&ex->l;
    cb_class f;

    f.partial_prediction = 0.;
    f.action = m_action;
    f.cost = FLT_MAX;
    f.probability = -1.f;

    ld->costs.push_back(f);
  }

  virtual String^ ToString() override
  { return "shared";
  }

  virtual void ReadFromExample(example* ex)
  {
  }
};

public ref class SimpleLabel sealed : ILabel
{
private:
  float m_label;

  Nullable<float> m_weight;

  Nullable<float> m_initial;

public:
  SimpleLabel() : m_label(0)
  { }

  [JsonProperty]
  property float Label
  { float get()
    { return m_label;
    }

    void set(float value)
    { m_label = value;
    }
  }

  [JsonProperty(NullValueHandling = NullValueHandling::Ignore)]
  property Nullable<float> Weight
  { Nullable<float> get()
    { return m_weight;
    }

    void set(Nullable<float> value)
    { m_weight = value;
    }
  }

  [JsonProperty(NullValueHandling = NullValueHandling::Ignore)]
  property Nullable<float> Initial
  { Nullable<float> get()
    { return m_initial;
    }

    void set(Nullable<float> value)
    { m_initial = value;
    }
  }

  virtual void ReadFromExample(example* ex)
  { label_data* ld = (label_data*)&ex->l;

    m_label = ld->label;
    m_weight = ld->weight;
    m_initial = ld->initial;
  }

  virtual void UpdateExample(vw* vw, example* ex)
  { label_data* ld = (label_data*)&ex->l;
    ld->label = m_label;

    if (m_weight.HasValue)
      ld->weight = m_weight.Value;

    if (m_initial.HasValue)
      ld->initial = m_initial.Value;

    count_label(vw->sd, ld->label);
  }

  virtual String^ ToString() override
  { auto sb = gcnew StringBuilder;

    sb->Append(m_label.ToString(CultureInfo::InvariantCulture));

    if (m_weight.HasValue)
    { sb->Append(L' ');
      sb->Append(m_weight.Value.ToString(CultureInfo::InvariantCulture));

      if (m_initial.HasValue)
      { sb->Append(L' ');
        sb->Append(m_initial.Value.ToString(CultureInfo::InvariantCulture));
      }
    }

    return sb->ToString();
  }
};

public ref class MulticlassLabel sealed : ILabel
{
public:
  ref class Label sealed
  {
  private:
    uint32_t m_class;
    Nullable<float> m_weight;

  public:
    property uint32_t Class
    { uint32_t get()
      { return m_class;
      }

      void set(uint32_t value)
      { m_class = value;
      }
    }

    [JsonProperty(NullValueHandling = NullValueHandling::Ignore)]
    property Nullable<float> Weight
    { Nullable<float> get()
      { return m_weight;
      }

      void set(Nullable<float> value)
      { m_weight = value;
      }
    }
  };

private:
  List<Label^>^ m_classes;

public:
  [JsonProperty]
  property List<Label^>^ Classes
  { List<Label^>^ get()
    { return m_classes;
    }

    void set(List<Label^>^ value)
    { m_classes = value;
    }
  }

  virtual void ReadFromExample(example* ex)
  { throw gcnew NotImplementedException("to be done...");
  }

  virtual void UpdateExample(vw* vw, example* ex)
  { throw gcnew NotImplementedException("to be done...");
  }

  virtual String^ ToString() override
  { auto sb = gcnew StringBuilder;

    for each (Label^ label in m_classes)
    { sb->Append(L' ');
      sb->Append(label->Class.ToString(CultureInfo::InvariantCulture));

      if (label->Weight.HasValue)
      { sb->Append(L' ');
        sb->Append(label->Weight.Value.ToString(CultureInfo::InvariantCulture));
      }
    }

    // strip first space
    if (sb->Length > 0)
      sb->Remove(0, 1);

    return sb->ToString();
  }
};

public ref class StringLabel sealed : ILabel
{
private:
  String^ m_label;

public:
  StringLabel()
  { }

  StringLabel(String^ label) : m_label(label)
  { }

  [JsonProperty]
  property String^ Label
  { String^ get()
    { return m_label;
    }

    void set(String^ value)
    { m_label = value;
    }
  }

  virtual void ReadFromExample(example* ex)
  { throw gcnew NotImplementedException("to be done...");
  }

  virtual void UpdateExample(vw* vw, example* ex)
  { auto bytes = System::Text::Encoding::UTF8->GetBytes(m_label);
    auto valueHandle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

    try
    { VW::parse_example_label(*vw, *ex, reinterpret_cast<char*>(valueHandle.AddrOfPinnedObject().ToPointer()));
    }
    CATCHRETHROW
    finally
    { valueHandle.Free();
    }
  }

  virtual String^ ToString() override
  { return m_label;
  }
};
}
}
