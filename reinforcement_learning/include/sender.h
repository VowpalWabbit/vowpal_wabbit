#pragma once

#include <string>
#include <vector>

namespace reinforcement_learning {
  class api_status;

  class i_sender {
  public:
    virtual int init(api_status* status) = 0;

    // TODO: Refactor this method assume string data, instead of binary format.
    //! In order to avoid having default parameters in the pure virtual function, wrap it in this call.
    int send(const std::string& data, api_status* status = nullptr)
    {
      return v_send(data, status);
    }

    int send(std::vector<char> data, api_status* status = nullptr)
    {
      return v_send(data, status);
    }

    virtual ~i_sender() = default;

  protected:
    // TODO: Refactor this method assume string data, instead of binary format.
    virtual int v_send(const std::string& data, api_status* status) = 0;

    virtual int v_send(std::vector<char> data, api_status* status) { return 0; };
  };
}
