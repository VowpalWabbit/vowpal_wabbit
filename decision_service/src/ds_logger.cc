#include "ds_logger.h"


namespace decision_service {

	logger::logger(const configuration& c)
		: _ranking_eventhub(c.eventhub_host(), c.shared_access_key_name(), c.shared_access_key(), c.eventhub_interaction_name()),
		_outcome_eventhub(c.eventhub_host(), c.shared_access_key_name(), c.shared_access_key(), c.eventhub_observation_name()),
		_async_batcher(_ranking_eventhub, c.batch_max_size(), c.batch_timeout_ms(), c.queue_max_size())
	{}

	void logger::append_ranking(const std::string & item)
	{
		//add item to the batch (will be sent later)
		_async_batcher.append(item);
	}

	void logger::append_outcome(const std::string & item)
	{
		//send to the eventhub
		_outcome_eventhub.send(item);
	}

}