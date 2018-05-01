#include "ds_logger.h"


namespace decision_service {

	logger::logger(const utility::config_collection& c)
		: _ranking_eventhub(
      c.get("eventhub_host", "localhost:8080"),
      c.get("shared_access_key_name",""), 
      c.get("shared_access_key",""), 
      c.get("eventhub_interaction_name","interaction")),
		_outcome_eventhub(
      c.get("eventhub_host", "localhost:8080"),
      c.get("shared_access_key_name",""), 
      c.get("shared_access_key",""), 
      c.get("eventhub_observation_name","observation")),
		_async_batcher(
      _ranking_eventhub, 
      c.get_int("batch_max_size",8*1024), 
      c.get_int("batch_timeout_ms",10000), 
      c.get_int("queue_max_size",1000*2))
	{}

	void logger::append_ranking(const std::string & item)
	{
		//add item to batch (will be sent later)
		_async_batcher.append(item);
	}

	void logger::append_outcome(const std::string & item)
	{
		//send now to the eventhub
		_outcome_eventhub.send(item);
	}

}