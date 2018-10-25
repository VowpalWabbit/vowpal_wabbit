#pragma once

#include <string>
#include <vector>

namespace reinforcement_learning {
  class api_status;

  class i_sender {
  public:
    virtual int init(api_status* status) = 0;

    int send_byte(std::vector<unsigned char>& data, api_status* status = nullptr)
    {
      return v_send(data, status);
    }

    int send_string(std::string &&data, api_status* status = nullptr)
    {
      return v_send(std::move(data), status);
    }

    virtual ~i_sender() = default;

  protected:
    virtual int v_send(std::vector<unsigned char>& data, api_status* status) { return 0; };
    virtual int v_send(std::string&& data, api_status* status) { return 0; };
  };
}
