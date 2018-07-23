#include "vw_model.h"
#include "err_constants.h"
#include "object_factory.h"
#include "explore.h"
#include "ranking_response.h"

namespace e = exploration;
namespace reinforcement_learning { namespace model_management {
  
  vw_model::vw_model() :_vw_pool(nullptr) 
  {}

  int vw_model::update(const model_data& data, api_status* status) {
    try {
      const auto new_model = std::make_shared<safe_vw>(data.data(), data.data_sz());
      _vw_pool.update_factory(new safe_vw_factory(new_model));
    }
    catch(const std::exception& e) {
      RETURN_ERROR_LS(status, model_update_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(status, model_update_error) << "Unkown error";
    }

    return error_code::success;
  }

  int vw_model::choose_rank(const char* rnd_seed, const char* features, ranking_response& response, api_status* status) {
    try {
      pooled_vw vw(_vw_pool, _vw_pool.get_or_create());

      // Rank actions using the model.  Should generate a pdf
      auto ranking = vw->rank(features);

      // Pick using the pdf. NOTE: sample_after_normalizing() can change the pdf
      uint32_t action;
      auto const scode = e::sample_after_normalizing(rnd_seed, std::begin(ranking), std::end(ranking), action);

      if ( S_EXPLORATION_OK != scode ) {
        RETURN_ERROR_LS(status, exploration_error) << scode;
      }

      response.push_back(action, ranking[action]);

      // Setup response with pdf from prediction and choosen action
      for ( size_t idx = 0; idx < ranking.size(); ++idx )
        if ( action != idx)
          response.push_back(idx, ranking[idx]);
      response.set_choosen_action_id(action);
      response.set_model_id(vw->id());

      return error_code::success;
    }
    catch ( const std::exception& e) {
      RETURN_ERROR_LS(status, model_rank_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(status, model_rank_error) << "Unkown error";
    }
  }
}}
