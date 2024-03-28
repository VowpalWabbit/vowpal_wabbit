#include "data.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "vw/common/random_details.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/array_parameters_sparse.h"
#include "vw/slim/example_predict_builder.h"

#include <stdlib.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <set>
#include <streambuf>
#include <vector>

using namespace ::testing;
using namespace vw_slim;

// #define VW_SLIM_TEST_DEBUG "vwslim-debug.log"

// some gymnastics to re-use the float reading code
class membuf : public std::streambuf
{
public:
  membuf(char* begin, char* end) { this->setg(begin, begin, end); }
};

std::vector<float> read_floats(std::istream& data)
{
  std::vector<float> floats;

  std::string line;
  while (std::getline(data, line)) { floats.push_back((float)atof(line.c_str())); }

  return floats;
}
std::vector<float> read_floats(unsigned char* data, unsigned int len)
{
  membuf mb((char*)data, (char*)(data + len));
  std::istream in(&mb);
  return read_floats(in);
}

std::vector<float> read_floats(const char* filename)
{
  std::ifstream data(filename);
  return read_floats(data);
}

class test_data
{
public:
  unsigned char* model;
  unsigned int model_len;
  unsigned char* pred;
  unsigned int pred_len;
};

#define TEST_DATA(input, filename)         \
  {                                        \
    if (!strcmp(input, #filename))         \
    {                                      \
      td.model = filename##_model;         \
      td.model_len = filename##_model_len; \
      td.pred = filename##_pred;           \
      td.pred_len = filename##_pred_len;   \
    }                                      \
  }

test_data get_test_data(const char* model_filename)
{
  test_data td;
  memset(&td, 0, sizeof(test_data));

  TEST_DATA(model_filename, regression_data_1);
  TEST_DATA(model_filename, regression_data_2);
  TEST_DATA(model_filename, regression_data_3);
  TEST_DATA(model_filename, regression_data_3);
  TEST_DATA(model_filename, regression_data_4);
  TEST_DATA(model_filename, regression_data_5);
  TEST_DATA(model_filename, regression_data_6);
  TEST_DATA(model_filename, regression_data_7);
  TEST_DATA(model_filename, regression_data_no_constant);
  TEST_DATA(model_filename, regression_data_ignore_linear);
  TEST_DATA(model_filename, multiclass_data_4);
  TEST_DATA(model_filename, multiclass_data_5);
  TEST_DATA(model_filename, cb_data_5);
  TEST_DATA(model_filename, cb_data_6);
  TEST_DATA(model_filename, cb_data_7);
  TEST_DATA(model_filename, cb_data_8);

  return td;
}

template <typename W>
void run_predict_in_memory(
    const char* model_filename, const char* data_filename, const char* /*prediction_reference_filename*/)
{
  std::vector<float> preds;

  vw_predict<W> vw;
  // if files would be available
  test_data td = get_test_data(model_filename);
  ASSERT_EQ(S_VW_PREDICT_OK, vw.load((const char*)td.model, td.model_len));
  EXPECT_FALSE(vw.is_cb_explore_adf());

  float score;
  if (!strcmp(data_filename, "regression_data_1.txt"))
  {
    VW::example_predict ex[2];
    // 1 |0 0:1
    example_predict_builder b0(&ex[0], (VW::namespace_index)0);
    b0.push_feature(0, 1.f);
    // 1 |0 0:5
    example_predict_builder b1(&ex[1], (VW::namespace_index)0);
    b1.push_feature(0, 5.f);

    for (auto& e : ex)
    {
      ASSERT_EQ(S_VW_PREDICT_OK, vw.predict(e, score));
      preds.push_back(score);
    }
  }
  else if (!strcmp(data_filename, "regression_data_2.txt"))
  {
    VW::example_predict ex[2];
    // 1 |0 0:1 |a 0:2
    example_predict_builder b00(&ex[0], (VW::namespace_index)0);
    b00.push_feature(0, 1.f);
    example_predict_builder b0a(&ex[0], "a");
    b0a.push_feature(0, 2.f);
    // 0 |c 0:3
    example_predict_builder b1(&ex[1], "c");
    b1.push_feature(0, 3.f);

    for (auto& e : ex)
    {
      ASSERT_EQ(S_VW_PREDICT_OK, vw.predict(e, score));
      preds.push_back(score);
    }
  }
  else if (!strcmp(data_filename, "regression_data_3.txt"))
  {
    VW::example_predict ex[2];
    // 1 |a 0:1 |b 2:2
    example_predict_builder b0a(&ex[0], "a");
    b0a.push_feature(0, 1.f);
    example_predict_builder b0b(&ex[0], "b");
    b0b.push_feature(2, 2.f);
    // 0 |a 0:1 |b 2:4
    example_predict_builder b1a(&ex[1], "a");
    b1a.push_feature(0, 1.f);
    example_predict_builder b1b(&ex[1], "b");
    b1b.push_feature(2, 4.f);

    for (auto& e : ex)
    {
      ASSERT_EQ(S_VW_PREDICT_OK, vw.predict(e, score));
      preds.push_back(score);
    }
  }
  else if (!strcmp(data_filename, "regression_data_4.txt"))
  {
    VW::example_predict ex[2];
    // 1 |a 0:1 |b 2:2 |c 3:3 |d 4:4
    example_predict_builder b0a(&ex[0], "a");
    b0a.push_feature(0, 1.f);
    example_predict_builder b0b(&ex[0], "b");
    b0b.push_feature(2, 2.f);
    example_predict_builder b0c(&ex[0], "c");
    b0c.push_feature(3, 3.f);
    example_predict_builder b0d(&ex[0], "d");
    b0d.push_feature(4, 4.f);
    // 0 |a 0:1 |b 2:4 |c 3:1 |d 1:2
    example_predict_builder b1a(&ex[1], "a");
    b1a.push_feature(0, 1.f);
    example_predict_builder b1b(&ex[1], "b");
    b1b.push_feature(2, 4.f);
    example_predict_builder b1c(&ex[1], "c");
    b1c.push_feature(3, 1.f);
    example_predict_builder b1d(&ex[1], "d");
    b1d.push_feature(1, 2.f);

    for (auto& e : ex)
    {
      ASSERT_EQ(S_VW_PREDICT_OK, vw.predict(e, score));
      preds.push_back(score);
    }
  }
  else if (!strcmp(data_filename, "regression_data_7.txt"))
  {
    VW::example_predict ex[2];
    // 1 |a x:1 |b y:2
    example_predict_builder b0a(&ex[0], "a");
    b0a.push_feature_string("x", 1.f);
    example_predict_builder b0b(&ex[0], "b");
    b0b.push_feature_string("y", 2.f);
    // 0 |a x:1 |5 y:4
    example_predict_builder b1a(&ex[1], "a");
    b1a.push_feature_string("x", 1.f);
    example_predict_builder b1b(&ex[1], 5);
    b1b.push_feature_string("y", 4.f);

    for (auto& e : ex)
    {
      ASSERT_EQ(S_VW_PREDICT_OK, vw.predict(e, score));
      preds.push_back(score);
    }
  }
  else { FAIL() << "Unknown data file: " << data_filename; }

  // compare output
  std::vector<float> preds_expected = read_floats(td.pred, td.pred_len);

  EXPECT_THAT(preds, Pointwise(FloatNear(1e-4f), preds_expected));
}

enum class predict_param_weight_type
{
  ALL,
  SPARSE,
  DENSE
};

struct predict_param
{
public:
  const char* model_filename;
  const char* data_filename;
  const char* prediction_reference_filename;
  predict_param_weight_type weight_type;
};

// nice rendering in unit tests
::std::ostream& operator<<(::std::ostream& os, const predict_param& param)
{
  return os << param.model_filename << " " << param.data_filename << " "
            << (param.weight_type == predict_param_weight_type::SPARSE ? "sparse" : "dense");
}

struct predict_test : public ::testing::TestWithParam<predict_param>
{
};

TEST_P(predict_test, Run)
{
  if (GetParam().weight_type == predict_param_weight_type::SPARSE)
  {
    run_predict_in_memory<VW::sparse_parameters>(
        GetParam().model_filename, GetParam().data_filename, GetParam().prediction_reference_filename);
  }
  else
  {
    run_predict_in_memory<VW::dense_parameters>(
        GetParam().model_filename, GetParam().data_filename, GetParam().prediction_reference_filename);
  }
}

std::vector<predict_param> generate_test_params()
{
  std::vector<predict_param> fixtures;

  predict_param predict_params[] = {
      {"regression_data_1", "regression_data_1.txt", "regression_data_1.pred", predict_param_weight_type::ALL},
      {"regression_data_2", "regression_data_2.txt", "regression_data_2.pred", predict_param_weight_type::ALL},
      {"regression_data_no_constant", "regression_data_1.txt", "regression_data_no-constant.pred",
          predict_param_weight_type::ALL},
      {"regression_data_ignore_linear", "regression_data_2.txt", "regression_data_ignore_linear.pred",
          predict_param_weight_type::ALL},
      {"regression_data_3", "regression_data_3.txt", "regression_data_3.pred", predict_param_weight_type::ALL},
      {"regression_data_4", "regression_data_4.txt", "regression_data_4.pred", predict_param_weight_type::ALL},
      {"regression_data_5", "regression_data_4.txt", "regression_data_5.pred", predict_param_weight_type::ALL},
      {"regression_data_6", "regression_data_3.txt", "regression_data_6.pred", predict_param_weight_type::SPARSE},
      {"regression_data_7", "regression_data_7.txt", "regression_data_7.pred", predict_param_weight_type::ALL}};

  for (size_t i = 0; i < sizeof(predict_params) / sizeof(predict_param); i++)
  {
    predict_param p = predict_params[i];
    if (p.weight_type != predict_param_weight_type::ALL) { fixtures.push_back(p); }
    else
    {
      std::initializer_list<predict_param_weight_type> weight_types = {
          predict_param_weight_type::SPARSE, predict_param_weight_type::DENSE};
      for (auto weight_type : weight_types)
      {
        p.weight_type = static_cast<predict_param_weight_type>(weight_type);
        fixtures.push_back(p);
      }
    }
  }

  return fixtures;
}

INSTANTIATE_TEST_SUITE_P(VowpalWabbitSlim, predict_test, ::testing::ValuesIn(generate_test_params()));

struct invalid_model_param
{
public:
  const char* name;
  unsigned char* model;
  unsigned int model_len;
  std::set<size_t> undetectable_offsets;
  predict_param_weight_type weight_type;
};

// nice rendering in unit tests
::std::ostream& operator<<(::std::ostream& os, const invalid_model_param& param) { return os << param.name; }

struct invalid_model_test : public ::testing::TestWithParam<invalid_model_param>
{
};

TEST_P(invalid_model_test, Run)
{
  const char* model_file = (const char*)GetParam().model;
  size_t model_file_size = (size_t)GetParam().model_len;

  auto& undetectable_offsets = GetParam().undetectable_offsets;

  for (size_t end = 0; end < model_file_size - 1; ++end)
  {
    // we're not able to detect if complete index:weight pairs are missing
    if (undetectable_offsets.find(end) != undetectable_offsets.end()) { continue; }

    // type parameterized and value parameterized test cases can't be combined:
    // https://stackoverflow.com/questions/8507385/google-test-is-there-a-way-to-combine-a-test-which-is-both-type-parameterized-a
    if (GetParam().weight_type == predict_param_weight_type::SPARSE)
    {
      vw_predict<VW::sparse_parameters> vw_pred;
      EXPECT_NE(S_VW_PREDICT_OK, vw_pred.load(&model_file[0], end))
          << "partial model read until " << end << " didn't throw";
    }
    else
    {
      vw_predict<VW::dense_parameters> vw_pred;
      EXPECT_NE(S_VW_PREDICT_OK, vw_pred.load(&model_file[0], end))
          << "partial model read until " << end << " didn't throw";
    }
  }
}

invalid_model_param invalid_model_param[] = {
    {"regression_data_1", regression_data_1_model, regression_data_1_model_len, {54, 62},
        predict_param_weight_type::SPARSE},  // 2 weights
    {"regression_data_1", regression_data_1_model, regression_data_1_model_len, {54, 62},
        predict_param_weight_type::DENSE},  // 2 weights
    {"regression_data_6", regression_data_6_model, regression_data_6_model_len, {69, 81, 93, 105},
        predict_param_weight_type::SPARSE}  // 4 weights
};

INSTANTIATE_TEST_SUITE_P(VowpalWabbitSlim, invalid_model_test, ::testing::ValuesIn(invalid_model_param));

TEST(VowpalWabbitSlim, MulticlassData4)
{
  vw_predict<VW::sparse_parameters> vw;
  test_data td = get_test_data("multiclass_data_4");
  ASSERT_EQ(0, vw.load((const char*)td.model, td.model_len));

  std::vector<float> out_scores;

  VW::example_predict shared;
  // shared |a 0:1 5:12
  example_predict_builder bs(&shared, (char*)"a");
  bs.push_feature(0, 1.f);
  bs.push_feature(5, 12.f);

  VW::example_predict ex[3];
  // 1:1.0 |b 0:1
  example_predict_builder b0(&ex[0], (char*)"b");
  b0.push_feature(0, 1.f);
  // 2:0.3 |b 0:2
  example_predict_builder b1(&ex[1], (char*)"b");
  b1.push_feature(0, 2.f);
  // 3:0.1 |b 0:3
  example_predict_builder b2(&ex[2], (char*)"b");
  b2.push_feature(0, 3.f);

  ASSERT_EQ(S_VW_PREDICT_OK, vw.predict(shared, ex, 3, out_scores));

  // 2:0.0386223,1:0.46983,0:0.901038
  std::vector<float> preds_expected = {0.901038f, 0.46983f, 0.0386223f};

  // compare output
  std::sort(out_scores.begin(), out_scores.end());
  std::sort(preds_expected.begin(), preds_expected.end());
  EXPECT_THAT(out_scores, Pointwise(FloatNear(1e-4f), preds_expected));
}

TEST(VowpalWabbitSlim, MulticlassData5)
{
  vw_predict<VW::sparse_parameters> vw;
  test_data td = get_test_data("multiclass_data_5");
  ASSERT_EQ(0, vw.load((const char*)td.model, td.model_len));

  std::vector<float> out_scores;

  VW::example_predict shared;
  // shared |aa 0:1 5:12
  example_predict_builder bs(&shared, "aa");
  bs.push_feature(0, 1.f);
  bs.push_feature(5, 12.f);

  VW::example_predict ex[8];
  // 1:1.0 |ab 0:1
  example_predict_builder b0(&ex[0], "ab");
  b0.push_feature(0, 1.f);
  // 2:0.3 |ab 0:2
  example_predict_builder b1(&ex[1], "ab");
  b1.push_feature(0, 2.f);
  // 3:0.1 |ab 0:3
  example_predict_builder b2(&ex[2], "ab");
  b2.push_feature(0, 3.f);
  // 1:1.0 |ab 0:1
  example_predict_builder b3(&ex[3], "ab");
  b3.push_feature(0, 1.f);
  // 1:0.7 |ac 0:1
  example_predict_builder b4(&ex[4], "ac");
  b4.push_feature(0, 1.f);
  // 2:1.0 |ac 0:2
  example_predict_builder b5(&ex[5], "ac");
  b5.push_feature(0, 2.f);
  // 3:0.4 |ac 0:3
  example_predict_builder b6(&ex[6], "ac");
  b6.push_feature(0, 3.f);
  // 1:0.7 |ac 0:1
  example_predict_builder b7(&ex[7], "ac");
  b7.push_feature(0, 1.f);

  ASSERT_EQ(S_VW_PREDICT_OK, vw.predict(shared, ex, 8, out_scores));

  // 3:0.127492,3:0.438331,2:0.461522,2:0.668748,1:0.795552,1:0.795552,1:0.899165,1:0.899165
  std::vector<float> preds_expected = {0.127492, 0.438331, 0.461522, 0.668748, 0.795552, 0.795552, 0.899165, 0.899165};

  // compare output
  std::sort(out_scores.begin(), out_scores.end());
  std::sort(preds_expected.begin(), preds_expected.end());
  EXPECT_THAT(out_scores, Pointwise(FloatNear(1e-4f), preds_expected));
}

TEST(VowpalWabbitSlim, InteractionNumBitsBug)
{
  std::ifstream input(
      VW_SLIM_TEST_DIR "data/Delay_Margin_AudioNetworkPCR_all_cb_FF8.model", std::ios::in | std::ios::binary);
  input.seekg(0, std::ios::end);
  auto length = input.tellg();
  input.seekg(0, std::ios::beg);
  std::unique_ptr<char[]> buffer_ptr(new char[length]);
  input.read(buffer_ptr.get(),
      length);  // Extract how many bytes need to be decoded and resize the payload based on those bytes.

  vw_slim::vw_predict<VW::sparse_parameters> vw;

  int result = vw.load(buffer_ptr.get(), length);
  EXPECT_EQ(result, 0);

  // we have loaded the model and can push the features
  VW::example_predict features;

  // Test with the single namespace.
  vw_slim::example_predict_builder bOa(&features, "Features", vw.feature_index_num_bits());  // NOLINT
  bOa.push_feature_string("Networkmobile", 1.f);
  bOa.push_feature_string("CallTypeP2P", 1.f);
  bOa.push_feature_string("PlatformAndroid", 1.f);
  bOa.push_feature_string("MediaTypeVideo", 1.f);

  static constexpr const int MINDELAYACTIONS = 10;
  // push actions
  VW::example_predict actions[MINDELAYACTIONS];
  for (int i = 0; i < MINDELAYACTIONS; i++)
  {
    vw_slim::example_predict_builder bOe(&actions[i], "80");  // NOLINT
    bOe.push_feature(i, 1.f);
  }

  // generate UUID
  std::string uuid_string("EventId_0");

  std::vector<float> pdfs;
  std::vector<int> rankings;

  result = vw.predict(uuid_string.c_str(), features, actions, MINDELAYACTIONS, pdfs, rankings);
  EXPECT_EQ(result, 0);
  EXPECT_EQ(rankings[0], 3);
}

void clear(VW::example_predict& ex)
{
  for (auto ns : ex.indices) { ex.feature_space[ns].clear(); }
  ex.indices.clear();
}

void generate_cb_data_5(VW::example_predict& shared, VW::example_predict* ex)
{
  // clean features
  clear(shared);
  clear(ex[0]);
  clear(ex[1]);
  clear(ex[2]);

  // shared |a 0:1 5:12
  example_predict_builder bs(&shared, "a");
  bs.push_feature(0, 1.f);
  bs.push_feature(5, 12.f);

  //  |b 0:1
  example_predict_builder b0(&ex[0], "b");
  b0.push_feature(0, 1.f);
  //  |b 0:2
  example_predict_builder b1(&ex[1], "b");
  b1.push_feature(0, 2.f);
  //  |b 0:3
  example_predict_builder b2(&ex[2], "b");
  b2.push_feature(0, 3.f);
}

std::string generate_string_seed(size_t i)
{
  std::stringstream s;
  s << "abcde" << i;
  return s.str();
}

struct cb_predict_param
{
public:
  const char* description;
  const char* model_filename;

  int replications;

  std::vector<float> pdf_expected;
  std::vector<float> ranking_pdf_expected;
};

// nice rendering in unit tests
::std::ostream& operator<<(::std::ostream& os, const cb_predict_param& param)
{
  return os << param.description << " " << param.model_filename;
}

struct cb_predict_test : public ::testing::TestWithParam<cb_predict_param>
{
};

TEST_P(cb_predict_test, CBRunPredict)
{
  vw_predict<VW::sparse_parameters> vw;
  test_data td = get_test_data(GetParam().model_filename);
  ASSERT_EQ(S_VW_PREDICT_OK, vw.load((const char*)td.model, td.model_len));

  EXPECT_TRUE(vw.is_cb_explore_adf());

  std::vector<float> pdf_expected = GetParam().pdf_expected;
  std::vector<float> histogram(pdf_expected.size() * pdf_expected.size());

  VW::example_predict shared;
  VW::example_predict ex[3];

  std::vector<int> ranking;

  generate_cb_data_5(shared, ex);

  size_t rep = GetParam().replications;
  for (size_t i = 0; i < rep; i++)
  {
    std::vector<float> pdf;

    // invoke prediction
    ASSERT_EQ(S_VW_PREDICT_OK, vw.predict(generate_string_seed(i).c_str(), shared, ex, 3, pdf, ranking));

    ASSERT_EQ(pdf_expected.size(), ranking.size());
    for (size_t i = 0; i < ranking.size(); i++) { histogram[i * ranking.size() + ranking[i]]++; }
  }

  for (auto& d : histogram) { d /= rep; }

#ifdef VW_SLIM_TEST_DEBUG
  // std::fstream log(VW_SLIM_TEST_DEBUG, std::fstream::app);
  // for (size_t i = 0; i < 3; i++)
  //{
  //	log << "slot " << i << " ";
  //	for (size_t j = 0; j < 3; j++)
  //		log << histogram[i * 3 + j] << " ";
  //	log << std::endl;
  //}
#endif

  EXPECT_THAT(histogram, Pointwise(FloatNear(2e-2f), GetParam().ranking_pdf_expected));
}

cb_predict_param cb_predict_params[] = {
    {"CB Epsilon Greedy", "cb_data_5", 10000, {0.8f, 0.1f, 0.1f},
        {
            0.8f, 0.1f, 0.1f,  // slot 0
                               // most of the time we should see action 0
            0.1f, 0.9f, 0.0f,  // slot 1
                               // most of the time we should see action 1
                               // in 10% we should see the top action 0 swapped from top-slot to here
            0.1f, 0.0f, 0.9f,  // slot 2
                               // most of the time we should see action 2
                               // in 10% we should see the top action 0 swapped from top-slot to here
        }},
    {"CB Softmax", "cb_data_6", 1000, {0.664f, 0.245f, 0.090f},
        {
            0.664f, 0.245f, 0.090f,  // slot 0
            0.245f, 0.755f, 0.000f,  // slot 1
            0.090f, 0.000f, 0.910f,  // slot 2
        }},
    {"CB Bag", "cb_data_7", 5, {1.0f, 0.0f, 0.0f},
        {
            1.0f, 0.0f, 0.0f,  // slot 0
            0.0f, 1.0f, 0.0f,  // slot 1
            0.0f, 0.0f, 1.0f,  // slot 2
        }},
    {"CB Bag Epsilon Greedy", "cb_data_8", 10000, {0.82f, 0.09f, 0.09f},
        {
            0.82f, 0.09f, 0.09f,  // slot 0
            0.09f, 0.91f, 0.00f,  // slot 1
            0.09f, 0.00f, 0.91f,  // slot 2
        }},
};

INSTANTIATE_TEST_SUITE_P(VowpalWabbitSlim, cb_predict_test, ::testing::ValuesIn(cb_predict_params));

// Test fixture to allow for both sparse and dense parameters
template <typename W>
struct vw_slim_tests : public ::testing::Test
{
};

TYPED_TEST_SUITE_P(vw_slim_tests);

using WeightParameters = ::testing::Types<VW::sparse_parameters, VW::dense_parameters>;

TYPED_TEST_P(vw_slim_tests, model_not_loaded)
{
  vw_predict<TypeParam> vw;
  VW::example_predict ex;
  VW::example_predict* actions = nullptr;
  std::vector<float> scores;
  std::vector<int> ranking;
  float score;

  EXPECT_EQ(E_VW_PREDICT_ERR_NO_MODEL_LOADED, vw.predict(ex, score));

  EXPECT_EQ(E_VW_PREDICT_ERR_NO_MODEL_LOADED, vw.predict(ex, actions, 0, scores));
  EXPECT_EQ(E_VW_PREDICT_ERR_NO_MODEL_LOADED, vw.predict("abc", ex, actions, 0, scores, ranking));
}

TYPED_TEST_P(vw_slim_tests, model_reduction_mismatch)
{
  vw_predict<TypeParam> vw;
  VW::example_predict ex;
  VW::example_predict* actions = nullptr;
  std::vector<float> scores;
  std::vector<int> ranking;

  test_data td = get_test_data("regression_data_1");
  ASSERT_EQ(0, vw.load((const char*)td.model, td.model_len));

  EXPECT_EQ(E_VW_PREDICT_ERR_NO_A_CSOAA_MODEL, vw.predict(ex, actions, 0, scores));
  EXPECT_EQ(E_VW_PREDICT_ERR_NOT_A_CB_MODEL, vw.predict("abc", ex, actions, 0, scores, ranking));
}

TYPED_TEST_P(vw_slim_tests, model_corrupted)
{
  test_data td = get_test_data("regression_data_1");

  vw_predict<TypeParam> vw;

  size_t num_bytes_to_corrupt = 10;
  float rand = 0;
  for (size_t i = 0; i < 100; i++)
  {
    std::vector<char> model_copy(td.model, td.model + td.model_len);
    for (size_t j = 0; j < num_bytes_to_corrupt; j++)
    {
      rand = VW::details::merand48_noadvance((uint64_t)rand);
      size_t random_idx = (size_t)(rand * model_copy.size());

      rand = VW::details::merand48_noadvance((uint64_t)rand);

      model_copy[random_idx] = (char)rand;

      ASSERT_NE(0, vw.load((const char*)&model_copy[0], model_copy.size()));
    }
  }
}

REGISTER_TYPED_TEST_SUITE_P(vw_slim_tests, model_not_loaded, model_reduction_mismatch, model_corrupted);
INSTANTIATE_TYPED_TEST_SUITE_P(VowpalWabbitSlim, vw_slim_tests, WeightParameters, );

TEST(ColdStartModelSlim, ActionSetNotReordered)
{
  std::ifstream input(VW_SLIM_TEST_DIR "data/cold_start.model", std::ios::in | std::ios::binary);
  input.seekg(0, std::ios::end);
  auto length = input.tellg();
  input.seekg(0, std::ios::beg);
  std::unique_ptr<char[]> buffer_ptr(new char[length]);
  input.read(buffer_ptr.get(), length);

  vw_slim::vw_predict<VW::sparse_parameters> vw;

  int result = vw.load(buffer_ptr.get(), length);
  EXPECT_EQ(result, 0);

  VW::example_predict features;

  vw_slim::example_predict_builder bOa(&features, "Features", vw.feature_index_num_bits());  // NOLINT
  bOa.push_feature_string("f1", 1.f);

  static const int NUM_ACTIONS = 5;
  std::array<VW::example_predict, NUM_ACTIONS> actions;
  for (size_t i = 0; i < actions.size(); i++)
  {
    vw_slim::example_predict_builder bOe(&actions[i], "ActionFeatures");  // NOLINT
    bOe.push_feature(i, 1.f);
  }

  std::string uuid_string("EventId_0");

  std::vector<float> pdfs;
  std::vector<int> rankings;

  result = vw.predict(uuid_string.c_str(), features, actions.data(), NUM_ACTIONS, pdfs, rankings);

  EXPECT_GT(pdfs[0], 0.8);
  EXPECT_GT(pdfs[0], pdfs[1]);
  EXPECT_THAT(rankings, ElementsAre(0, 1, 2, 3, 4));
}