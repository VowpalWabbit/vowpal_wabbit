#pragma once

namespace vw_lib {

  class i_trace;
  class api_status;
  class i_online_trainer {
  public:
    virtual int init(api_status* api_status) = 0;
    virtual ~i_online_trainer() = default;
  };

  class vw_online_trainer : public i_online_trainer {
  public:
    int init(api_status* api_status) override;
  private:
    i_trace * _trace_logger;
  };
}
