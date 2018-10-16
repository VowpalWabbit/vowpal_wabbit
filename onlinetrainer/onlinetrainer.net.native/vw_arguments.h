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

    }
    const char* CommandLine;
  };
}
