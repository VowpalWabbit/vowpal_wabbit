#pragma once
#include "ranking_response.h"
#include "err_constants.h"
#include "factory_resolver.h"

namespace reinforcement_learning {

  //// Forward declarations ////////
  class live_model_impl;          //
  class ranking_response;         //
  class api_status;               //
                                  //
  namespace model_management {    //
    class i_data_transport;       //
    class i_model;                //
  }                               //
                                  //
  namespace utility {             //
    class config_collection;      //
  }                               //
  //////////////////////////////////

	// Reinforcement learning client
	class live_model {

	public:
    // Type declarations
    using error_fn            = void(*)(const api_status&, void*);
    using transport_factory_t = utility::object_factory<model_management::i_data_transport>;
    using model_factory_t     = utility::object_factory<model_management::i_model>;

    template<typename ErrCntxt>
    using error_fn_t = void(*)( const api_status&, ErrCntxt* );

    template<typename ErrCntxt>
    explicit live_model(
      const utility::config_collection& config,
      error_fn_t<ErrCntxt> fn = nullptr,
      ErrCntxt* err_context = nullptr,
      transport_factory_t* t_factory = &data_transport_factory,
      model_factory_t* m_factory = &model_factory);

    explicit live_model(
      const utility::config_collection& config,
      error_fn fn = nullptr,
      void* err_context = nullptr,
      transport_factory_t* t_factory = &data_transport_factory,
      model_factory_t* m_factory = &model_factory);

    int init(api_status* status=nullptr);

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

    ~live_model();

  private:
		live_model_impl* _pimpl;
    bool _initialized;
	};

  template <typename ErrCtxt>
  live_model::live_model(
    const utility::config_collection& config,
    error_fn_t<ErrCtxt> fn,
    ErrCtxt* err_context,
    transport_factory_t* t_factory,
    model_factory_t* m_factory) 
  : live_model(config, (error_fn)(fn), (void*)(err_context), t_factory, m_factory) { }
}
