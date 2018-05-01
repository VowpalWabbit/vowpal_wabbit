#pragma once

#include "ds_eventhub.h"
#include "ds_async_batch.h"
#include "ds_config_collection.h"


namespace decision_service {

	class logger {

	public:
		logger(const utility::config_collection&);

		void append_ranking(const std::string&);
		void append_outcome(const std::string&);

	private:
		eventhub _ranking_eventhub, _outcome_eventhub;
		async_batch<eventhub> _async_batcher;//layer between the caller and the eventhub that handle batching
	};

}
