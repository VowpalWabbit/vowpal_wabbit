#include "online_trainer.h"
#include "trace_logger.h"

namespace online_trainer {
  online_trainer::online_trainer(const char * arg, uint8_t * model, size_t l)
  {
    m_settings = new vw_settings(arg);
    m_settings->EnableThreadSafeExamplePooling = true;
    m_settings->MaxExamples = 4 * 1024; // this value needs to be in-sync with the queue size of the deserialization part
    m_settings->EnableStringExampleGeneration = false;  // be careful when enabling this, as this slows down learning by 10x
    m_settings->EnableStringFloatCompact = false;
    if (model != nullptr) {
      m_settings->ModelStream = new memstream(model, l);
    }
  }
}
