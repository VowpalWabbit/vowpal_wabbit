#include "logger.h"


namespace reinforcement_learning
{
  logger::logger(const utility::config_collection& c, error_callback_fn* perror_cb)
    : _ranking_client(
        c.get("eventhub_host", "localhost:8080"),
        c.get("shared_access_key_name", ""),
        c.get("shared_access_key", ""),
        c.get("eventhub_interaction_name", "interaction"),
        c.get_bool("local_eventhub_test",false)),
      _outcome_client(
        c.get("eventhub_host", "localhost:8080"),
        c.get("shared_access_key_name", ""),
        c.get("shared_access_key", ""),
        c.get("eventhub_observation_name", "observation"),
        c.get_bool("local_eventhub_test", false)),
      _async_batcher(
        _ranking_client,
        perror_cb,
        c.get_int("batch_max_size", 8 * 1024),
        c.get_int("batch_timeout_ms", 1000),
        c.get_int("queue_max_size", 1000 * 2))
  {
  }

  int logger::init(api_status* status) {
    auto err_code = error_code::success;
    err_code = _async_batcher.init(status);
    if ( err_code != error_code::success )
      return err_code;
    err_code = _ranking_client.init(status);
    if ( err_code != error_code::success )
      return err_code;
    err_code = _outcome_client.init(status);
    if ( err_code != error_code::success )
      return err_code;
    return err_code;
  }

  int logger::append_ranking(std::string& item, api_status* status)
  {
    //add item to the batch (will be sent later)
    return _async_batcher.append(item, status);
  }

  int logger::append_outcome(std::string& item, api_status* status)
  {
    //send to the eventhub
    return _outcome_client.send(item, status);
  }
}
