#include "ds_logger.h"


namespace decision_service {

	logger::logger(const utility::config_collection& c, error_callback_fn* perror_cb = nullptr)
		: _ranking_client(
      c.get("eventhub_host", "localhost:8080"),
      c.get("shared_access_key_name",""), 
      c.get("shared_access_key",""), 
      c.get("eventhub_interaction_name","interaction")),
		_outcome_client(
      c.get("eventhub_host", "localhost:8080"),
      c.get("shared_access_key_name",""), 
      c.get("shared_access_key",""), 
      c.get("eventhub_observation_name","observation")),
		_async_batcher(
      _ranking_client,
      perror_cb,
      c.get_int("batch_max_size",8*1024), 
      c.get_int("batch_timeout_ms",1000),
      c.get_int("queue_max_size",1000*2))
	{}

	int logger::append_ranking(const std::string & item, api_status* status)
	{
		//add item to the batch (will be sent later)
		return _async_batcher.append(item, status);
	}

	int logger::append_outcome(const std::string & item, api_status* status)
	{
		//send to the eventhub
		return _outcome_client.send(item, status);
	}
}
