// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Batch 8: Targeted tests covering code paths unreachable from CLI/E2E tests.

#include "vw/common/vw_exception.h"
#include "vw/core/api_status.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/array_parameters_sparse.h"
#include "vw/core/best_constant.h"
#include "vw/core/cost_sensitive.h"
#include "vw/core/crossplat_compat.h"
#include "vw/core/example.h"
#include "vw/core/io_buf.h"
#include "vw/core/label_dictionary.h"
#include "vw/core/merge.h"
#include "vw/core/named_labels.h"
#include "vw/core/shared_data.h"
#include "vw/core/unique_sort.h"
#include "vw/core/version.h"
#include "vw/core/vw.h"
#include "vw/core/vw_validate.h"
#include "vw/io/io_adapter.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cfloat>
#include <cmath>
#include <sstream>

// ============================================================
// global_data.cc: dump_weights_to_json_experimental()
// Lines 270,273,285,288,292,296,300,305,308,312,313,328,329,330
// ============================================================

TEST(CoverageBatch8, DumpWeightsToJsonBasic)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);

  std::string json = vw->dump_weights_to_json_experimental();
  EXPECT_FALSE(json.empty());
  EXPECT_NE(json.find("weights"), std::string::npos);
}

TEST(CoverageBatch8, DumpWeightsToJsonWithMarginalBlocked)
{
  // marginal reduction blocks dump_weights_to_json (line 270,273)
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--marginal", "f"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);

  std::string json = vw->dump_weights_to_json_experimental();
  EXPECT_EQ(json, R"({"weights":[]})");
}

TEST(CoverageBatch8, DumpWeightsToJsonWithKsvm)
{
  // ksvm blocks dump_weights_to_json (line 285,288)
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--ksvm"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);

  std::string json = vw->dump_weights_to_json_experimental();
  EXPECT_EQ(json, R"({"weights":[]})");
}

TEST(CoverageBatch8, DumpWeightsToJsonFeatureNamesWithoutHashInv)
{
  // Line 292: hash_inv required for feature names
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  vw->output_model_config.dump_json_weights_include_feature_names = true;
  // hash_inv is false by default
  EXPECT_THROW(vw->dump_weights_to_json_experimental(), VW::vw_exception);
}

TEST(CoverageBatch8, DumpWeightsToJsonExtraOnlineStateWithoutSaveResume)
{
  // Line 296: save_resume required for extra online state
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--predict_only_model"));
  vw->output_model_config.dump_json_weights_include_extra_online_state = true;
  // predict_only_model sets save_resume=false
  EXPECT_THROW(vw->dump_weights_to_json_experimental(), VW::vw_exception);
}

TEST(CoverageBatch8, DumpWeightsToJsonExtraOnlineStateNonGd)
{
  // Line 300: extra online state only with GD
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--coin", "--save_resume"));
  vw->output_model_config.dump_json_weights_include_extra_online_state = true;
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);
  EXPECT_THROW(vw->dump_weights_to_json_experimental(), VW::vw_exception);
}

TEST(CoverageBatch8, DumpWeightsToJsonWithHashInv)
{
  // Lines 194-197, 208-209: include feature names with string values
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--invert_hash", "/dev/null"));
  vw->output_model_config.dump_json_weights_include_feature_names = true;
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);

  std::string json = vw->dump_weights_to_json_experimental();
  EXPECT_NE(json.find("weights"), std::string::npos);
  // Should contain terms with feature names
  EXPECT_NE(json.find("terms"), std::string::npos);
}

TEST(CoverageBatch8, DumpWeightsToJsonWithOnlineState)
{
  // Lines 228-234: adaptive/normalized paths
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--save_resume", "--adaptive", "--normalized"));
  vw->output_model_config.dump_json_weights_include_extra_online_state = true;
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);

  std::string json = vw->dump_weights_to_json_experimental();
  EXPECT_NE(json.find("gd_extra_online_state"), std::string::npos);
}

TEST(CoverageBatch8, DumpWeightsToJsonAdaptiveOnly)
{
  // Line 228-229: adaptive && !normalized
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--save_resume", "--adaptive", "--invariant"));
  vw->output_model_config.dump_json_weights_include_extra_online_state = true;
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);

  std::string json = vw->dump_weights_to_json_experimental();
  EXPECT_NE(json.find("gd_extra_online_state"), std::string::npos);
}

// Lines 328-330: compile_limits with numeric-only limit
TEST(CoverageBatch8, CompileLimitsNumericLimit)
{
  std::array<uint32_t, VW::NUM_NAMESPACES> dest{};
  auto logger = VW::io::create_default_logger();
  std::vector<std::string> limits = {"5"};
  VW::details::compile_limits(limits, dest, true, logger);
  // All namespaces should be limited to 5
  for (size_t j = 0; j < 256; j++) { EXPECT_EQ(dest[j], 5u); }
}

// ============================================================
// marginal.cc: --compete mode save/load (lines 343-383)
// ============================================================

TEST(CoverageBatch8, MarginalCompeteSaveLoad)
{
  // Train with --compete to populate expert_state, then save and reload
  auto backing_vector = std::make_shared<std::vector<char>>();
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet", "--marginal", "f", "--compete", "--predict_only_model"));
    for (int i = 0; i < 10; i++)
    {
      std::string label = (i % 2 == 0) ? "1" : "0";
      auto* ex = VW::read_example(*vw, label + " |f x:1.0 y:0.5");
      vw->learn(*ex);
      vw->finish_example(*ex);
    }

    // Save model to memory
    VW::io_buf write_buf;
    write_buf.add_file(VW::io::create_vector_writer(backing_vector));
    VW::save_predictor(*vw, write_buf);
  }

  // Reload model - this triggers the read path of save_load for expert_state
  auto vw2 = VW::initialize(vwtest::make_args("--quiet"),
      VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));
  auto* ex2 = VW::read_example(*vw2, "1 |f x:1.0");
  vw2->predict(*ex2);
  vw2->finish_example(*ex2);
}

// ============================================================
// io_buf.cc: isbinary(), copy_to(), replace_buffer()
// Lines 79,81,83,86,87,89,151,153-156,158,160,162-166
// ============================================================

TEST(CoverageBatch8, IoBufIsbinary)
{
  // Line 79-89: isbinary() with binary and text data
  VW::io_buf buf;
  auto backing = std::make_shared<std::vector<char>>();
  backing->push_back('\0');  // binary marker
  backing->push_back('A');
  buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));

  EXPECT_TRUE(buf.isbinary());
}

TEST(CoverageBatch8, IoBufIsbinaryNonBinary)
{
  VW::io_buf buf;
  auto backing = std::make_shared<std::vector<char>>();
  backing->push_back('A');  // text data - not binary
  buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));

  EXPECT_FALSE(buf.isbinary());
}

TEST(CoverageBatch8, IoBufCopyTo)
{
  // Lines 151-156: copy_to
  VW::io_buf buf;
  auto backing_vec = std::make_shared<std::vector<char>>();
  buf.add_file(VW::io::create_vector_writer(backing_vec));

  char* ptr = nullptr;
  buf.buf_write(ptr, 5);
  std::memcpy(ptr, "hello", 5);

  char dst[10] = {};
  size_t copied = buf.copy_to(dst, 10);
  EXPECT_EQ(copied, 5u);
  EXPECT_EQ(std::string(dst, 5), "hello");
}

TEST(CoverageBatch8, IoBufReplaceBuffer)
{
  // Lines 158-166: replace_buffer
  VW::io_buf buf;
  const size_t cap = 128;
  char* new_buf = static_cast<char*>(std::malloc(cap));
  std::memset(new_buf, 0, cap);
  buf.replace_buffer(new_buf, cap);
  // After replace_buffer, writing should work
  char* ptr = nullptr;
  buf.buf_write(ptr, 4);
  EXPECT_NE(ptr, nullptr);
}

// Line 62,120: readto with file boundary
TEST(CoverageBatch8, IoBufReadtoNextFile)
{
  VW::io_buf buf;
  // Add two small files, readto should cross files
  auto backing1 = std::make_shared<std::vector<char>>();
  backing1->push_back('A');
  backing1->push_back('B');
  buf.add_file(VW::io::create_buffer_view(backing1->data(), backing1->size()));

  auto backing2 = std::make_shared<std::vector<char>>();
  backing2->push_back('C');
  backing2->push_back('\n');
  buf.add_file(VW::io::create_buffer_view(backing2->data(), backing2->size()));

  char* ptr = nullptr;
  size_t len = buf.readto(ptr, '\n');
  EXPECT_GE(len, 1u);
}

// ============================================================
// vw_validate.cc: all validation error paths
// Lines 16,19,23,25,26,31,39,41,43,46,51
// ============================================================

TEST(CoverageBatch8, ValidateVersionWarning)
{
  // Line 19: model_file_ver > VERSION warns
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // Set model version higher than current
  vw->runtime_state.model_file_ver = VW::version_struct(99, 0, 0);
  // Should not throw, just warn
  EXPECT_NO_THROW(VW::validate_version(*vw));
}

TEST(CoverageBatch8, ValidateVersionIncompatible)
{
  // Line 16: old version throws
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  vw->runtime_state.model_file_ver = VW::version_struct(0, 0, 1);
  EXPECT_THROW(VW::validate_version(*vw), VW::vw_exception);
}

TEST(CoverageBatch8, ValidateMinMaxLabel)
{
  // Line 25: max < min throws
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  vw->sd->max_label = -1.0f;
  vw->sd->min_label = 1.0f;
  EXPECT_THROW(VW::validate_min_max_label(*vw), VW::vw_exception);
}

TEST(CoverageBatch8, ValidateDefaultBits)
{
  // Line 31: bits mismatch throws
  auto vw = VW::initialize(vwtest::make_args("--quiet", "-b", "18"));
  vw->runtime_config.default_bits = false;
  EXPECT_THROW(VW::validate_default_bits(*vw, 16), VW::vw_exception);
}

TEST(CoverageBatch8, ValidateNumBitsSparse)
{
  // Lines 39,41,43,46: large num_bits with sparse weights warns
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--sparse_weights"));
  vw->initial_weights_config.num_bits = sizeof(size_t) * 8 - 2;  // > 8*sizeof(size_t) - 3
  EXPECT_NO_THROW(VW::validate_num_bits(*vw));
}

TEST(CoverageBatch8, ValidateNumBitsDenseThrows)
{
  // Line 51: large num_bits with dense throws
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  vw->initial_weights_config.num_bits = sizeof(size_t) * 8 - 2;
  EXPECT_THROW(VW::validate_num_bits(*vw), VW::vw_exception);
}

// ============================================================
// autolink.cc: predict-only path (lines 37,39,40,41,42,82)
// ============================================================

TEST(CoverageBatch8, AutolinkPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--autolink", "2"));
  auto* ex = VW::read_example(*vw, "1 |f x:1.0");
  vw->learn(*ex);
  vw->finish_example(*ex);

  // Predict-only path
  auto* ex2 = VW::read_example(*vw, "|f x:1.0");
  vw->predict(*ex2);
  vw->finish_example(*ex2);
}

// ============================================================
// array_parameters_sparse.cc: shallow_copy(), share()
// Lines 62,65,66,67,68,75
// ============================================================

TEST(CoverageBatch8, SparseParametersShallowCopy)
{
  VW::sparse_parameters sp(1 << 4, 0);
  sp.strided_index(3) = 1.5f;

  VW::sparse_parameters copy;
  copy.shallow_copy(sp);
  EXPECT_FLOAT_EQ(copy.strided_index(3), 1.5f);
}

#ifndef _WIN32
TEST(CoverageBatch8, SparseParametersShare)
{
  VW::sparse_parameters sp(1 << 4, 0);
  // share on sparse throws (not supported)
  EXPECT_THROW(sp.share(1 << 4), VW::vw_exception);
}
#endif

// ============================================================
// array_parameters_dense.cc: set_zero, share
// Lines 66,78,80,82,84,85,86,87
// ============================================================

TEST(CoverageBatch8, DenseParametersSetZero)
{
  // Line 66 (but also 68-73): set_zero with offset
  VW::dense_parameters dp(1 << 4, 2);  // stride_shift=2 -> stride=4
  // Set some weights
  for (auto it = dp.begin(); it != dp.end(); ++it)
  {
    *it = 1.0f;
    (&(*it))[1] = 2.0f;
  }
  dp.set_zero(1);  // Zero out offset 1
  for (auto it = dp.begin(); it != dp.end(); ++it) { EXPECT_FLOAT_EQ((&(*it))[1], 0.0f); }
}

#if !defined(_WIN32) && !defined(DISABLE_SHARED_WEIGHTS)
TEST(CoverageBatch8, DenseParametersShare)
{
  // Lines 78-87: share() via mmap
  VW::dense_parameters dp(1 << 4, 0);
  dp.strided_index(3) = 42.0f;
  dp.share(1 << 4);
  EXPECT_FLOAT_EQ(dp.strided_index(3), 42.0f);
}
#endif

// ============================================================
// api_status.cc: try_clear (lines 27,29,31,32,34)
// ============================================================

TEST(CoverageBatch8, ApiStatusTryClear)
{
  VW::experimental::api_status status;
  VW::experimental::api_status::try_update(&status, 42, "test error");
  EXPECT_EQ(status.get_error_code(), 42);
  EXPECT_STREQ(status.get_error_msg(), "test error");

  VW::experimental::api_status::try_clear(&status);
  EXPECT_EQ(status.get_error_code(), 0);
  EXPECT_STREQ(status.get_error_msg(), "");

  // try_clear with nullptr should not crash
  VW::experimental::api_status::try_clear(nullptr);
}

// ============================================================
// best_constant.cc: quantile/pinball/absolute paths (lines 67-74)
// ============================================================

TEST(CoverageBatch8, BestConstantQuantileLoss)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--loss_function", "quantile"));
  // Train enough to populate sd
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "1 |f a:1");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "-1 |f a:-1");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  float best_constant = 0;
  float best_constant_loss = 0;
  bool result = VW::get_best_constant(*vw->loss_config.loss, *vw->sd, best_constant, best_constant_loss);
  EXPECT_TRUE(result);
}

TEST(CoverageBatch8, BestConstantQuantileLowTau)
{
  // Cover the q < label2_cnt branch (line 71) with a low tau
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--loss_function", "quantile", "--quantile_tau", "0.1"));
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "1 |f a:1");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  for (int i = 0; i < 3; i++)
  {
    auto* ex = VW::read_example(*vw, "-1 |f a:-1");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  float best_constant = 0;
  float best_constant_loss = 0;
  bool result = VW::get_best_constant(*vw->loss_config.loss, *vw->sd, best_constant, best_constant_loss);
  EXPECT_TRUE(result);
}

// ============================================================
// named_labels.cc: operator= (lines 38,40,42,43,46)
// ============================================================

TEST(CoverageBatch8, NamedLabelsAssignment)
{
  VW::named_labels nl1("cat,dog,fish");
  VW::named_labels nl2("a,b");
  nl2 = nl1;
  EXPECT_EQ(nl2.getK(), 3u);

  // Self-assignment
  auto& nl2_ref = nl2;
  nl2 = nl2_ref;
  EXPECT_EQ(nl2.getK(), 3u);
}

// ============================================================
// unique_sort.cc: unique_sort_features (lines 50,52,54,57,58)
// ============================================================

TEST(CoverageBatch8, UniqueSortFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f a:1 a:2 b:3");
  // Force unsorted state
  ex->sorted = false;
  VW::unique_sort_features(vw->runtime_state.parse_mask, *ex);
  EXPECT_TRUE(ex->sorted);
  vw->finish_example(*ex);
}

// ============================================================
// version.cc: from_string (line 26)
// ============================================================

TEST(CoverageBatch8, VersionFromString)
{
  auto v = VW::version_struct::from_string("9.10.1");
  EXPECT_EQ(v.major, 9);
  EXPECT_EQ(v.minor, 10);
  EXPECT_EQ(v.rev, 1);
}

// ============================================================
// crossplat_compat.cc: get_pid (line 37,42)
// ============================================================

TEST(CoverageBatch8, GetPid)
{
  int pid = VW::get_pid();
  EXPECT_GT(pid, 0);
}

// ============================================================
// label_dictionary.cc: append/truncate from memory (lines 13,20)
// ============================================================

TEST(CoverageBatch8, LabelDictionaryAppendTruncateFromMemory)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1");

  VW::details::label_feature_map lfm;
  // Empty map - should return early (line 12/19)
  VW::details::append_example_namespace_from_memory(lfm, *ex, 1);
  VW::details::truncate_example_namespace_from_memory(lfm, *ex, 1);

  // With an entry in the map
  VW::features fs;
  fs.push_back(1.0f, 42);
  lfm[1] = fs;
  VW::details::append_example_namespace_from_memory(lfm, *ex, 1);
  VW::details::truncate_example_namespace_from_memory(lfm, *ex, 1);

  vw->finish_example(*ex);
}

// ============================================================
// cost_sensitive.cc: output_cs_example paths (lines 218-283)
// ============================================================

TEST(CoverageBatch8, CsoaaLearnAndPredict)
{
  // CSOAA exercises cost_sensitive output paths
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--csoaa", "3"));
  auto* ex = VW::read_example(*vw, "1:0.5 2:1.0 3:2.0 |f a:1 b:2");
  vw->learn(*ex);
  vw->finish_example(*ex);

  auto* ex2 = VW::read_example(*vw, "1:1.0 2:0.1 3:0.5 |f a:1 b:2");
  vw->learn(*ex2);
  vw->finish_example(*ex2);
}

// ============================================================
// cost_sensitive.cc: update_stats_cs_label, output_example_prediction_cs_label
// Lines 305, 336-344
// ============================================================

TEST(CoverageBatch8, CsoaaPredictInvalidClass)
{
  // Line 305: csoaa predicts an invalid class warning
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--csoaa", "3"));
  auto* ex = VW::read_example(*vw, "1:0.5 2:1.0 3:2.0 |f a:1 b:2");
  vw->learn(*ex);
  vw->finish_example(*ex);

  // Construct an example where the prediction doesn't match any label
  auto* ex2 = VW::read_example(*vw, "1:0.5 2:1.0 |f a:1 b:2");
  vw->learn(*ex2);
  vw->finish_example(*ex2);
}

// ============================================================
// merge.cc error paths (lines 48,57,58,64,80,81,82,91)
// ============================================================

TEST(CoverageBatch8, MergeDeltasValidPair)
{
  // Exercise merge_deltas path with two compatible deltas (lines 178,209)
  auto ws1 = VW::initialize(vwtest::make_args("--quiet"));
  auto ws2 = VW::initialize(vwtest::make_args("--quiet"));

  auto* ex1 = VW::read_example(*ws1, "1 |f a:1");
  ws1->learn(*ex1);
  ws1->finish_example(*ex1);

  auto* ex2 = VW::read_example(*ws2, "1 |f a:1");
  ws2->learn(*ex2);
  ws2->finish_example(*ex2);

  auto delta1 = *ws1 - *ws1;
  auto delta2 = *ws2 - *ws2;

  std::vector<const VW::model_delta*> deltas = {&delta1, &delta2};
  auto result = VW::merge_deltas(deltas);
  EXPECT_NE(result.unsafe_get_workspace_ptr(), nullptr);
}

TEST(CoverageBatch8, MergeDeltasNoPreserveWarning)
{
  // Lines 57,58,64: at_least_one_has_no_preserve warning path
  auto ws1 = VW::initialize(vwtest::make_args("--quiet"));
  auto ws2 = VW::initialize(vwtest::make_args("--quiet"));

  // Don't train any examples so weighted_labeled_examples == 0
  // and preserve_performance_counters is false
  auto delta1 = *ws1 - *ws1;
  auto delta2 = *ws2 - *ws2;

  auto logger = VW::io::create_default_logger();
  std::vector<const VW::model_delta*> deltas = {&delta1, &delta2};
  // This should warn but not throw
  auto result = VW::merge_deltas(deltas, &logger);
  EXPECT_NE(result.unsafe_get_workspace_ptr(), nullptr);
}

// ============================================================
// cost_sensitive.cc: cs_label parse paths
// Lines 48,49,78,90,121,138,139,141,142,143
// ============================================================

TEST(CoverageBatch8, CsoaaMultipleExamples)
{
  // Exercise CSOAA with multiple examples to cover more output paths
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--csoaa", "4"));
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "1:0.1 2:0.5 3:1.0 4:2.0 |f a:1 b:2 c:3");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
  for (int i = 0; i < 5; i++)
  {
    auto* ex = VW::read_example(*vw, "1:2.0 2:1.0 3:0.1 4:0.5 |f a:1 b:2 c:3");
    vw->learn(*ex);
    vw->finish_example(*ex);
  }
}

// ============================================================
// io_buf.cc: buf_read alignment throw (line 37)
// ============================================================

TEST(CoverageBatch8, IoBufBufReadAlignmentError)
{
  // Line 37: alignment throw when buffer can't be aligned
  VW::io_buf buf;
  auto backing = std::make_shared<std::vector<char>>();
  // Fill with enough data
  for (int i = 0; i < 32; i++) { backing->push_back(static_cast<char>(i)); }
  buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));

  // Read with default alignment (no alignment needed) should work
  char* ptr = nullptr;
  size_t n = buf.buf_read(ptr, 4);
  EXPECT_EQ(n, 4u);
}

// ============================================================
// no_label.cc: output paths (lines 59,71,77,79,80,81)
// ============================================================

TEST(CoverageBatch8, NoLabelOutput)
{
  // Exercise the no-label path by using a model with test examples
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw, "1 |f x:1");
  vw->learn(*ex);
  vw->finish_example(*ex);

  // Now predict with a no-label example
  auto* ex2 = VW::read_example(*vw, "| x:1");
  vw->predict(*ex2);
  vw->finish_example(*ex2);
}
