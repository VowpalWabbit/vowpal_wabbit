#pragma once

#include "vw.h"

namespace online_trainer {
  class vw_arguments {
  private:
    const char* m_data;
    const char* m_finalRegressor;
    const bool m_testonly;
    const int m_passes;
    const char* m_commandLine;
  public:
    vw_arguments(vw* vw):
      m_data(vw->data_filename.c_str()),
      m_finalRegressor(vw->final_regressor_name.c_str()),
      m_testonly(!vw->training),
      m_passes((int)vw->numpasses)
    {
      po::variables_map& vm = vw->opts_n_args.vm;
     /* if (vm.count("initial_regressor") || vm.count("i"))
      {
        m_regressors = gcnew List<String^>;

        vector<string> regs = vm["initial_regressor"].as< vector<string> >();
        for (auto& r : regs)
          m_regressors->Add(gcnew String(r.c_str()));
      }

      StringBuilder^ sb = gcnew StringBuilder();
      for (auto& s : vw->opts_n_args.args)
        sb->AppendFormat("{0} ", gcnew String(s.c_str()));

      m_commandLine = sb->ToString()->TrimEnd();

      if (vw->opts_n_args.vm.count("cb"))
        m_numberOfActions = (int)vw->opts_n_args.vm["cb"].as<uint32_t>();

      m_learning_rate = vw->eta;
      m_power_t = vw->power_t;*/
    }
    const char* CommandLine;
  };
}
