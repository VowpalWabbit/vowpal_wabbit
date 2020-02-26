#pragma once
#include "learner.h"
#include "options.h"
#include "cb.h"
#include "action_score.h"

namespace VW { namespace pmf_to_pdf
{
  LEARNER::base_learner* pmf_to_pdf_setup(VW::config::options_i& options, vw& all);
  struct reduction
  {
    void predict(example& ec);
    void learn(example& ec);

    ~reduction();
    std::vector<float> scores;
    uint32_t num_actions;
    uint32_t bandwidth;  // radius
    float min_value;
    float max_value;
    LEARNER::single_learner* _p_base;

    private:
      void transform_prediction(example& ec);

      CB::label temp_lbl_cb;
      ACTION_SCORE::action_scores temp_pred_a_s;
  };
  }}  // namespace VW::pmf_to_pdf
