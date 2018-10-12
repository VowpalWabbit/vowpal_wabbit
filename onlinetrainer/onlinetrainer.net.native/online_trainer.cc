#include "online_trainer.h"
#include "trace_logger.h"

namespace online_trainer {
  online_trainer::online_trainer(const char * arg)
  {
    m_settings = new vw_settings(arg);
  }
}
