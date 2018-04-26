#pragma once

#include "ds_configuration.h"
#include "ds_eventhub.h"
#include "ds_async_batcher.h"


namespace decision_service {

	//this class wraps logging utilities, nicely exposed for the driver class
	class logger {

	public:
		logger(const configuration&);

		void append_ranking(const std::string&);
		void append_outcome(const std::string&);

	private:
		eventhub _ranking_eventhub, _outcome_eventhub;
		async_batcher<eventhub> _async_batcher;//layer between the caller and the eventhub that handle batching
	};

}