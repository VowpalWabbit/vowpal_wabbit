// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw_clr.h"
#include "vw_interface.h"
#include "vw_labelcomparator.h"
#include "vw_label.h"

namespace VW
{
using namespace System::Collections::Generic;
using namespace VW::Labels;

ref class VowpalWabbitExample;
ref class VowpalWabbit;

[System::Diagnostics::DebuggerDisplay("{m_weight_index}:{m_x}")]
public ref struct VowpalWabbitFeature
{
private:
  feature_value m_x;
  uint64_t m_weight_index;
  VowpalWabbitExample^ m_example;
  VowpalWabbit^ m_vw;

public:
  VowpalWabbitFeature(VowpalWabbitExample^ example, feature_value x, uint64_t weight_index);
  VowpalWabbitFeature(VowpalWabbit^ vw, feature_value x, uint64_t weight_index);

  property feature_value X
  { float get();
  }

  property uint64_t FeatureIndex
  { uint64_t get();
  }

  property uint64_t WeightIndex
  { uint64_t get();
  }

  property float Weight
  { float get();
  }

  property float AuditWeight
  { float get();
  }

  virtual bool Equals(Object^ o) override;

  virtual int GetHashCode() override;
};

template <typename T>
struct Holder
{ T value;
};

[System::Diagnostics::DebuggerDisplay("{Index} = '{(char)Index}'")]
public ref struct VowpalWabbitNamespace : public IEnumerable<VowpalWabbitFeature^>
{
private:
  ref class FeatureEnumerator : public IEnumerator<VowpalWabbitFeature^>
  {
  private:
    VowpalWabbitExample^ m_example;
    features* m_features;
    Holder<features::iterator>* m_iterator;
    Holder<features::iterator>* m_end;

  internal:
    FeatureEnumerator(VowpalWabbitExample^ example, features* features);
    ~FeatureEnumerator();

    property System::Object^ IEnumeratorCurrent
    { virtual System::Object^ get() sealed = System::Collections::IEnumerator::Current::get;
    }

  public:
    virtual bool MoveNext();

    virtual void Reset();

    property VowpalWabbitFeature^ Current
    { virtual VowpalWabbitFeature^ get();
    }
  };

  namespace_index m_ns;
  features* m_features;
  VowpalWabbitExample^ m_example;

  property System::Collections::IEnumerator^ EnumerableGetEnumerator
  { virtual System::Collections::IEnumerator^ get() sealed = System::Collections::IEnumerable::GetEnumerator;
  }

public:
  VowpalWabbitNamespace(VowpalWabbitExample^ m_example, namespace_index ns, features* features);
  ~VowpalWabbitNamespace();

  property namespace_index Index
  { namespace_index get();
  }


  virtual IEnumerator<VowpalWabbitFeature^>^ GetEnumerator();
};

/// <summary>
/// A CLR representation of a vowpal wabbit example.
/// </summary>
/// <remarks>
/// Underlying memory is allocated by native code, but examples are not part of the ring.
/// </remarks>
[System::Diagnostics::DebuggerDisplay("{m_example}: '{m_string}'")]
public ref class VowpalWabbitExample : public IEnumerable<VowpalWabbitNamespace^>
{
private:
  /// <summary>
  /// Reference to an optional underlying example.
  /// </summary>
  /// <remarks>If this instance owns <see name="m_example"/> this is null.</remarks>
  initonly VowpalWabbitExample^ m_innerExample;

  ref class NamespaceEnumerator : public IEnumerator<VowpalWabbitNamespace^>
  {
  private:
    VowpalWabbitExample^ m_example;
    namespace_index* m_current;

  internal:
    NamespaceEnumerator(VowpalWabbitExample^ example);
    ~NamespaceEnumerator();

    property System::Object^ IEnumeratorCurrent
    { virtual System::Object^ get() sealed = System::Collections::IEnumerator::Current::get;
    }

  public:
    virtual bool MoveNext();

    virtual void Reset();

    property VowpalWabbitNamespace^ Current
    { virtual VowpalWabbitNamespace^ get();
    }
  };

protected:
  /// <summary>
  /// Returns native example data structure to owning instance.
  /// </summary>
  !VowpalWabbitExample();

internal:
  /// <summary>
  /// Initializes a new instance of <see cref="VowpalWabbitExample"/>.
  /// </summary>
  /// <param name="owner">The parent instance. Examples cannot be shared between vw instances.</param>
  /// <param name="example">The already allocated example structure</param>
  VowpalWabbitExample(IVowpalWabbitExamplePool^ owner, example* example);

  /// <summary>
  /// The native example data structure.
  /// </summary>
  example* m_example;

  /// <summary>
  /// The owner of this example.
  /// </summary>
  IVowpalWabbitExamplePool^ m_owner;

  /// <summary>
  /// The optional string version of the example.
  /// </summary>
  String^ m_string;

public:
  /// <summary>
  /// Initializes a new instance of <see cref="VowpalWabbitExample"/>.
  /// </summary>
  /// <param name="owner">The parent instance. Examples cannot be shared between <see cref="IVowpalWabbitExamplePool"/> instances.</param>
  /// <param name="example">The inner example this instance wraps.</param>
  VowpalWabbitExample(IVowpalWabbitExamplePool^ owner, VowpalWabbitExample^ example);

  /// <summary>
  /// Returns native example data structure to owning pool.
  /// </summary>
  ~VowpalWabbitExample();

  /// <summary>
  /// Extracts the prediction from this example using the given prediction factory.
  /// </summary>
  /// <returns>The prediction stored in this example.</returns>
  generic<typename T> T GetPrediction(VowpalWabbit^ vw, IVowpalWabbitPredictionFactory<T>^ factory);

  /// <summary>
  /// An optional inner example this example wraps.
  /// </summary>
  property VowpalWabbitExample^ InnerExample
  { VowpalWabbitExample^ get();
  }

  /// <summary>
  /// The owner of this example.
  /// </summary>
  property IVowpalWabbitExamplePool^ Owner
  { IVowpalWabbitExamplePool^ get();
  }

  /// <summary>
  /// The corresponding VowpalWabbitString for this example.
  /// </summary>
  property String^ VowpalWabbitString
  { String^ get();
    void set(String^ value);
  }

  /// <summary>
  /// True if this is a new line example, otherwise false.
  /// </summary>
  /// <remarks>A example without features is considered a new line example.</remarks>
  property bool IsNewLine
  { bool get();
  }

  String^ Diff(VowpalWabbit^ vw, VowpalWabbitExample^ other, IVowpalWabbitLabelComparator^ labelComparator);

  void MakeEmpty(VowpalWabbit^ vw);

  property System::Collections::IEnumerator^ EnumerableGetEnumerator
  { virtual System::Collections::IEnumerator^ get() sealed = System::Collections::IEnumerable::GetEnumerator;
  }

  virtual IEnumerator<VowpalWabbitNamespace^>^ GetEnumerator();

  property size_t NumberOfFeatures
  { size_t get();
  }

  property ILabel^ Label
  { ILabel^ get();
    void set(ILabel^ label);
  }
};
}
