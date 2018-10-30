#include "vw_model.h"
#include "err_constants.h"
#include "object_factory.h"
#include "explore.h"
#include "ranking_response.h"
#include "trace_logger.h"
#include "str_util.h"

namespace e = exploration;
namespace reinforcement_learning { namespace model_management {

  vw_model::vw_model(i_trace* trace_logger) :
    _vw_pool(nullptr) , _trace_logger(trace_logger) {
  }

  int vw_model::update(const model_data& data, api_status* status) {
    try {
      TRACE_INFO(_trace_logger, utility::concat("Recieved new model data. With size ", data.data_sz()));
      
      // safe_vw_factory will create a copy of the model data to use for vw object construction.
      _vw_pool.update_factory(new safe_vw_factory(std::move(data)));
    }
    catch(const std::exception& e) {
      RETURN_ERROR_LS(_trace_logger, status, model_update_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(_trace_logger, status, model_update_error) << "Unkown error";
    }

    return error_code::success;
  }

  int vw_model::choose_rank(uint64_t rnd_seed, const char* features, ranking_response& response, api_status* status) {
    try {
      pooled_vw vw(_vw_pool, _vw_pool.get_or_create());

      // Rank actions using the model.  Should generate a pdf
      std::vector<int> action_ids;
      std::vector<float> pdf;

      // Get a ranked list of action_ids and corresponding pdf
      vw->rank(features, action_ids, pdf);

      // Pick a slot using the pdf. NOTE: sample_after_normalizing() can change the pdf
      uint32_t chosen_index;
      auto scode = e::sample_after_normalizing(rnd_seed, std::begin(pdf), std::end(pdf), chosen_index);

      if ( S_EXPLORATION_OK != scode ) {
        RETURN_ERROR_LS(_trace_logger, status, exploration_error) << scode;
      }

      // Setup response with pdf from prediction and action indexes
      for ( size_t idx = 0; idx < pdf.size(); ++idx ) {
        response.push_back(action_ids[idx], pdf[idx]);
      }

      // Swap values in first position with values in chosen index
      scode = e::swap_chosen(std::begin(response), std::end(response), chosen_index);

      if ( S_EXPLORATION_OK != scode ) {
        RETURN_ERROR_LS(_trace_logger, status, exploration_error) << "Exploration error code: " << scode;
      }

      response.set_chosen_action_id(action_ids[chosen_index]);
      response.set_model_id(vw->id());

      return error_code::success;
    }
    catch ( const std::exception& e) {
      RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << "Unkown error";
    }
  }
}}
