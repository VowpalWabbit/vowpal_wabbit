/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include <msclr\marshal_cppstd.h>
#include "vw.h"
#include <algorithm>

using namespace std;
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
  { po::variables_map& vm = vw->vm;
    if (vm.count("initial_regressor") || vm.count("i"))
    { m_regressors = gcnew List<String^>;

      vector<string> regs = vm["initial_regressor"].as< vector<string> >();
      for (auto& r : regs)
        m_regressors->Add(gcnew String(r.c_str()));
    }

    StringBuilder^ sb = gcnew StringBuilder();
    for (auto& s : vw->args)
      sb->AppendFormat("{0} ", gcnew String(s.c_str()));

    m_commandLine = sb->ToString()->TrimEnd();

    if (vw->vm.count("cb"))
      m_numberOfActions = (int)vw->vm["cb"].as<size_t>();

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
