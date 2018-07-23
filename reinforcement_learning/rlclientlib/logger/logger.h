#pragma once

#include "config_collection.h"
#include "async_batcher.h"
#include "eventhub_client.h"
#include "api_status.h"
#include "../error_callback_fn.h"


namespace reinforcement_learning {

	//this class wraps logging utilities, nicely exposed for the live_model class
	class logger {

	public:
		logger(const utility::config_collection&, error_callback_fn* perror_cb = nullptr);
    
	  int init(api_status* status);

		//log to the ranking eventhub, use a background queue to send batch
		int append_ranking(std::string&, api_status* = nullptr);

		//log to the outcome eventhub (direct sending, no batching)
		int append_outcome(std::string&, api_status* = nullptr);

	  private:
    eventhub_client _ranking_client, _outcome_client; //clients to send data to the eventhub
		async_batcher<eventhub_client> _async_batcher;    //handle batching for the data sent to the eventhub client
	};

}
