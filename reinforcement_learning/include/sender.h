#pragma once

#include <string>
#include <vector>

namespace reinforcement_learning {
  class api_status;

  class i_sender {
  public:
    virtual int init(api_status* status) = 0;

    int send_byte(const std::vector<unsigned char> &data, api_status* status = nullptr)
    {
      return v_send(data, status);
    }

    int send_string(const std::string &data, api_status* status = nullptr)
    {
      return v_send(data, status);
    }

    virtual ~i_sender() = default;

  protected:
    virtual int v_send(const std::vector<unsigned char> &data, api_status* status) { return 0; };
    virtual int v_send(const std::string& data, api_status* status) { return 0; };
  };
}
