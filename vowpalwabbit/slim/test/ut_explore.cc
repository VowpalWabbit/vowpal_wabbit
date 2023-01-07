#include "vw/explore/explore.h"
#include "vw/slim/vw_slim_predict.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sstream>
#include <vector>

using namespace ::testing;

TEST(ExploreSlim, SamplingRank)
{
  std::vector<float> scores = {0.f, 3.f, 1.f};  // 1,2,0 <-- top ranking
  std::vector<float> histogram(scores.size() * scores.size());
  std::vector<int> ranking(3);

  // std::fstream log("vwslim-debug.log", std::fstream::app);

  size_t rep = 50000;
  for (size_t i = 0; i < rep; i++)
  {
    std::vector<float> pdf = {0.8f, 0.1f, 0.1f};

    std::stringstream s;
    s << "abcde" << i;

    ASSERT_EQ(S_EXPLORATION_OK,
        vw_slim::vw_predict<float>::sort_by_scores(std::begin(pdf), std::end(pdf), std::begin(scores), std::end(scores),
            std::begin(ranking), std::end(ranking)));

    // Sample from the pdf
    uint32_t chosen_action_idx;
    ASSERT_EQ(S_EXPLORATION_OK,
        VW::explore::sample_after_normalizing(s.str().c_str(), std::begin(pdf), std::end(pdf), chosen_action_idx));

    // Swap top element with chosen one (unless chosen is the top)
    if (chosen_action_idx != 0)
    {
      std::iter_swap(std::begin(ranking), std::begin(ranking) + chosen_action_idx);
      std::iter_swap(std::begin(pdf), std::begin(pdf) + chosen_action_idx);
    }

    for (size_t i = 0; i < ranking.size(); i++) { histogram[i * ranking.size() + ranking[i]]++; }
  }

  for (auto& d : histogram) { d /= rep; }

  // best order is 0, 2, 1
  // rows: slots
  // cols: actions
  std::vector<float> ranking_pdf = {
      // see top action 1 w / 0.8
      0.8f, 0.1f, 0.1f,  // slot 0
      // most of the time we should see action 0
      // in 10% we should see the top action 1 swapped from top-slot to here
      0.1f, 0.0f, 0.9f,  // slot 1
      // most of the time we should see action 2
      // in 10% we should see the top action swapped from top-slot to here
      0.1f, 0.9f, 0.0f,  // slot 2
  };

  // for (size_t i = 0; i < 3; i++)
  //{
  //	log << "slot " << i << " ";
  //	for (size_t j = 0; j < 3; j++)
  //		log << histogram[i * 3 + j] << " ";
  //	log << std::endl;
  //}

  EXPECT_THAT(histogram, Pointwise(FloatNear(1e-2f), ranking_pdf));
}
