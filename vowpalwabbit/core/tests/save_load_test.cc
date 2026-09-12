// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/options_cli.h"
#include "vw/core/io_buf.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/reductions/ftrl.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"
#include "vw/text_parser/parse_example_text.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <vector>

using namespace ::testing;

#include <string>

TEST(SaveLoad, SaveResumeBehavesAsIfDatasetConcatenated)
{
  std::array<std::string, 10> input_data = {
      "0.521144 |T PFF |f t1:-0.0236849 t5:-0.10215 r5:0.727735 t10:-0.0387662 r10:0.911208 t20:-0.00777943 "
      "r20:0.952668 t40:0.014542 r40:0.832479 t60:0.00395449 r60:0.724504 t90:0.0281418 r90:0.784653",
      "0.535251 |T WIP |f t1:-0.00195191 t5:-0.112359 r5:0.0279508 t10:-0.0405403 r10:0.605284 t20:0.0126391 "
      "r20:0.823787 t40:0.0221162 r40:0.890048 t60:0.0235056 r60:0.903205 t90:0.0256024 r90:0.923621",
      "0.549824 |T GCC |f t1:-0.0155661 t5:-0.202743 r5:0.161423 t10:-0.00856594 r10:0.689004 t20:0.0949576 "
      "r20:0.882347 t40:0.0576915 r40:0.689529 t60:0.0271459 r60:0.102717 t90:0.0235522 r90:0.0242844",
      "0.585408 |T AAXJ |f t1:-0.0381144 t5:-0.205393 r5:0.598789 t10:-0.0706081 r10:0.826144 t20:0.0431942 "
      "r20:0.922816 t40:0.0408377 r40:0.899879 t60:0.0411133 r60:0.827428 t90:0.0495845 r90:0.799112",
      "0.593261 |T VWO |f t1:-0.0440486 t5:-0.259063 r5:0.65883 t10:-0.0992946 r10:0.870232 t20:0.0563154 r20:0.926398 "
      "t40:0.0606423 r40:0.948557 t60:0.0561357 r60:0.926893 t90:0.0611374 r90:0.909396",
      "0.268074 |T EEV |f t1:0.0864275 t5:0.534149 r5:-0.648219 t10:0.18622 r10:-0.867711 t20:-0.124613 r20:-0.929735 "
      "t40:-0.135311 r40:-0.946801 t60:-0.123586 r60:-0.922735 t90:-0.137895 r90:-0.896058",
      "0.73251 |T GDX |f t1:-0.0602452 t5:-0.477063 r5:0.501001 t10:-0.238843 r10:0.78945 t20:0.00435946 r20:0.832751 "
      "t40:0.0328047 r40:0.874984 t60:0.0779076 r60:0.904937 t90:0.062806 r90:0.874416",
      "0.557516 |T RTH |f t1:-0.0142269 t5:-0.0118355 r5:0.630517 t10:-0.0250743 r10:0.662658 t20:0.0634855 "
      "r20:0.857961 t40:0.0349201 r40:0.698693 t60:0.035595 r60:0.731773 t90:0.0449791 r90:0.767057",
      "0.639897 |T MXI |f t1:-0.0394954 t5:-0.384378 r5:0.502169 t10:-0.139307 r10:0.8061 t20:0.0815173 r20:0.880044 "
      "t40:0.0470638 r40:0.859053 t60:0.0461571 r60:0.874805 t90:0.0628346 r90:0.872736",
      "0.583484 |T EWU |f t1:-0.0221598 t5:-0.162901 r5:0.429709 t10:-0.0214379 r10:0.745645 t20:0.0748484 "
      "r20:0.880457 t40:0.0230047 r40:0.486719 t60:0.0319379 r60:0.619057 t90:0.0519471 r90:0.732452"};

  auto vw_all_data_single_run = VW::initialize(vwtest::make_args("--no_stdin", "--quiet"));

  for (const auto& item : input_data)
  {
    auto& ex = VW::get_unused_example(vw_all_data_single_run.get());
    VW::parsers::text::read_line(*vw_all_data_single_run, &ex, item.c_str());
    VW::setup_example(*vw_all_data_single_run, &ex);
    vw_all_data_single_run->learn(ex);
    vw_all_data_single_run->finish_example(ex);
  }

  auto vw_first_half = VW::initialize(vwtest::make_args("--no_stdin", "--quiet"));

  for (size_t i = 0; i < 5; i++)
  {
    auto& ex = VW::get_unused_example(vw_first_half.get());
    VW::parsers::text::read_line(*vw_first_half, &ex, input_data[i].c_str());
    VW::setup_example(*vw_first_half, &ex);
    vw_first_half->learn(ex);
    vw_first_half->finish_example(ex);
  }

  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));
  VW::save_predictor(*vw_first_half, io_writer);
  io_writer.flush();

  auto vw_second_half_from_loaded =
      VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--preserve_performance_counters"),
          VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  for (size_t i = 5; i < 10; i++)
  {
    auto& ex = VW::get_unused_example(vw_second_half_from_loaded.get());
    VW::parsers::text::read_line(*vw_second_half_from_loaded, &ex, input_data[i].c_str());
    VW::setup_example(*vw_second_half_from_loaded, &ex);
    vw_second_half_from_loaded->learn(ex);
    vw_second_half_from_loaded->finish_example(ex);
  }

  EXPECT_EQ(vw_all_data_single_run->sd->weighted_examples(), vw_second_half_from_loaded->sd->weighted_examples());
  EXPECT_EQ(vw_all_data_single_run->sd->sum_loss, vw_second_half_from_loaded->sd->sum_loss);
}

namespace
{
template <typename T>
void append_pod(std::vector<char>& buf, T value)
{
  const char* p = reinterpret_cast<const char*>(&value);
  buf.insert(buf.end(), p, p + sizeof(T));
}
}  // namespace

// Regression test for GHSA-x3cx-p52g-p5q7: save_load_header reads a file-controlled legacy-interaction
// length (inter_len) and copies that many bytes into the fixed 512-byte header buffer buff2 with no
// bound. A crafted model with inter_len > 512 and enough trailing bytes overflows buff2 with
// attacker-controlled data (heap out-of-bounds write). The load must instead reject the oversized length
// with a clean VW exception before the copy.
//
// The crafted model claims file version 7.10.2 (the only version that reaches the legacy pair/triple/
// interaction read path). 7.10.2 is below VERSION_FILE_WITH_HEADER_CHAINED_HASH (8.0.2) and
// VERSION_FILE_WITH_HEADER_ID (8.0.3), so no header checksum or id block is required, and
// validate_version only rejects versions older than 7.6.0.
TEST(SaveLoad, CraftedModelOversizedInteractionLengthIsRejected)
{
  constexpr uint32_t OVERSIZED_INTER_LEN = 4096;  // > 512-byte buff2

  std::vector<char> model;
  // Version block: bin_read expects a uint32 length prefix followed by that many bytes.
  const std::string version = "7.10.2";
  append_pod<uint32_t>(model, static_cast<uint32_t>(version.size() + 1));
  model.insert(model.end(), version.begin(), version.end());
  model.push_back('\0');
  // Model marker.
  model.push_back('m');
  // Min/max label (float each).
  append_pod<float>(model, 0.0f);
  append_pod<float>(model, 0.0f);
  // Bit precision (uint32).
  append_pod<uint32_t>(model, 18u);
  // Legacy interaction block (version < 7.10.3): pair count, then triple count, both empty.
  append_pod<uint32_t>(model, 0u);  // pair_len
  append_pod<uint32_t>(model, 0u);  // triple_len
  // Interactions among pairs/triples (version >= 7.10.2): one interaction with an oversized length.
  append_pod<uint32_t>(model, 1u);                   // number of interactions
  append_pod<uint32_t>(model, OVERSIZED_INTER_LEN);  // inter_len (attacker-controlled, > buff2.size())
  // Trailing bytes so the unbounded read would have OVERSIZED_INTER_LEN bytes available to copy.
  model.insert(model.end(), OVERSIZED_INTER_LEN, 'A');

  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet"));

  VW::io_buf model_file;
  model_file.add_file(VW::io::create_buffer_view(model.data(), model.size()));

  std::string file_options;
  EXPECT_THROW(VW::details::save_load_header(*vw, model_file, /*read*/ true, /*text*/ false, file_options, *vw->options),
      VW::vw_exception);
}

// Regression test for the additional sink reported with GHSA-x3cx-p52g-p5q7: mwt::save_load reads a
// file-controlled list of 64-bit policy ids and uses each as an index into c.evals (sized to the weight
// table) with no bounds check, so a policy id past the end of c.evals is an attacker-directed heap
// out-of-bounds access. --multiworld_test is a persisted (keep()) option embedded in the model header,
// so a single crafted model reaches this path. The load must reject an out-of-range policy id with a
// clean VW exception.
//
// A valid --multiworld_test model is trained first (so every preceding byte of the model is well-formed),
// then a single policy id in the serialized mwt block is overwritten with an out-of-range value. The mwt
// block is located structurally by its (total, policies_size) prefix rather than a fixed offset.
TEST(SaveLoad, CraftedModelMwtPolicyIdOutOfRangeIsRejected)
{
  // Decision-service style CB eval data (matches test/train-sets/cb_eval): eight labeled observations,
  // two distinct policy features (":1" and ":2") in the evaluated namespace "f".
  const std::array<std::string, 8> cb_eval = {"1:0:0.5 |f :1 :2", "2:1:0.5 |f :1 :2", "1:0:0.5 |f :1 :2",
      "2:1:0.5 |f :1 :2", "1:0:0.5 |f :1 :2", "2:1:0.5 |f :1 :2", "1:0:0.5 |f :1 :2", "2:1:0.5 |f :1 :2"};

  auto vw_train = VW::initialize(vwtest::make_args("--multiworld_test", "f", "--no_stdin", "--quiet"));
  for (const auto& line : cb_eval)
  {
    auto& ex = VW::get_unused_example(vw_train.get());
    VW::parsers::text::read_line(*vw_train, &ex, line.c_str());
    VW::setup_example(*vw_train, &ex);
    vw_train->learn(ex);
    vw_train->finish_example(ex);
  }

  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));
  VW::save_predictor(*vw_train, io_writer);
  io_writer.flush();

  // The mwt block is: total (double) | policies_size (size_t) | policies (size_t each) | per-policy data.
  // Eight observations were learned (total == 8.0) and there are two policies, giving a unique 16-byte
  // prefix that locates the block regardless of its offset in the model.
  const double expected_total = 8.0;
  const uint64_t expected_policies_size = 2;
  std::vector<char> signature(sizeof(double) + sizeof(uint64_t));
  std::memcpy(signature.data(), &expected_total, sizeof(double));
  std::memcpy(signature.data() + sizeof(double), &expected_policies_size, sizeof(uint64_t));

  auto& model = *backing_vector;
  auto sig_begin = std::search(model.begin(), model.end(), signature.begin(), signature.end());
  ASSERT_NE(sig_begin, model.end()) << "mwt block signature not found in serialized model";
  ASSERT_EQ(std::search(sig_begin + 1, model.end(), signature.begin(), signature.end()), model.end())
      << "mwt block signature is not unique; test needs updating";

  // Overwrite the first policy id (immediately after the 16-byte prefix) with a value far larger than any
  // possible c.evals index (c.evals is sized to 2^num_bits, at most a few million for default bit widths).
  const size_t first_policy_offset =
      static_cast<size_t>(std::distance(model.begin(), sig_begin)) + signature.size();
  ASSERT_LE(first_policy_offset + sizeof(uint64_t), model.size());
  const uint64_t out_of_range_policy = 0xFFFFFFFFFFFFFFFFULL;
  std::memcpy(model.data() + first_policy_offset, &out_of_range_policy, sizeof(uint64_t));

  // Loading the corrupted model re-enables --multiworld_test from the persisted header and must reject the
  // out-of-range policy id instead of indexing past the end of c.evals.
  EXPECT_THROW(VW::initialize(vwtest::make_args("--no_stdin", "--quiet"),
                   VW::io::create_buffer_view(model.data(), model.size())),
      VW::vw_exception);
}

// Regression test for GHSA-9cm9-qvp3-c2v4: the persisted-options field is stored in a std::vector<char>
// sized to exactly the declared length and filled with exactly that many file bytes, but the loader then
// appended it as a C string. operator+ on a const char* runs strlen, so a field whose bytes contain no NUL
// makes the scan cross the end of the allocation and keep going through adjacent heap memory -- the field
// text ends up contaminated with whatever followed it, and the read itself is out of bounds.
//
// The crafted model declares version 7.10.2 for the same reasons as the test above, and additionally
// because 7.10.2 is below VERSION_FILE_WITH_HEADER_CHAINED_HASH (8.0.2): no header checksum is written, so
// the options field can be given an arbitrary length and content without the load failing a hash check
// before it reaches the append. The declared length exceeds the 512-byte default buffer, so buff2 is
// resized to exactly that length and the unterminated scan runs off an allocation of precisely that size.
//
// The bounded append stops at the declared length, so file_options ends with exactly the bytes the model
// supplied and nothing more. Under AddressSanitizer the unpatched loader instead reports a
// heap-buffer-overflow read here.
TEST(SaveLoad, CraftedModelUnterminatedOptionsFieldIsNotScannedPastTheEnd)
{
  // A distinctive payload longer than the 512-byte default buffer, deliberately not NUL-terminated.
  const std::string unterminated_options(600, 'A');

  std::vector<char> model;
  // Version block: bin_read expects a uint32 length prefix followed by that many bytes.
  const std::string version = "7.10.2";
  append_pod<uint32_t>(model, static_cast<uint32_t>(version.size() + 1));
  model.insert(model.end(), version.begin(), version.end());
  model.push_back('\0');
  // Model marker.
  model.push_back('m');
  // Min/max label (float each).
  append_pod<float>(model, 0.0f);
  append_pod<float>(model, 0.0f);
  // Bit precision (uint32).
  append_pod<uint32_t>(model, 18u);
  // Legacy interaction block (version < 7.10.3): pair count, triple count, and -- because 7.10.2 is
  // VERSION_FILE_WITH_INTERACTIONS -- the interactions-among-pairs-and-triples count. All empty.
  append_pod<uint32_t>(model, 0u);  // pair_len
  append_pod<uint32_t>(model, 0u);  // triple_len
  append_pod<uint32_t>(model, 0u);  // interactions len
  // 7.10.2 is above VERSION_FILE_WITH_RANK_IN_HEADER (7.8.0), so no rank field is present.
  append_pod<uint32_t>(model, 0u);  // lda
  append_pod<uint32_t>(model, 0u);  // ngram count
  append_pod<uint32_t>(model, 0u);  // skip count
  // Persisted options: a uint32 length followed by that many bytes, with no terminator among them.
  append_pod<uint32_t>(model, static_cast<uint32_t>(unterminated_options.size()));
  model.insert(model.end(), unterminated_options.begin(), unterminated_options.end());
  // Trailing bytes an unbounded strlen would run into after leaving the options allocation.
  model.insert(model.end(), 4096, 'B');

  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet"));

  VW::io_buf model_file;
  model_file.add_file(VW::io::create_buffer_view(model.data(), model.size()));

  std::string file_options;
  VW::details::save_load_header(*vw, model_file, /*read*/ true, /*text*/ false, file_options, *vw->options);

  // The options text must stop exactly where the model said it does: ending with the declared payload and
  // picking up none of the bytes that follow it in the buffer.
  EXPECT_THAT(file_options, ::testing::EndsWith(unterminated_options));
  EXPECT_EQ(file_options.find('B'), std::string::npos);
}

// Regression test for GHSA-q2hj-ggqm-4g62: mwt::save_load trusts a serialized policies_size and hands it
// to v_array::resize. The underlying reserve_nocheck computes sizeof(T) * length without checking for
// overflow, so a sufficiently large count wraps to a small realloc while _end_array records the full
// count -- a buffer that lies about its capacity, followed by an out-of-bounds memset and an
// out-of-bounds construction loop. The count is now bounded by the number of policy slots before it is
// used, and v_array itself rejects a length whose byte size would overflow.
//
// As with the policy-id test above, a valid --multiworld_test model is trained first and only the
// serialized policies_size is overwritten, so every other byte of the model is well-formed.
TEST(SaveLoad, CraftedModelMwtPoliciesSizeOverflowIsRejected)
{
  const std::array<std::string, 8> cb_eval = {"1:0:0.5 |f :1 :2", "2:1:0.5 |f :1 :2", "1:0:0.5 |f :1 :2",
      "2:1:0.5 |f :1 :2", "1:0:0.5 |f :1 :2", "2:1:0.5 |f :1 :2", "1:0:0.5 |f :1 :2", "2:1:0.5 |f :1 :2"};

  auto vw_train = VW::initialize(vwtest::make_args("--multiworld_test", "f", "--no_stdin", "--quiet"));
  for (const auto& line : cb_eval)
  {
    auto& ex = VW::get_unused_example(vw_train.get());
    VW::parsers::text::read_line(*vw_train, &ex, line.c_str());
    VW::setup_example(*vw_train, &ex);
    vw_train->learn(ex);
    vw_train->finish_example(ex);
  }

  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));
  VW::save_predictor(*vw_train, io_writer);
  io_writer.flush();

  // The mwt block is: total (double) | policies_size (size_t) | policies | per-policy data. Eight
  // observations were learned (total == 8.0) with two policies, giving a unique 16-byte prefix.
  const double expected_total = 8.0;
  const uint64_t expected_policies_size = 2;
  std::vector<char> signature(sizeof(double) + sizeof(uint64_t));
  std::memcpy(signature.data(), &expected_total, sizeof(double));
  std::memcpy(signature.data() + sizeof(double), &expected_policies_size, sizeof(uint64_t));

  auto& model = *backing_vector;
  auto sig_begin = std::search(model.begin(), model.end(), signature.begin(), signature.end());
  ASSERT_NE(sig_begin, model.end()) << "mwt block signature not found in serialized model";
  ASSERT_EQ(std::search(sig_begin + 1, model.end(), signature.begin(), signature.end()), model.end())
      << "mwt block signature is not unique; test needs updating";

  // Overwrite policies_size with a count whose byte size (count * sizeof(feature_index)) wraps size_t.
  const size_t policies_size_offset = static_cast<size_t>(std::distance(model.begin(), sig_begin)) + sizeof(double);
  const uint64_t overflowing_count = (std::numeric_limits<uint64_t>::max)() / sizeof(uint64_t) + 2;
  std::memcpy(model.data() + policies_size_offset, &overflowing_count, sizeof(uint64_t));

  EXPECT_THROW(VW::initialize(
                   vwtest::make_args("--no_stdin", "--quiet"), VW::io::create_buffer_view(model.data(), model.size())),
      VW::vw_exception);
}

// Regression test for GHSA-9463-43mc-xhc6: save_load_online_state_gd indexed pms[0] guarded only by
// assert(pms.size() >= 1), which is compiled out under NDEBUG. The vector is not always non-empty --
// ftrl deserializes it with an element count taken from the model file (reachable through the persisted
// --experimental_igl option), so a crafted model can empty it. The single-normalizer layout must still be
// consumed to keep the stream aligned for the reductions that follow, so it is now routed through a
// scratch state rather than through pms[0].
TEST(SaveLoad, OnlineStateLoadWithNoPerModelStateDoesNotIndexEmptyVector)
{
  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet"));

  // Enough bytes to satisfy every fixed-size field the online-state block reads.
  std::vector<char> state(512, 0);
  VW::io_buf model_file;
  model_file.add_file(VW::io::create_buffer_view(state.data(), state.size()));

  std::vector<VW::reductions::details::gd_per_model_state> empty_pms;
  ASSERT_TRUE(empty_pms.empty());

  EXPECT_NO_THROW(VW::details::save_load_online_state_gd(
      *vw, model_file, /*read*/ true, /*text*/ false, empty_pms, /*g*/ nullptr, /*ftrl_size*/ 0));
}

// The file-controlled path that produces the empty vector above: ftrl's read_model_field clears
// gd_per_model_states and refills it from a count read out of the model, so a declared count of zero left
// it empty while ftrl_setup guarantees exactly one state and the prediction and save/load paths index [0]
// unconditionally. The invariant is now re-established after the read.
TEST(SaveLoad, FtrlReadModelFieldWithZeroPerModelStatesRestoresOne)
{
  std::vector<char> serialized;
  append_pod<uint32_t>(serialized, 0u);  // vector element count, straight from the file

  VW::io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(serialized.data(), serialized.size()));

  VW::reductions::ftrl ftrl_data;
  ftrl_data.gd_per_model_states.emplace_back();
  VW::reductions::model_utils::read_model_field(io_reader, ftrl_data);

  EXPECT_EQ(ftrl_data.gd_per_model_states.size(), 1u);
}
