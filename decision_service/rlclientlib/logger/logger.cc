#include "logger.h"
#include "err_constants.h"
<<<<<<< HEAD
#include "constants.h"
=======
>>>>>>> master

namespace reinforcement_learning
{
  logger::logger(const utility::config_collection& c, error_callback_fn* perror_cb)
    : _ranking_client(
        c.get(name::INTERACTION_EH_HOST     , "localhost:8080"),
        c.get(name::INTERACTION_EH_KEY_NAME , ""),
        c.get(name::INTERACTION_EH_KEY      , ""),
        c.get(name::INTERACTION_EH_NAME     , "interaction"),
        c.get_bool(name::EH_TEST            ,false)),
      _outcome_client(
        c.get(name::OBSERVATION_EH_HOST     , "localhost:8080"),
        c.get(name::OBSERVATION_EH_KEY_NAME , ""),
        c.get(name::OBSERVATION_EH_KEY      , ""),
        c.get(name::OBSERVATION_EH_NAME     , "observation"),
        c.get_bool(name::EH_TEST            ,false)),
      _async_batcher(
        _ranking_client,
        perror_cb,
        c.get_int(name::SEND_HIGH_WATER_MARK, 4 * 1024 * 1024),
        c.get_int(name::SEND_BATCH_INTERVAL , 1000),
        c.get_int(name::SEND_QUEUE_MAXSIZE  , 1000 * 2))
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
