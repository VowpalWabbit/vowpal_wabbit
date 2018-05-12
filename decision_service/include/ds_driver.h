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
    using error_fn = void(*)(const api_status&, void*);

    explicit driver(const utility::config_collection& config, error_fn fn = nullptr, void* err_context = nullptr);
		~driver();

		// request the decision service, in order to rank the N actions provided in the context_json string
		int choose_rank(const char * uuid, const char * context_json, ranking_response&, api_status* = nullptr);
		int choose_rank(const char * context_json, ranking_response&, api_status* = nullptr);//uuid is auto-generated

		// report the reward for the top action
		int report_outcome(const char* uuid, const char* outcome_data, api_status* = nullptr);
		int report_outcome(const char* uuid, float reward, api_status* = nullptr);

    driver(driver&&) = default;
    driver& operator=(driver&&) = default;

	  // prevent accidental copy, since destructor will deallocate the implementation
    driver(const driver&) = delete;
    driver& operator=(driver&) = delete;

	  private:
		driver_impl* _pimpl;
	};
}
