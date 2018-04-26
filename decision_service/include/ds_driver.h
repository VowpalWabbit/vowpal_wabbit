#pragma once

#include "ds_configuration.h"
#include "ds_ranking_response.h"


namespace decision_service {

	//decision service client
	class driver {

	public:
		static driver* create(const char* config);
		void destroy();

		//choose the top action, given a context containing user and action features
		ranking_response choose_ranking(const char* uuid, const char* context);
		ranking_response choose_ranking(const char* context);//uuid is auto-generated

		//report the reward for the top action
		void report_outcome(const char* uuid, const char* outcome_data);
		void report_outcome(const char* uuid, float reward);
	};
}
