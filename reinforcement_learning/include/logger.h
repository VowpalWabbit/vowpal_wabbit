#pragma once

#include "config_collection.h"
#include "constants.h"

#include <memory>

namespace reinforcement_learning {
  class api_status;
  class error_callback_fn;
  class event_hub_logger_impl;

  class logger_i {
  public:
    virtual int init(api_status* status) = 0;

    // In order to avoid having default parameters in the pure virtual function, wrap it in this call.
    int append(std::string& data, api_status* status = nullptr)
    {
      return v_append(data, status);
    }

    protected:
      virtual int v_append(std::string& data, api_status* status) = 0;
  };

  // This class wraps logging utilities, nicely exposed for the live_model class.
  class event_hub_logger : public logger_i {
  public:
    event_hub_logger(
      const utility::config_collection&,
      const std::string& event_hub_host,
      const std::string& event_hub_key_name,
      const std::string& event_hub_key,
      const std::string& event_hub_name,
      int send_high_watermark,
      int send_batch_interval_ms,
      int send_queue_maxsize,
      error_callback_fn* perror_cb = nullptr);

    int init(api_status* status) override;

    ~event_hub_logger();

  protected:
    virtual int v_append(std::string& data, api_status* status) override;

  private:
    std::unique_ptr<event_hub_logger_impl> _pimpl;
  };

  class event_hub_observation_logger : public event_hub_logger {
  public:
    event_hub_observation_logger(const utility::config_collection& c, error_callback_fn* perror_cb = nullptr)
      : event_hub_logger(
        c,
        c.get(name::OBSERVATION_EH_HOST, "localhost:8080"),
        c.get(name::OBSERVATION_EH_KEY_NAME, ""),
        c.get(name::OBSERVATION_EH_KEY, ""),
        c.get(name::OBSERVATION_EH_NAME, "interaction"),
        c.get_int(name::OBSERVATION_SEND_HIGH_WATER_MARK, 198 * 1024),
        c.get_int(name::OBSERVATION_SEND_BATCH_INTERVAL_MS, 1000),
        c.get_int(name::OBSERVATION_SEND_QUEUE_MAXSIZE, 10000 * 2),
        perror_cb)
    {}
  };

  class event_hub_interaction_logger : public event_hub_logger {
  public:
    event_hub_interaction_logger(const utility::config_collection& c, error_callback_fn* perror_cb = nullptr)
      : event_hub_logger(
        c,
        c.get(name::INTERACTION_EH_HOST, "localhost:8080"),
        c.get(name::INTERACTION_EH_KEY_NAME, ""),
        c.get(name::INTERACTION_EH_KEY, ""),
        c.get(name::INTERACTION_EH_NAME, "interaction"),
        c.get_int(name::INTERACTION_SEND_HIGH_WATER_MARK, 198 * 1024),
        c.get_int(name::INTERACTION_SEND_BATCH_INTERVAL_MS, 1000),
        c.get_int(name::INTERACTION_SEND_QUEUE_MAXSIZE, 10000 * 2),
        perror_cb)
    {}
  };
}
