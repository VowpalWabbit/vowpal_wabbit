#pragma once

#include <string>

namespace reinforcement_learning {
  class api_status;

  class i_sender {
  public:
    virtual int init(api_status* status) = 0;

    //! In order to avoid having default parameters in the pure virtual function, wrap it in this call.
    int send(const std::string& data, api_status* status = nullptr)
    {
      return v_send(data, status);
    }

    virtual ~i_sender() = default;
  
  protected:
    virtual int v_send(const std::string& data, api_status* status) = 0;
  };
}
