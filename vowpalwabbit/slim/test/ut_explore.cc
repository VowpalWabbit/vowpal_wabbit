#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ut_util.h"
#include <vector>
#include <sstream>
#include "explore.h"
#include "vw_slim_predict.h"
using namespace ::testing;

TEST(ExploreTestSuite, EpsilonGreedy)
{
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_epsilon_greedy(0.4f, 2, begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNearPointwise(1e-6f), std::vector<float>{0.1f, 0.1f, 0.7f, 0.1f}));
}

TEST(ExploreTestSuite, EpsilonGreedyTopActionOutOfBounds)
{
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_epsilon_greedy(0.4f, 8, begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNearPointwise(1e-6f), std::vector<float>{0.1f, 0.1f, 0.1f, 0.7f}));
}

TEST(ExploreTestSuite, EpsilonGreedy_bad_range)
{
  std::vector<float> pdf;
  float x;
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::generate_epsilon_greedy(0.4f, 0, begin(pdf), end(pdf)));
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::generate_epsilon_greedy(0.4f, 0, &x, &x - 3));
  EXPECT_THAT(pdf.size(), 0);
}

TEST(ExploreTestSuite, Softmax)
{
  std::vector<float> scores = {1, 2, 3, 8};
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_softmax(0.2f, begin(scores), end(scores), begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNearPointwise(1e-3f), std::vector<float>{0.128f, 0.157f, 0.192f, 0.522f}));
}

TEST(ExploreTestSuite, SoftmaxInBalanced)
{
  std::vector<float> scores = {1, 2, 3};
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_softmax(0.2f, begin(scores), end(scores), begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNearPointwise(1e-3f), std::vector<float>{0.269f, 0.328f, 0.401f, 0}));
}

TEST(ExploreTestSuite, SoftmaxInBalanced2)
{
  std::vector<float> scores = {1, 2, 3, 8, 4};
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_softmax(0.2f, begin(scores), end(scores), begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNearPointwise(1e-3f), std::vector<float>{0.128f, 0.157f, 0.192f, 0.522f}));
}

TEST(ExploreTestSuite, Softmax_bad_range)
{
  std::vector<float> scores;
  std::vector<float> pdf;
  float x;
  EXPECT_THAT(
      E_EXPLORATION_BAD_RANGE, exploration::generate_softmax(0.2f, begin(scores), end(scores), begin(pdf), end(pdf)));
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::generate_softmax(0.2f, begin(scores), end(scores), &x, &x - 3));
}

TEST(ExploreTestSuite, Bag)
{
  std::vector<uint16_t> top_actions = {0, 0, 1, 1};
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_bag(begin(top_actions), end(top_actions), begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNearPointwise(1e-3f), std::vector<float>{0, 0, 0.5, 0.5f}));
}

TEST(ExploreTestSuite, Bag10)
{
  std::vector<uint16_t> top_actions = {10};
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_bag(begin(top_actions), end(top_actions), begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNearPointwise(1e-3f), std::vector<float>{1.f, 0, 0, 0}));
}

TEST(ExploreTestSuite, BagEmpty)
{
  std::vector<uint16_t> top_actions;
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_bag(begin(top_actions), end(top_actions), begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNearPointwise(1e-3f), std::vector<float>{1.f, 0, 0, 0}));
}

TEST(ExploreTestSuite, Bag_bad_range)
{
  std::vector<uint16_t> top_actions;
  std::vector<float> pdf;
  float x;

  EXPECT_THAT(
      E_EXPLORATION_BAD_RANGE, exploration::generate_bag(begin(top_actions), end(top_actions), begin(pdf), end(pdf)));
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::generate_bag(begin(top_actions), end(top_actions), &x, &x - 3));
}

TEST(ExploreTestSuite, enforce_minimum_probability)
{
  std::vector<float> pdf = {1.f, 0, 0};
  exploration::enforce_minimum_probability(0.3f, true, begin(pdf), end(pdf));
  EXPECT_THAT(pdf, Pointwise(FloatNearPointwise(1e-3f), std::vector<float>{.8f, .1f, .1f}));
}

TEST(ExploreTestSuite, enforce_minimum_probability_no_zeros)
{
  std::vector<float> pdf = {0.9f, 0.1f, 0};
  EXPECT_THAT(S_EXPLORATION_OK, exploration::enforce_minimum_probability(0.6f, false, begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNearPointwise(1e-3f), std::vector<float>{.8f, .2f, .0f}));
}

TEST(ExploreTestSuite, enforce_minimum_probability_uniform)
{
  std::vector<float> pdf = {0.9f, 0.1f, 0, 0};
  EXPECT_THAT(S_EXPLORATION_OK, exploration::enforce_minimum_probability(1.f, true, begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNearPointwise(1e-3f), std::vector<float>{.25f, .25f, .25f, .25f}));
}

TEST(ExploreTestSuite, enforce_minimum_probability_uniform_no_zeros)
{
  std::vector<float> pdf = {0.9f, 0.1f, 0};
  EXPECT_THAT(S_EXPLORATION_OK, exploration::enforce_minimum_probability(1.f, false, begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNearPointwise(1e-3f), std::vector<float>{.5f, .5f, .0f}));
}

TEST(ExploreTestSuite, enforce_minimum_probability_bad_range)
{
  std::vector<float> pdf;
  float x;
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::enforce_minimum_probability(1.f, false, begin(pdf), end(pdf)));
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::enforce_minimum_probability(1.f, false, &x, &x - 3));
}

TEST(ExploreTestSuite, sampling)
{
  std::vector<float> pdf = {0.8f, 0.1f, 0.1f};
  std::vector<float> histogram(3);

  size_t rep = 10000;
  uint32_t chosen_index;
  for (size_t i = 0; i < rep; i++)
  {
    std::stringstream s;
    s << "abcde" << i;
    ASSERT_EQ(0, exploration::sample_after_normalizing(s.str().c_str(), std::begin(pdf), std::end(pdf), chosen_index));

    histogram[chosen_index]++;
  }
  for (auto& d : histogram) d /= rep;

  EXPECT_THAT(pdf, Pointwise(FloatNearPointwise(1e-2f), histogram));
}

TEST(PairIteratorTestSuite, simple_test)
{
  using ActionType = size_t;
  // Verify sort of actions using scores
  const int num_actions = 3;
  ActionType actions[num_actions];
  float pdf[num_actions];
  float n = 0.f;
  std::generate(std::begin(pdf), std::end(pdf), [&n]() { return n++; });
  std::iota(std::begin(actions), std::end(actions), 0);
  float scores[] = {.4f, .1f, .2f};

  // Sort two vectors using scores
  using FirstVal = ActionType;
  using SecondVal = float;
  using FirstIt = FirstVal*;
  using SecondIt = SecondVal*;
  using iter = vw_slim::internal::collection_pair_iterator<FirstIt, SecondIt>;
  using loc = vw_slim::internal::location_value<FirstIt, SecondIt>;
  using loc_ref = vw_slim::internal::location_reference<FirstIt, SecondIt>;

  const iter begin_coll(std::begin(actions), std::begin(pdf));
  const iter end_coll(std::end(actions), std::end(pdf));
  size_t diff = end_coll - begin_coll;
  std::sort(begin_coll, end_coll, [&scores](const loc& l, const loc& r) { return scores[l._val1] < scores[r._val1]; });

  EXPECT_THAT(actions, ElementsAre(1, 2, 0));
  EXPECT_THAT(pdf, ElementsAre(1.0f, 2.0f, 0.0f));
}

TEST(ExploreTestSuite, sampling_rank)
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
        exploration::sample_after_normalizing(s.str().c_str(), std::begin(pdf), std::end(pdf), chosen_action_idx));

    // Swap top element with chosen one (unless chosen is the top)
    if (chosen_action_idx != 0)
    {
      std::iter_swap(std::begin(ranking), std::begin(ranking) + chosen_action_idx);
      std::iter_swap(std::begin(pdf), std::begin(pdf) + chosen_action_idx);
    }

    for (size_t i = 0; i < ranking.size(); i++) histogram[i * ranking.size() + ranking[i]]++;
  }

  for (auto& d : histogram) d /= rep;

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

  EXPECT_THAT(histogram, Pointwise(FloatNearPointwise(1e-2f), ranking_pdf));
}

TEST(ExploreTestSuite, sampling_rank_bad_range)
{
  std::vector<float> pdf;
  std::vector<float> scores;
  std::vector<int> ranking(3);
  float x;
  uint32_t chosen_index;

  EXPECT_THAT(E_EXPLORATION_BAD_RANGE,
      exploration::sample_after_normalizing("abc", std::begin(pdf), std::end(pdf), chosen_index));
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::sample_after_normalizing("abc", &x, &x - 3, chosen_index));
}

TEST(ExploreTestSuite, sampling_rank_zero_pdf)
{
  std::vector<float> pdf = {0.f, 0.f, 0.f};
  std::vector<float> expected_pdf = {1.f, 0.f, 0.f};

  uint32_t chosen_index;

  EXPECT_THAT(
      S_EXPLORATION_OK, exploration::sample_after_normalizing("abc", std::begin(pdf), std::end(pdf), chosen_index));

  EXPECT_THAT(expected_pdf, Pointwise(FloatNearPointwise(1e-2f), pdf));
}

TEST(ExploreTestSuite, sampling_rank_negative_pdf)
{
  std::vector<float> pdf = {1.0f, -2.f, -3.f};
  std::vector<float> expected_pdf = {1.f, 0.f, 0.f};
  std::vector<int> ranking(3);
  uint32_t chosen_index;

  EXPECT_THAT(
      S_EXPLORATION_OK, exploration::sample_after_normalizing("abc", std::begin(pdf), std::end(pdf), chosen_index));
  EXPECT_THAT(expected_pdf, Pointwise(FloatNearPointwise(1e-2f), pdf));
  EXPECT_THAT(0, chosen_index);
}
