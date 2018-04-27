#pragma once

#include "ds_configuration.h"
#include "ds_eventhub_client.h"
#include "ds_async_batcher.h"


namespace decision_service {

	//this class wraps logging utilities, nicely exposed for the driver class
	class logger {

	public:
		logger(const configuration&);

		//log to the ranking eventhub, use a background queue to send batch
		void append_ranking(const std::string&);

		//log to the outcome eventhub (direct sending, no batching)
		void append_outcome(const std::string&);

	private:
		eventhub_client _ranking_client, _outcome_client; //clients to send data to the eventhub
		async_batcher<eventhub_client> _async_batcher;    //handle batching for the data sent to the eventhub client
	};

}