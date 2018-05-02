#pragma once

#include "ds_ranking_response.h"
#include "ds_config_collection.h"
#include "ds_api_status.h"
#include <memory>

namespace decision_service {

	class driver_impl;

	//decision service client
	class driver {

	public:
		explicit driver(const utility::config_collection& config);
		~driver();

		//request the decision service, in order to rank the N actions provided in the context_json string
		int ranking_request(const char * uuid, const char * context_json, ranking_response&, api_status* = nullptr);
		int ranking_request(const char * context_json, ranking_response&, api_status* = nullptr);//uuid is auto-generated

		//report the reward for the top action
		int report_outcome(const char* uuid, const char* outcome_data, api_status* = nullptr);
		int report_outcome(const char* uuid, float reward, api_status* = nullptr);

	private:
		std::unique_ptr<driver_impl> _pimpl;
	};
}
