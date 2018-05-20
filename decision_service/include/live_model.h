#pragma once

#include "ranking_response.h"
#include "config_collection.h"
#include "api_status.h"

namespace reinforcement_learning {

	class live_model_impl;

	//decision service client
	class live_model {

	public:
    using error_fn = void(*)(const api_status&, void*);

    explicit live_model(const utility::config_collection& config, error_fn fn = nullptr, void* err_context = nullptr);
		~live_model();

		// request the decision service, in order to rank the N actions provided in the context_json string
		int choose_rank(const char * uuid, const char * context_json, ranking_response&, api_status* = nullptr);
		int choose_rank(const char * context_json, ranking_response&, api_status* = nullptr);//uuid is auto-generated

		// report the reward for the top action
		int report_outcome(const char* uuid, const char* outcome_data, api_status* = nullptr);
		int report_outcome(const char* uuid, float reward, api_status* = nullptr);

    live_model(live_model&&) = default;
    live_model& operator=(live_model&&) = default;

	  // prevent accidental copy, since destructor will deallocate the implementation
    live_model(const live_model&) = delete;
    live_model& operator=(live_model&) = delete;

	  private:
		live_model_impl* _pimpl;
	};
}
