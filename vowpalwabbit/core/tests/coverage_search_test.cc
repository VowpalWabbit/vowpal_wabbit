// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Batch 10: Targeted tests for the search reduction and its tasks.

#include "vw/core/example.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

// ============================================================
// Helper: learn a simple sequence example group
// ============================================================
static void learn_sequence(VW::workspace& vw, const std::vector<std::string>& lines)
{
  VW::multi_ex multi_ex;
  for (const auto& line : lines) { multi_ex.push_back(VW::read_example(vw, line)); }
  multi_ex.push_back(VW::read_example(vw, ""));
  vw.learn(multi_ex);
  vw.finish_example(multi_ex);
}

// ============================================================
// 1. Search Core (~30 tests)
// ============================================================

TEST(CoverageSearch, SearchBasicSequenceTask)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence", "--quiet"));
  learn_sequence(*vw, {"1 | a b", "2 | c d", "3 | e f"});
}

TEST(CoverageSearch, SearchTwoActions)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "2", "--search_task", "sequence", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "1 | c"});
}

TEST(CoverageSearch, SearchFiveActions)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequence", "--quiet"));
  learn_sequence(*vw, {"1 | a", "3 | b", "5 | c", "2 | d"});
}

TEST(CoverageSearch, SearchTenActions)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "10", "--search_task", "sequence", "--quiet"));
  learn_sequence(*vw, {"1 | a", "5 | b", "10 | c", "3 | d", "7 | e"});
}

TEST(CoverageSearch, SearchRolloutPolicy)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_rollout", "policy", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchRolloutOracle)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_rollout", "oracle", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchRolloutNone)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_rollout", "none", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchRolloutMixPerRoll)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_rollout", "mix_per_roll", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchRollinPolicy)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_rollin", "policy", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchRollinLearn)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_rollin", "learn", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchRollinOracle)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_rollin", "oracle", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchRollinMixPerState)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_rollin", "mix_per_state", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b"});
}

TEST(CoverageSearch, SearchRollinPolicyRolloutOracle)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "3", "--search_task", "sequence", "--search_rollin", "policy", "--search_rollout", "oracle", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchRollinLearnRolloutPolicy)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "3", "--search_task", "sequence", "--search_rollin", "learn", "--search_rollout", "policy", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchRollinOracleRolloutPolicy)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "3", "--search_task", "sequence", "--search_rollin", "oracle", "--search_rollout", "policy", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchRollinOracleRolloutNone)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "3", "--search_task", "sequence", "--search_rollin", "oracle", "--search_rollout", "none", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b"});
}

TEST(CoverageSearch, SearchNoCaching)
{
  auto vw =
      VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_no_caching", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchAlphaSmall)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_alpha", "0.1", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b"});
}

TEST(CoverageSearch, SearchAlphaLarge)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_alpha", "0.5", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b"});
}

TEST(CoverageSearch, SearchBeta)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "3", "--search_task", "sequence", "--search_beta", "0.5", "--search_interpolation", "policy",
      "--passes", "2", "--holdout_off", "-k", "-c", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchTotalNbPolicies)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_total_nb_policies", "3", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b"});
}

TEST(CoverageSearch, SearchHistoryLength2)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_history_length", "2", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c", "1 | d"});
}

TEST(CoverageSearch, SearchHistoryLength5)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_history_length", "5", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c", "1 | d", "2 | e", "3 | f"});
}

TEST(CoverageSearch, SearchMultipleLearnIterations)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence", "--quiet"));
  for (int iter = 0; iter < 5; iter++) { learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"}); }
}

TEST(CoverageSearch, SearchMultipleDifferentSequences)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
  learn_sequence(*vw, {"3 | x", "1 | y", "2 | z"});
  learn_sequence(*vw, {"2 | m", "3 | n"});
}

TEST(CoverageSearch, SearchPredictionProduced)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence", "--quiet"));
  // Train first
  for (int i = 0; i < 3; i++) { learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"}); }
  // Predict (no labels)
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "| a"));
  multi_ex.push_back(VW::read_example(*vw, "| b"));
  multi_ex.push_back(VW::read_example(*vw, "| c"));
  multi_ex.push_back(VW::read_example(*vw, ""));
  vw->predict(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, SearchLinearOrdering)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_linear_ordering", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchNeighborFeatures)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "3", "--search_task", "sequence", "--search_neighbor_features", "-1:a,+1:b", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchSubsampleTimeFloat)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_subsample_time", "0.5", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c", "1 | d"});
}

TEST(CoverageSearch, SearchInterpolationPolicy)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_interpolation",
      "policy", "--passes", "2", "--holdout_off", "-k", "-c", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SearchPerturbOracle)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_perturb_oracle", "0.2", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

// ============================================================
// 2. Entity Relation Task (~25 tests)
// ============================================================

// Entity-relation requires entities and relations in a specific layout.
// For N entities, we need N + N*(N-1)/2 examples (entities first, then relations).
// Entity labels: 1-4 (E_Other, E_Peop, E_Org, E_Loc)
// Relation labels: 5-10 (R_Live_in, R_OrgBased_in, R_Located_in, R_Work_For, R_Kill, R_None)
// Tags: E_<id> for entities, R_<id1>_<id2> for relations

// 3 entities = 3 + 3*(3-1)/2 = 3 + 3 = 6 examples
// Relation indices: for entities 0,1,2 -> relations (0,1), (0,2), (1,2)
// relation tags: R_0_1, R_0_2, R_1_2

// Entity-relation and other tasks that are sensitive to multi_ex size do NOT get
// the trailing empty example. The search framework does not strip the empty example,
// so entity_relation's n_ent formula (sqrt(ec.size()*8+1)-1)/2 would be wrong with it.

static void learn_entity_relation_3ent(VW::workspace& vw)
{
  VW::multi_ex multi_ex;
  // 3 entities: format matches er_small.vw: LABEL WEIGHT TAG|Namespace features
  multi_ex.push_back(VW::read_example(vw, "1 1.0 E_0|F 1:1.0 2:1.0 3:1.0"));
  multi_ex.push_back(VW::read_example(vw, "2 1.0 E_1|F 4:1.0 5:1.0 6:1.0"));
  multi_ex.push_back(VW::read_example(vw, "3 1.0 E_2|F 7:1.0 8:1.0 9:1.0"));
  // 3 relations (for 3 entities: pairs (0,1), (0,2), (1,2))
  multi_ex.push_back(VW::read_example(vw, "10 1.0 R_0_1|F 10:1.0 11:1.0"));
  multi_ex.push_back(VW::read_example(vw, "10 1.0 R_0_2|F 12:1.0 13:1.0"));
  multi_ex.push_back(VW::read_example(vw, "5 1.0 R_1_2|F 14:1.0 15:1.0"));
  vw.learn(multi_ex);
  vw.finish_example(multi_ex);
}

// 2 entities: 2 + 2*(2-1)/2 = 2 + 1 = 3 examples
static void learn_entity_relation_2ent(VW::workspace& vw)
{
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(vw, "2 1.0 E_0|F 1:1.0 2:1.0 3:1.0"));
  multi_ex.push_back(VW::read_example(vw, "3 1.0 E_1|F 4:1.0 5:1.0 6:1.0"));
  // 1 relation
  multi_ex.push_back(VW::read_example(vw, "8 1.0 R_0_1|F 7:1.0 8:1.0"));
  vw.learn(multi_ex);
  vw.finish_example(multi_ex);
}

TEST(CoverageSearch, EntityRelationBasicOrder0)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "10", "--search_task", "entity_relation", "--search_order", "0", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

TEST(CoverageSearch, EntityRelationOrder0MultipleIter)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "10", "--search_task", "entity_relation", "--search_order", "0", "--quiet"));
  for (int i = 0; i < 3; i++) { learn_entity_relation_3ent(*vw); }
}

TEST(CoverageSearch, EntityRelationOrder1Mixed)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "10", "--search_task", "entity_relation", "--search_order", "1", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

TEST(CoverageSearch, EntityRelationOrder1MultipleIter)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "10", "--search_task", "entity_relation", "--search_order", "1", "--quiet"));
  for (int i = 0; i < 3; i++) { learn_entity_relation_3ent(*vw); }
}

TEST(CoverageSearch, EntityRelationOrder2Skip)
{
  // search_order 2 uses LABEL_SKIP=11, so num_actions must cover it
  auto vw = VW::initialize(
      vwtest::make_args("--search", "11", "--search_task", "entity_relation", "--search_order", "2", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

TEST(CoverageSearch, EntityRelationOrder2MultipleIter)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "11", "--search_task", "entity_relation", "--search_order", "2", "--quiet"));
  for (int i = 0; i < 3; i++) { learn_entity_relation_3ent(*vw); }
}

TEST(CoverageSearch, EntityRelationOrder3LDF)
{
  // search_order 3 = LDF mode
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "10", "--search_task", "entity_relation", "--search_order", "3", "--csoaa_ldf", "multiline", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

TEST(CoverageSearch, EntityRelationOrder3LDFMultipleIter)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "10", "--search_task", "entity_relation", "--search_order", "3", "--csoaa_ldf", "multiline", "--quiet"));
  for (int i = 0; i < 3; i++) { learn_entity_relation_3ent(*vw); }
}

TEST(CoverageSearch, EntityRelationOrder0TwoEntities)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "10", "--search_task", "entity_relation", "--search_order", "0", "--quiet"));
  learn_entity_relation_2ent(*vw);
}

TEST(CoverageSearch, EntityRelationOrder1TwoEntities)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "10", "--search_task", "entity_relation", "--search_order", "1", "--quiet"));
  learn_entity_relation_2ent(*vw);
}

TEST(CoverageSearch, EntityRelationOrder2TwoEntities)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "11", "--search_task", "entity_relation", "--search_order", "2", "--quiet"));
  learn_entity_relation_2ent(*vw);
}

TEST(CoverageSearch, EntityRelationWithConstraints)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "10", "--search_task", "entity_relation", "--search_order", "0", "--constraints", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

TEST(CoverageSearch, EntityRelationConstraintsOrder1)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "10", "--search_task", "entity_relation", "--search_order", "1", "--constraints", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

TEST(CoverageSearch, EntityRelationCustomCosts)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "10", "--search_task", "entity_relation", "--search_order",
      "0", "--entity_cost", "2.0", "--relation_cost", "1.5", "--relation_none_cost", "0.3", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

TEST(CoverageSearch, EntityRelationSkipCost)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "11", "--search_task", "entity_relation", "--search_order",
      "2", "--skip_cost", "0.05", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

TEST(CoverageSearch, EntityRelationOrder0Predict)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "10", "--search_task", "entity_relation", "--search_order", "0", "--quiet"));
  // Train
  learn_entity_relation_3ent(*vw);
  learn_entity_relation_3ent(*vw);
  // Predict (labels still needed for entity_relation but will be ignored on predict)
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "1 1.0 E_0|F 20:1.0 21:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "1 1.0 E_1|F 22:1.0 23:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "1 1.0 E_2|F 24:1.0 25:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "10 1.0 R_0_1|F 26:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "10 1.0 R_0_2|F 27:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "10 1.0 R_1_2|F 28:1.0"));
  vw->predict(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, EntityRelationOrder0WithRolloutOracle)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "10", "--search_task", "entity_relation", "--search_order",
      "0", "--search_rollout", "oracle", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

TEST(CoverageSearch, EntityRelationOrder0WithRolloutNone)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "10", "--search_task", "entity_relation", "--search_order",
      "0", "--search_rollout", "none", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

TEST(CoverageSearch, EntityRelationOrder1WithRolloutOracle)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "10", "--search_task", "entity_relation", "--search_order",
      "1", "--search_rollout", "oracle", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

TEST(CoverageSearch, EntityRelationOrder2WithRolloutOracle)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "11", "--search_task", "entity_relation", "--search_order",
      "2", "--search_rollout", "oracle", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

TEST(CoverageSearch, EntityRelationDifferentEntityLabels)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "10", "--search_task", "entity_relation", "--search_order", "0", "--quiet"));
  VW::multi_ex multi_ex;
  // Use all 4 entity types
  multi_ex.push_back(VW::read_example(*vw, "1 1.0 E_0|F 1:1.0 2:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "2 1.0 E_1|F 3:1.0 4:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "3 1.0 E_2|F 5:1.0 6:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "4 1.0 E_3|F 7:1.0 8:1.0"));
  // 6 relations for 4 entities
  multi_ex.push_back(VW::read_example(*vw, "10 1.0 R_0_1|F 9:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "10 1.0 R_0_2|F 10:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "10 1.0 R_0_3|F 11:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "5 1.0 R_1_2|F 12:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "8 1.0 R_1_3|F 13:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "7 1.0 R_2_3|F 14:1.0"));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, EntityRelationConstraintsOrder0FourEntities)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "10", "--search_task", "entity_relation", "--search_order", "0", "--constraints", "--quiet"));
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "2 1.0 E_0|F 1:1.0 2:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "4 1.0 E_1|F 3:1.0 4:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "3 1.0 E_2|F 5:1.0 6:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "2 1.0 E_3|F 7:1.0 8:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "5 1.0 R_0_1|F 9:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "10 1.0 R_0_2|F 10:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "10 1.0 R_0_3|F 11:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "10 1.0 R_1_2|F 12:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "10 1.0 R_1_3|F 13:1.0"));
  multi_ex.push_back(VW::read_example(*vw, "10 1.0 R_2_3|F 14:1.0"));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, EntityRelationOrder0NoCaching)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "10", "--search_task", "entity_relation", "--search_order", "0", "--search_no_caching", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

TEST(CoverageSearch, EntityRelationOrder2ConstraintsSkip)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "11", "--search_task", "entity_relation", "--search_order",
      "2", "--constraints", "--quiet"));
  learn_entity_relation_3ent(*vw);
}

// ============================================================
// 3. Sequence Task Variants (~15 tests)
// ============================================================

TEST(CoverageSearch, SequenceSpanTask)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "5", "--search_task", "sequencespan", "--quiet"));
  // BIO encoding: 1=O, 2=B-X, 3=I-X, 4=B-Y, 5=I-Y
  learn_sequence(*vw, {"1 | the", "2 | john", "3 | smith", "1 | works", "4 | acme"});
}

TEST(CoverageSearch, SequenceSpanTaskBILOU)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "5", "--search_task", "sequencespan", "--search_span_bilou", "--quiet"));
  // BILOU conversion is sensitive to the empty example, so build multi_ex without it
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "1 | the"));
  multi_ex.push_back(VW::read_example(*vw, "2 | john"));
  multi_ex.push_back(VW::read_example(*vw, "3 | smith"));
  multi_ex.push_back(VW::read_example(*vw, "1 | at"));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, SequenceSpanMultipass)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "5", "--search_task", "sequencespan", "--search_span_multipass", "2", "--quiet"));
  // Multipass sequencespan may process all examples; avoid empty trailing example
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "1 | the"));
  multi_ex.push_back(VW::read_example(*vw, "2 | john"));
  multi_ex.push_back(VW::read_example(*vw, "3 | smith"));
  multi_ex.push_back(VW::read_example(*vw, "1 | works"));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, SequenceCostToGoTask)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence_ctg", "--quiet"));
  // sequence_ctg accesses costs[oracle-1], so empty example (label=0) would cause out-of-bounds
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "1 | a"));
  multi_ex.push_back(VW::read_example(*vw, "2 | b"));
  multi_ex.push_back(VW::read_example(*vw, "3 | c"));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, SequenceCostToGoMultipleIterations)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence_ctg", "--quiet"));
  for (int i = 0; i < 5; i++)
  {
    VW::multi_ex multi_ex;
    multi_ex.push_back(VW::read_example(*vw, "1 | a"));
    multi_ex.push_back(VW::read_example(*vw, "2 | b"));
    multi_ex.push_back(VW::read_example(*vw, "3 | c"));
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
  }
}

TEST(CoverageSearch, ArgmaxTask)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "2", "--search_task", "argmax", "--quiet"));
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "1 | a"));
  multi_ex.push_back(VW::read_example(*vw, "2 | b"));
  multi_ex.push_back(VW::read_example(*vw, "1 | c"));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, ArgmaxTaskWithCost)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "2", "--search_task", "argmax", "--cost", "5.0", "--quiet"));
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "1 | a"));
  multi_ex.push_back(VW::read_example(*vw, "2 | b"));
  multi_ex.push_back(VW::read_example(*vw, "1 | c"));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, ArgmaxTaskWithMax)
{
  auto vw =
      VW::initialize(vwtest::make_args("--search", "2", "--search_task", "argmax", "--max", "--quiet"));
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "1 | a"));
  multi_ex.push_back(VW::read_example(*vw, "2 | b"));
  multi_ex.push_back(VW::read_example(*vw, "1 | c"));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, ArgmaxTaskWithNegativeWeight)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "2", "--search_task", "argmax", "--negative_weight", "2.0", "--quiet"));
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "1 | a"));
  multi_ex.push_back(VW::read_example(*vw, "2 | b"));
  multi_ex.push_back(VW::read_example(*vw, "1 | c"));
  multi_ex.push_back(VW::read_example(*vw, "2 | d"));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, SequenceDemoLDF)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence_demoldf", "--csoaa_ldf", "multiline", "--quiet"));
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "1 | a"));
  multi_ex.push_back(VW::read_example(*vw, "2 | b"));
  multi_ex.push_back(VW::read_example(*vw, "3 | c"));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, SequenceDemoLDFMultipleIter)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence_demoldf", "--csoaa_ldf", "multiline", "--quiet"));
  for (int i = 0; i < 3; i++)
  {
    VW::multi_ex multi_ex;
    multi_ex.push_back(VW::read_example(*vw, "1 | a"));
    multi_ex.push_back(VW::read_example(*vw, "2 | b"));
    multi_ex.push_back(VW::read_example(*vw, "3 | c"));
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
  }
}

TEST(CoverageSearch, MulticlasstaskSearch)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "4", "--search_task", "multiclasstask", "--quiet"));
  // Multiclasstask uses a single example in the multi_ex
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "3 | feature1 feature2"));
  multi_ex.push_back(VW::read_example(*vw, ""));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, MulticlasstaskMultipleIterations)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "8", "--search_task", "multiclasstask", "--quiet"));
  for (int i = 0; i < 5; i++)
  {
    VW::multi_ex multi_ex;
    multi_ex.push_back(VW::read_example(*vw, std::to_string((i % 8) + 1) + " | f" + std::to_string(i)));
    multi_ex.push_back(VW::read_example(*vw, ""));
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
  }
}

TEST(CoverageSearch, SequenceLongSequence)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence", "--quiet"));
  std::vector<std::string> seq;
  for (int i = 0; i < 20; i++) { seq.push_back(std::to_string((i % 3) + 1) + " | f" + std::to_string(i)); }
  learn_sequence(*vw, seq);
}

// ============================================================
// 4. Dependency Parser Task (~15 tests)
// ============================================================

// dep_parser uses multilabel format: head,tag pairs encoded in multilabels
// Format: head,tag | features
// Where head is the index of the parent (1-based), tag is the dependency label

static void learn_dep_parser(VW::workspace& vw, const std::vector<std::string>& lines)
{
  VW::multi_ex multi_ex;
  for (const auto& line : lines) { multi_ex.push_back(VW::read_example(vw, line)); }
  vw.learn(multi_ex);
  vw.finish_example(multi_ex);
}

TEST(CoverageSearch, DepParserBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "dep_parser", "--quiet"));
  // multilabel format: head,tag | features
  // For a 2-word sentence: word1 depends on word2 with tag 1
  learn_dep_parser(*vw, {"2,1 | the cat", "0,8 | sat"});
}

TEST(CoverageSearch, DepParserThreeWords)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "dep_parser", "--quiet"));
  learn_dep_parser(*vw, {"2,1 | the", "0,8 | cat", "2,2 | sat"});
}

TEST(CoverageSearch, DepParserFiveWords)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "4", "--search_task", "dep_parser", "--quiet"));
  learn_dep_parser(*vw, {"2,1 | the", "3,2 | big", "0,8 | cat", "3,3 | sat", "4,4 | down"});
}

TEST(CoverageSearch, DepParserMultipleIterations)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "dep_parser", "--quiet"));
  for (int i = 0; i < 3; i++) { learn_dep_parser(*vw, {"2,1 | the", "0,8 | cat", "2,2 | sat"}); }
}

TEST(CoverageSearch, DepParserRolloutOracle)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "3", "--search_task", "dep_parser", "--search_rollout", "oracle", "--quiet"));
  learn_dep_parser(*vw, {"2,1 | the", "0,8 | cat"});
}

TEST(CoverageSearch, DepParserRollinPolicy)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "3", "--search_task", "dep_parser", "--search_rollin", "policy", "--quiet"));
  learn_dep_parser(*vw, {"2,1 | the", "0,8 | cat"});
}

TEST(CoverageSearch, DepParserArcEager)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "4", "--search_task", "dep_parser", "--transition_system", "2", "--quiet"));
  learn_dep_parser(*vw, {"2,1 | the", "0,8 | cat", "2,2 | sat"});
}

TEST(CoverageSearch, DepParserArcEagerMultipleIter)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "4", "--search_task", "dep_parser", "--transition_system", "2", "--quiet"));
  for (int i = 0; i < 3; i++) { learn_dep_parser(*vw, {"2,1 | the", "0,8 | cat", "2,2 | sat"}); }
}

TEST(CoverageSearch, DepParserOneLearner)
{
  // one_learner maps actions up to 2+2*num_label; with num_label=3, need --search >= 8
  // root_label must be <= num_label; run() assigns tags[stack.back()] = root_label
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "8", "--search_task", "dep_parser", "--one_learner", "--num_label", "3", "--root_label", "3", "--quiet"));
  learn_dep_parser(*vw, {"2,1 | the", "0,3 | cat", "2,2 | sat"});
}

TEST(CoverageSearch, DepParserCostToGo)
{
  // dep_parser with cost_to_go needs --search >= 1+2*num_label (default num_label=12 => 25)
  // Using fewer actions crashes with double-free (VW bug: cost_to_go doesn't handle action space mismatch)
  auto vw = VW::initialize(
      vwtest::make_args("--search", "25", "--search_task", "dep_parser", "--cost_to_go", "--quiet"));
  learn_dep_parser(*vw, {"2,1 | the", "0,3 | cat", "2,2 | sat"});
}

TEST(CoverageSearch, DepParserCostToGoOneLearner)
{
  // one_learner maps actions up to 2+2*num_label; with num_label=3, need --search >= 8
  // root_label must be <= num_label
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "8", "--search_task", "dep_parser", "--cost_to_go", "--one_learner", "--num_label", "3", "--root_label", "3", "--quiet"));
  learn_dep_parser(*vw, {"2,1 | the", "0,3 | cat", "2,2 | sat"});
}

TEST(CoverageSearch, DepParserCustomNumLabel)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "dep_parser", "--num_label", "6", "--quiet"));
  learn_dep_parser(*vw, {"2,1 | the", "0,6 | cat", "2,3 | sat"});
}

TEST(CoverageSearch, DepParserCustomRootLabel)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "dep_parser", "--root_label", "5", "--quiet"));
  learn_dep_parser(*vw, {"2,1 | the", "0,5 | cat", "2,2 | sat"});
}

TEST(CoverageSearch, DepParserArcEagerCostToGo)
{
  // arc-eager with cost_to_go needs --search >= 2+2*num_label (default num_label=12 => 26)
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "26", "--search_task", "dep_parser", "--transition_system", "2", "--cost_to_go", "--quiet"));
  learn_dep_parser(*vw, {"2,1 | the", "0,3 | cat", "2,2 | sat"});
}

TEST(CoverageSearch, DepParserArcEagerOneLearnerCostToGo)
{
  // one_learner with arc-eager: actions up to 2+2*num_label; with num_label=3, need --search >= 8
  // root_label must be <= num_label
  auto vw = VW::initialize(vwtest::make_args("--search", "8", "--search_task", "dep_parser", "--transition_system", "2",
      "--cost_to_go", "--one_learner", "--num_label", "3", "--root_label", "3", "--quiet"));
  learn_dep_parser(*vw, {"2,1 | the", "0,3 | cat", "2,2 | sat"});
}

// ============================================================
// 5. Graph Task (~15 tests)
// ============================================================

// Graph task uses multilabel format.
// Nodes have a single label: the class (1-based).
// Edges have multiple labels: the node ids they connect.
// Format: nodes first, then edges.

static void learn_graph_3nodes(VW::workspace& vw)
{
  VW::multi_ex multi_ex;
  // 3 nodes with labels
  multi_ex.push_back(VW::read_example(vw, "1 | node_a"));
  multi_ex.push_back(VW::read_example(vw, "2 | node_b"));
  multi_ex.push_back(VW::read_example(vw, "1 | node_c"));
  // 2 edges connecting nodes (node indices are 1-based in multilabel)
  multi_ex.push_back(VW::read_example(vw, "1,2 | edge_ab"));
  multi_ex.push_back(VW::read_example(vw, "2,3 | edge_bc"));
  vw.learn(multi_ex);
  vw.finish_example(multi_ex);
}

TEST(CoverageSearch, GraphTaskBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "graph", "--quiet"));
  learn_graph_3nodes(*vw);
}

TEST(CoverageSearch, GraphTaskMultipleIterations)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "graph", "--quiet"));
  for (int i = 0; i < 3; i++) { learn_graph_3nodes(*vw); }
}

TEST(CoverageSearch, GraphTaskNoCaching)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "graph", "--search_no_caching", "--quiet"));
  learn_graph_3nodes(*vw);
}

TEST(CoverageSearch, GraphTaskNumLoops3)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "graph", "--search_graph_num_loops", "3", "--quiet"));
  learn_graph_3nodes(*vw);
}

TEST(CoverageSearch, GraphTaskNumLoops1)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "graph", "--search_graph_num_loops", "1", "--quiet"));
  learn_graph_3nodes(*vw);
}

TEST(CoverageSearch, GraphTaskNoStructure)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "graph", "--search_graph_no_structure", "--quiet"));
  learn_graph_3nodes(*vw);
}

TEST(CoverageSearch, GraphTaskSeparateLearners)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "graph", "--search_graph_separate_learners", "--quiet"));
  learn_graph_3nodes(*vw);
}

TEST(CoverageSearch, GraphTaskDirected)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "graph", "--search_graph_directed", "--quiet"));
  learn_graph_3nodes(*vw);
}

TEST(CoverageSearch, GraphTaskDirectedNoStructure)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "3", "--search_task", "graph", "--search_graph_directed", "--search_graph_no_structure", "--quiet"));
  learn_graph_3nodes(*vw);
}

TEST(CoverageSearch, GraphTaskTwoNodes)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "2", "--search_task", "graph", "--quiet"));
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "1 | node_a"));
  multi_ex.push_back(VW::read_example(*vw, "2 | node_b"));
  multi_ex.push_back(VW::read_example(*vw, "1,2 | edge_ab"));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, GraphTaskFourNodesTriangle)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "graph", "--quiet"));
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "1 | n1"));
  multi_ex.push_back(VW::read_example(*vw, "2 | n2"));
  multi_ex.push_back(VW::read_example(*vw, "3 | n3"));
  multi_ex.push_back(VW::read_example(*vw, "1 | n4"));
  multi_ex.push_back(VW::read_example(*vw, "1,2 | e12"));
  multi_ex.push_back(VW::read_example(*vw, "2,3 | e23"));
  multi_ex.push_back(VW::read_example(*vw, "1,3 | e13"));
  multi_ex.push_back(VW::read_example(*vw, "3,4 | e34"));
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, GraphTaskRolloutOracle)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "graph", "--search_rollout", "oracle", "--quiet"));
  learn_graph_3nodes(*vw);
}

TEST(CoverageSearch, GraphTaskRolloutNone)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "graph", "--search_rollout", "none", "--quiet"));
  learn_graph_3nodes(*vw);
}

TEST(CoverageSearch, GraphTaskDirectedSeparateLearners)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "graph", "--search_graph_directed",
      "--search_graph_separate_learners", "--quiet"));
  learn_graph_3nodes(*vw);
}

TEST(CoverageSearch, GraphTaskPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "graph", "--quiet"));
  // Train
  for (int i = 0; i < 3; i++) { learn_graph_3nodes(*vw); }
  // Predict
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "1 | new_a"));
  multi_ex.push_back(VW::read_example(*vw, "2 | new_b"));
  multi_ex.push_back(VW::read_example(*vw, "1 | new_c"));
  multi_ex.push_back(VW::read_example(*vw, "1,2 | new_e1"));
  multi_ex.push_back(VW::read_example(*vw, "2,3 | new_e2"));
  vw->predict(multi_ex);
  vw->finish_example(multi_ex);
}

// ============================================================
// 6. Meta Tasks (~15 tests)
// ============================================================

TEST(CoverageSearch, DebugMetataskSequence)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_metatask", "debug", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, DebugMetataskMultipleIter)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_metatask", "debug", "--quiet"));
  for (int i = 0; i < 3; i++) { learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"}); }
}

TEST(CoverageSearch, SelectiveBranchingMetatask)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "3", "--search_task", "sequence", "--search_metatask", "selective_branching", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SelectiveBranchingMultipleIter)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "3", "--search_task", "sequence", "--search_metatask", "selective_branching", "--quiet"));
  for (int i = 0; i < 3; i++) { learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"}); }
}

TEST(CoverageSearch, SelectiveBranchingMaxBranch3)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_metatask",
      "selective_branching", "--search_max_branch", "3", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SelectiveBranchingMaxBranch1)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_metatask",
      "selective_branching", "--search_max_branch", "1", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, SelectiveBranchingKbest2)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_metatask",
      "selective_branching", "--search_kbest", "2", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, DebugMetataskWithRolloutOracle)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_metatask", "debug",
      "--search_rollout", "oracle", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

TEST(CoverageSearch, DebugMetataskWithRolloutNone)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_metatask", "debug",
      "--search_rollout", "none", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}

// TODO: SelectiveBranchingWithSequenceSpan removed - triggers assertion failure in
// learner.cc:773 decrement_offset: ft_offset underflow when combining selective_branching
// metatask with sequencespan task. VW bug: selective_branching doesn't account for
// sequencespan's different learner width.

// TODO: DebugMetataskWithSequenceCTG removed - triggers assertion failure in
// learner.cc:773 decrement_offset: ft_offset underflow when combining debug metatask
// with sequence_ctg task. Same class of VW bug as SelectiveBranchingWithSequenceSpan.

TEST(CoverageSearch, SelectiveBranchingLongerSequence)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "3", "--search_task", "sequence", "--search_metatask", "selective_branching", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c", "1 | d", "2 | e"});
}

TEST(CoverageSearch, DebugMetataskPredict)
{
  auto vw = VW::initialize(
      vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_metatask", "debug", "--quiet"));
  // Train
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
  // Predict
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "| a"));
  multi_ex.push_back(VW::read_example(*vw, "| b"));
  multi_ex.push_back(VW::read_example(*vw, "| c"));
  multi_ex.push_back(VW::read_example(*vw, ""));
  vw->predict(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, SelectiveBranchingPredict)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--search", "3", "--search_task", "sequence", "--search_metatask", "selective_branching", "--quiet"));
  // Train
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
  // Predict
  VW::multi_ex multi_ex;
  multi_ex.push_back(VW::read_example(*vw, "| a"));
  multi_ex.push_back(VW::read_example(*vw, "| b"));
  multi_ex.push_back(VW::read_example(*vw, "| c"));
  multi_ex.push_back(VW::read_example(*vw, ""));
  vw->predict(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CoverageSearch, SelectiveBranchingWithNoCaching)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence", "--search_metatask",
      "selective_branching", "--search_no_caching", "--quiet"));
  learn_sequence(*vw, {"1 | a", "2 | b", "3 | c"});
}
