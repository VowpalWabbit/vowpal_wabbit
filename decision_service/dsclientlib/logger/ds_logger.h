#pragma once

#include "ds_config_collection.h"
#include "ds_async_batcher.h"
#include "ds_eventhub_client.h"
#include "ds_api_status.h"
#include "../ds_error_callback_fn.h"


namespace decision_service {

	//this class wraps logging utilities, nicely exposed for the driver class
	class logger {

	public:
		logger(const utility::config_collection&, error_callback_fn* perror_cb);

		//log to the ranking eventhub, use a background queue to send batch
		int append_ranking(const std::string&, api_status* = nullptr);

		//log to the outcome eventhub (direct sending, no batching)
		int append_outcome(const std::string&, api_status* = nullptr);

	private:
    eventhub_client _ranking_client, _outcome_client; //clients to send data to the eventhub
		async_batcher<eventhub_client> _async_batcher;    //handle batching for the data sent to the eventhub client
	};

}
