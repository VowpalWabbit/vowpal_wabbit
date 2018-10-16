#include "io_buf.h"

namespace online_trainer {
  class memstream;
  class vw_io_buf : public io_buf {
  public:
    vw_io_buf(memstream* stream);
  };
}
