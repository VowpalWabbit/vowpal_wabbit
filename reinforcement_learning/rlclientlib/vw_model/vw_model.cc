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
      const auto new_model = std::make_shared<safe_vw>(data.data(), data.data_sz());
      _vw_pool.update_factory(new safe_vw_factory(new_model));
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
      std::vector<int> actions;
      std::vector<float> scores;
      vw->rank(features, actions, scores);

      // Pick using the pdf. NOTE: sample_after_normalizing() can change the pdf
      uint32_t action;
      auto const scode = e::sample_after_normalizing(rnd_seed, std::begin(scores), std::end(scores), action);

      if ( S_EXPLORATION_OK != scode ) {
        RETURN_ERROR_LS(_trace_logger, status, exploration_error) << scode;
      }

      response.push_back(actions[action], scores[action]);

      // Setup response with pdf from prediction and chosen action
      for (size_t idx = 1; idx < actions.size(); ++idx) {
        const auto cur_idx = action != idx ? idx : 0;
        response.push_back(actions[cur_idx], scores[cur_idx]);
      }
      response.set_chosen_action_id(action);
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
