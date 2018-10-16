#include "vw_base.h"
#include "vw_model.h"
#include <cstring>

namespace online_trainer {
  vw_base::vw_base(vw_settings* settings) : m_vw(nullptr), m_model(nullptr), m_settings(settings), m_instanceCount(0)
  {
    try {
      std::string string;
      //if (settings->Arguments != nullptr)
      //  string = msclr::interop::marshal_as<std::string>(settings->Arguments);

      if (settings->Model != nullptr)
      {
        m_model = settings->Model;
        if (!settings->Verbose && strstr(settings->Arguments,"--quiet") == nullptr  && strstr(m_model->Arguments->CommandLine, "--quiet") == nullptr)
          string.append(" --quiet");
        m_vw = VW::seed_vw_model(m_model->m_vw, string);
        // m_model->IncrementReference();
      }
      else
      {
        if (settings->ModelStream == nullptr)
        {
          if (!settings->Verbose && strstr(settings->Arguments, "--quiet") == nullptr)
            string.append(" --quiet");

          m_vw = VW::initialize(string, nullptr, false);
        }
        else
        {
          vw_io_buf model(settings->ModelStream);
          if (strstr(settings->Arguments, "--no_stdin") == nullptr)
            string += " --no_stdin";
          m_vw = VW::initialize(string, &model, false);
          delete settings->ModelStream;
          settings->ModelStream = nullptr;
        }
      }
    }
    catch (...)
    { // memory leak, but better than crashing
      m_vw = nullptr;
      throw;
    }
  }
}
