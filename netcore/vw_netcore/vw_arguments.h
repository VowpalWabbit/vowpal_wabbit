// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <msclr\marshal_cppstd.h>
#include "vw.h"
#include "options_serializer_boost_po.h"
#include <algorithm>

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;

namespace VW
{
/// <summary>
/// command line arguments extracted from native C++.
/// </summary>
public ref class VowpalWabbitArguments
{
private:
  initonly String^ m_data;
  initonly String^ m_finalRegressor;
  const bool m_testonly;
  const int m_passes;
  List<String^>^ m_regressors;
  String^ m_commandLine;

  int m_numberOfActions;
  float m_learning_rate;
  float m_power_t;

internal:
  VowpalWabbitArguments(vw* vw) :
    m_data(gcnew String(vw->data_filename.c_str())),
    m_finalRegressor(gcnew String(vw->final_regressor_name.c_str())),
    m_testonly(!vw->training),
    m_passes((int)vw->numpasses)
  {
    auto options = vw->options;

    if (vw->initial_regressors.size() > 0)
    { m_regressors = gcnew List<String^>;

      for (auto& r : vw->initial_regressors)
        m_regressors->Add(gcnew String(r.c_str()));
    }

    VW::config::options_serializer_boost_po serializer;
    for (auto const& option : options->get_all_options())
    {
      if (options->was_supplied(option->m_name))
      {
        serializer.add(*option);
      }
    }

    auto serialized_keep_options = serializer.str();
    m_commandLine = gcnew String(serialized_keep_options.c_str());

    if (options->was_supplied("cb")) {
      m_numberOfActions = (int)options->get_typed_option<uint32_t>("cb").value();
    }

	m_learning_rate = vw->eta;
	m_power_t = vw->power_t;
  }

public:
  /// <summary>
  /// The input data file.
  /// </summary>
  property String^ Data
  { String^ get()
    { return m_data;
    }
  }

  /// <summary>
  /// True if "-t" for test only mode supplied as part of arguments.
  /// </summary>
  property bool TestOnly
  { bool get()
    { return m_testonly;
    }
  }

  /// <summary>
  /// Number of passes.
  /// </summary>
  property int NumPasses
  { int get()
    { return m_passes;
    }
  }

  /// <summary>
  /// The output model filename.
  /// </summary>
  property String^ FinalRegressor
  { String^ get()
    { return m_finalRegressor;
    }
  }

  /// <summary>
  ///The list of input model filenames.
  /// </summary>
  property List<String^>^ InitialRegressors
  { List<String^>^ get()
    { return m_regressors;
    }
  }

  property String^ CommandLine
  { String^ get()
    { return m_commandLine;
    }
  }

  property int ContextualBanditNumberOfActions
  { int get()
    { return m_numberOfActions;
    }
  }

  property float LearningRate
  {
	  float get()
	  {
		  return m_learning_rate;
	  }
  }

  property float PowerT
  {
	  float get()
	  {
		  return m_power_t;
	  }
  }
};
}
