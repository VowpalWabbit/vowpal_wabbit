#pragma once

#include "ds_ranking_response.h"
#include "ds_config_collection.h"
#include <memory>

namespace decision_service {
  class driver_impl;

	//decision service client
	class driver {

	public:
    explicit driver(const utility::config_collection& config);
    ~driver();

	  //choose the top action, given a context containing user and action features
		ranking_response choose_ranking(const char* uuid, const char* context);
		ranking_response choose_ranking(const char* context);//uuid is auto-generated

		//report the reward for the top action
		void report_outcome(const char* uuid, const char* outcome_data);
		void report_outcome(const char* uuid, float reward);
	private:
	  std::unique_ptr<driver_impl> _pimpl;
	};
}
