#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

// avoid mmap dependency
#define DISABLE_SHARED_WEIGHTS

#include "model_parser.h"
#include "opts.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/example_predict.h"
#include "vw/core/gd_predict.h"
#include "vw/core/interactions.h"
#include "vw/explore/explore.h"

namespace vw_slim
{
/**
 * @brief Exploration algorithm specified by the model.
 */
enum class vw_predict_exploration
{
  epsilon_greedy,
  softmax,
  bag
};

uint64_t ceil_log_2(uint64_t v);

// this guard assumes that namespaces are added in order
// the complete feature_space of the added namespace is cleared afterwards
class namespace_copy_guard
{
public:
  namespace_copy_guard(VW::example_predict& ex, unsigned char ns);
  ~namespace_copy_guard();

  void feature_push_back(VW::feature_value v, VW::feature_index idx);

private:
  VW::example_predict& _ex;
  unsigned char _ns;
  bool _remove_ns;
};

class feature_offset_guard
{
public:
  feature_offset_guard(VW::example_predict& ex, uint64_t ft_offset);
  ~feature_offset_guard();

private:
  VW::example_predict& _ex;
  uint64_t _old_ft_offset;
};

class stride_shift_guard
{
public:
  stride_shift_guard(VW::example_predict& ex, uint64_t shift);
  ~stride_shift_guard();

private:
  VW::example_predict& _ex;
  uint64_t _shift;
};

/**
 * @brief Vowpal Wabbit slim predictor. Supports: regression, multi-class classification and contextual bandits.
 */
template <typename W>
class vw_predict
{
public:
  vw_predict() : _contains_wildcard(false), _model_loaded(false) {}

  /**
   * @brief Reads the Vowpal Wabbit model from the supplied buffer (produced using vw -f <modelname>)
   *
   * @param model The binary model.
   * @param length The length of the binary model.
   * @return int Returns 0 (S_VW_PREDICT_OK) if succesful, otherwise one of the error codes (see E_VW_PREDICT_ERR_*).
   */
  int load(const char* model, size_t length)
  {
    if (!model || length == 0) { return E_VW_PREDICT_ERR_INVALID_MODEL; }

    _model_loaded = false;

    // required for inline_predict
    _ignore_linear.fill(false);

    model_parser mp(model, length);

    // parser_regressor.cc: save_load_header
    RETURN_ON_FAIL(mp.read_string<false>("version", _version));

    // read model id
    RETURN_ON_FAIL(mp.read_string<true>("model_id", _id));

    RETURN_ON_FAIL(mp.skip(sizeof(char)));   // "model character"
    RETURN_ON_FAIL(mp.skip(sizeof(float)));  // "min_label"
    RETURN_ON_FAIL(mp.skip(sizeof(float)));  // "max_label"

    RETURN_ON_FAIL(mp.read("num_bits", _num_bits));
    // Cannot use more than 32 bits on a 32 bit architecture
    if (sizeof(size_t) == 4 && _num_bits > 32) { return E_VW_PREDICT_ERR_INVALID_MODEL; }

    RETURN_ON_FAIL(mp.skip(sizeof(uint32_t)));  // "lda"

    uint32_t ngram_len;
    RETURN_ON_FAIL(mp.read("ngram_len", ngram_len));
    mp.skip(3 * ngram_len);

    uint32_t skips_len;
    RETURN_ON_FAIL(mp.read("skips_len", skips_len));
    mp.skip(3 * skips_len);

    RETURN_ON_FAIL(mp.read_string<true>("file_options", _command_line_arguments));

    // command line arg parsing
    _no_constant = _command_line_arguments.find("--noconstant") != std::string::npos;

    // only 0-valued hash_seed supported
    int hash_seed;
    if (find_opt_int(_command_line_arguments, "--hash_seed", hash_seed) && hash_seed)
    {
      return E_VW_PREDICT_ERR_HASH_SEED_NOT_SUPPORTED;
    }

    _interactions.clear();
    find_opt(_command_line_arguments, "-q", _interactions);
    find_opt(_command_line_arguments, "--quadratic", _interactions);
    find_opt(_command_line_arguments, "--cubic", _interactions);
    find_opt(_command_line_arguments, "--interactions", _interactions);

    // VW performs the following transformation as a side-effect of looking for duplicates.
    // This affects how interaction hashes are generated.
    std::vector<std::vector<VW::namespace_index>> vec_sorted;
    for (auto& interaction : _interactions) { std::sort(std::begin(interaction), std::end(interaction)); }

    for (const auto& inter : _interactions)
    {
      if (VW::contains_wildcard(inter))
      {
        _contains_wildcard = true;
        break;
      }
    }

    // TODO: take --cb_type dr into account
    uint64_t num_weights = 0;

    if (_command_line_arguments.find("--cb_explore_adf") != std::string::npos)
    {
      // parse exploration options
      int bag_size;
      if (find_opt_int(_command_line_arguments, "--bag", bag_size))
      {
        if (bag_size < 0) { return E_VW_PREDICT_ERR_INVALID_MODEL; }
        _bag_size = static_cast<size_t>(bag_size);

        _exploration = vw_predict_exploration::bag;
        num_weights = _bag_size;

        // check for additional minimum epsilon greedy
        _minimum_epsilon = 0.f;
        find_opt_float(_command_line_arguments, "--epsilon", _minimum_epsilon);
      }
      else if (_command_line_arguments.find("--softmax") != std::string::npos)
      {
        if (find_opt_float(_command_line_arguments, "--lambda", _lambda))
        {
          if (_lambda > 0)
          {  // Lambda should always be negative because we are using a cost basis.
            _lambda = -_lambda;
          }
          _exploration = vw_predict_exploration::softmax;
        }
      }
      else if (find_opt_float(_command_line_arguments, "--epsilon", _epsilon))
      {
        _exploration = vw_predict_exploration::epsilon_greedy;
      }
      else { return E_VW_PREDICT_ERR_CB_EXPLORATION_MISSING; }
    }

    // VW style check_sum validation
    uint32_t check_sum_computed = mp.checksum();

    // perform check sum check
    uint32_t check_sum_len;
    RETURN_ON_FAIL((mp.read<uint32_t, false>("check_sum_len", check_sum_len)));
    if (check_sum_len != sizeof(uint32_t)) { return E_VW_PREDICT_ERR_INVALID_MODEL; }

    uint32_t check_sum;
    RETURN_ON_FAIL((mp.read<uint32_t, false>("check_sum", check_sum)));

    if (check_sum_computed != check_sum) { return E_VW_PREDICT_ERR_INVALID_MODEL_CHECK_SUM; }

    if (_command_line_arguments.find("--cb_adf") != std::string::npos)
    {
      RETURN_ON_FAIL(mp.skip(sizeof(uint64_t)));  // cb_adf.cc: event_sum
      RETURN_ON_FAIL(mp.skip(sizeof(uint64_t)));  // cb_adf.cc: action_sum
    }

    // gd.cc: save_load
    bool gd_resume;
    RETURN_ON_FAIL(mp.read("resume", gd_resume));
    if (gd_resume) { return E_VW_PREDICT_ERR_GD_RESUME_NOT_SUPPORTED; }

    // read sparse weights into dense
    _stride_shift = (uint32_t)ceil_log_2(num_weights);

    RETURN_ON_FAIL(mp.read_weights<W>(_weights, _num_bits, _stride_shift));

    // TODO: check that permutations is not enabled (or parse it)

    _model_loaded = true;

    return S_VW_PREDICT_OK;
  }

  /**
   * @brief True if the model describes a contextual bandit (cb) model using action dependent features (afd)
   *
   * @return true True if contextual bandit predict method can be used.
   * @return false False if contextual bandit predict method cannot be used.
   */
  bool is_cb_explore_adf() { return _command_line_arguments.find("--cb_explore_adf") != std::string::npos; }

  /**
   * @brief True if the model describes a cost sensitive one-against-all (csoaa). This is also true for cb_explore_adf
   * models, as they are reduced to csoaa.
   *
   * @return true True if csoaa predict method can be used.
   * @return false False if csoaa predict method cannot be used.
   */
  bool is_csoaa_ldf() { return _command_line_arguments.find("--csoaa_ldf") != std::string::npos; }

  /**
   * @brief Predicts a score (as in regression) for the provided example.
   *
   * Regular regression with support for constant feature (bias term) and interactions
   *
   * @param ex The example to get the prediction for.
   * @param score The output score produced by the model.
   * @return int Returns 0 (S_VW_PREDICT_OK) if succesful, otherwise one of the error codes (see E_VW_PREDICT_ERR_*).
   */
  int predict(VW::example_predict& ex, float& score)
  {
    if (!_model_loaded) { return E_VW_PREDICT_ERR_NO_MODEL_LOADED; }

    std::unique_ptr<namespace_copy_guard> ns_copy_guard;

    if (!_no_constant)
    {
      // add constant feature
      ns_copy_guard =
          std::unique_ptr<namespace_copy_guard>(new namespace_copy_guard(ex, VW::details::CONSTANT_NAMESPACE));
      ns_copy_guard->feature_push_back(1.f, (VW::details::CONSTANT << _stride_shift) + ex.ft_offset);
    }

    if (_contains_wildcard)
    {
      // permutations is not supported by slim so we can just use combinations!
      _generate_interactions.update_interactions_if_new_namespace_seen<
          VW::details::generate_namespace_combinations_with_repetition, false>(_interactions, ex.indices);
      score = VW::inline_predict<W>(*_weights, false, _ignore_linear, _generate_interactions.generated_interactions,
          _unused_extent_interactions,
          /* permutations */ false, ex, _generate_interactions_object_cache);
    }
    else
    {
      score = VW::inline_predict<W>(*_weights, false, _ignore_linear, _interactions, _unused_extent_interactions,
          /* permutations */ false, ex, _generate_interactions_object_cache);
    }
    return S_VW_PREDICT_OK;
  }

  // multiclass classification
  int predict(
      VW::example_predict& shared, VW::example_predict* actions, size_t num_actions, std::vector<float>& out_scores)
  {
    if (!_model_loaded) { return E_VW_PREDICT_ERR_NO_MODEL_LOADED; }

    if (!is_csoaa_ldf()) { return E_VW_PREDICT_ERR_NO_A_CSOAA_MODEL; }

    out_scores.resize(num_actions);

    VW::example_predict* action = actions;
    for (size_t i = 0; i < num_actions; i++, action++)
    {
      std::vector<std::unique_ptr<namespace_copy_guard>> ns_copy_guards;

      // shared feature copying
      for (auto ns : shared.indices)
      {
        // insert namespace
        auto ns_copy_guard = std::unique_ptr<namespace_copy_guard>(new namespace_copy_guard(*action, ns));

        // copy features
        for (auto fs : shared.feature_space[ns]) { ns_copy_guard->feature_push_back(fs.value(), fs.index()); }

        // keep guard around
        ns_copy_guards.push_back(std::move(ns_copy_guard));
      }

      RETURN_ON_FAIL(predict(*action, out_scores[i]));
    }

    return S_VW_PREDICT_OK;
  }

  int predict(const char* event_id, VW::example_predict& shared, VW::example_predict* actions, size_t num_actions,
      std::vector<float>& pdf, std::vector<int>& ranking)
  {
    if (!_model_loaded) { return E_VW_PREDICT_ERR_NO_MODEL_LOADED; }

    if (!is_cb_explore_adf()) { return E_VW_PREDICT_ERR_NOT_A_CB_MODEL; }

    std::vector<float> scores;

    // add exploration
    pdf.resize(num_actions);
    ranking.resize(num_actions);

    switch (_exploration)
    {
      case vw_predict_exploration::epsilon_greedy:
      {
        // get the prediction
        RETURN_ON_FAIL(predict(shared, actions, num_actions, scores));

        // generate exploration distribution
        // model is trained against cost -> minimum is better
        auto top_action_iterator = std::min_element(std::begin(scores), std::end(scores));
        uint32_t top_action = (uint32_t)(top_action_iterator - std::begin(scores));

        RETURN_EXPLORATION_ON_FAIL(
            VW::explore::generate_epsilon_greedy(_epsilon, top_action, std::begin(pdf), std::end(pdf)));
        break;
      }
      case vw_predict_exploration::softmax:
      {
        // get the prediction
        RETURN_ON_FAIL(predict(shared, actions, num_actions, scores));

        // generate exploration distribution
        RETURN_EXPLORATION_ON_FAIL(VW::explore::generate_softmax(
            _lambda, std::begin(scores), std::end(scores), std::begin(pdf), std::end(pdf)));
        break;
      }
      case vw_predict_exploration::bag:
      {
        std::vector<uint32_t> top_actions(num_actions);

        // apply stride shifts
        std::vector<std::unique_ptr<stride_shift_guard>> stride_shift_guards;
        stride_shift_guards.push_back(
            std::unique_ptr<stride_shift_guard>(new stride_shift_guard(shared, _stride_shift)));
        VW::example_predict* actions_end = actions + num_actions;
        for (VW::example_predict* action = actions; action != actions_end; ++action)
        {
          stride_shift_guards.push_back(
              std::unique_ptr<stride_shift_guard>(new stride_shift_guard(*action, _stride_shift)));
        }

        for (size_t i = 0; i < _bag_size; i++)
        {
          std::vector<std::unique_ptr<feature_offset_guard>> feature_offset_guards;
          for (VW::example_predict* action = actions; action != actions_end; ++action)
          {
            feature_offset_guards.push_back(
                std::unique_ptr<feature_offset_guard>(new feature_offset_guard(*action, i)));
          }

          RETURN_ON_FAIL(predict(shared, actions, num_actions, scores));

          auto top_action_iterator = std::min_element(std::begin(scores), std::end(scores));
          uint32_t top_action = (uint32_t)(top_action_iterator - std::begin(scores));

          top_actions[top_action]++;
        }

        // generate exploration distribution
        RETURN_EXPLORATION_ON_FAIL(
            VW::explore::generate_bag(std::begin(top_actions), std::end(top_actions), std::begin(pdf), std::end(pdf)));

        if (_minimum_epsilon > 0)
          RETURN_EXPLORATION_ON_FAIL(
              VW::explore::enforce_minimum_probability(_minimum_epsilon, true, std::begin(pdf), std::end(pdf)));

        break;
      }
      default:
        return E_VW_PREDICT_ERR_NOT_A_CB_MODEL;
    }

    RETURN_EXPLORATION_ON_FAIL(sort_by_scores(
        std::begin(pdf), std::end(pdf), std::begin(scores), std::end(scores), std::begin(ranking), std::end(ranking)));

    // Sample from the pdf
    uint32_t chosen_action_idx;
    RETURN_EXPLORATION_ON_FAIL(
        VW::explore::sample_after_normalizing(event_id, std::begin(pdf), std::end(pdf), chosen_action_idx));

    // Swap top element with chosen one (unless chosen is the top)
    if (chosen_action_idx != 0)
    {
      std::iter_swap(std::begin(ranking), std::begin(ranking) + chosen_action_idx);
      std::iter_swap(std::begin(pdf), std::begin(pdf) + chosen_action_idx);
    }

    return S_VW_PREDICT_OK;
  }

  template <typename PdfIt, typename InputScoreIt, typename OutputIt>
  static int sort_by_scores(PdfIt pdf_first, PdfIt pdf_last, InputScoreIt scores_first, InputScoreIt scores_last,
      OutputIt ranking_begin, OutputIt ranking_last)
  {
    _UNUSED(scores_last);
    const size_t pdf_size = pdf_last - pdf_first;
    const size_t ranking_size = ranking_last - ranking_begin;

    if (pdf_size != ranking_size) { return E_EXPLORATION_PMF_RANKING_SIZE_MISMATCH; }

    // Initialize ranking with actions 0,1,2,3 ...
    std::iota(ranking_begin, ranking_last, 0);

    assert(std::distance(pdf_first, pdf_last) == std::distance(scores_first, scores_last));
    assert(std::distance(pdf_first, pdf_last) == std::distance(ranking_begin, ranking_last));

    assert(pdf_first <= pdf_last);
    const size_t size = std::distance(pdf_first, pdf_last);
    using zipped_tuple_t =
        std::tuple<typename PdfIt::value_type, typename InputScoreIt::value_type, typename OutputIt::value_type>;
    std::vector<zipped_tuple_t> zipped_values;
    zipped_values.reserve(size);
    auto pdf_it = pdf_first;
    auto scores_it = scores_first;
    auto ranking_it = ranking_begin;
    for (size_t i = 0; i < size; i++)
    {
      zipped_values.emplace_back(*pdf_it, *scores_it, *ranking_it);
      ++pdf_it;
      ++scores_it;
      ++ranking_it;
    }

    std::sort(zipped_values.begin(), zipped_values.end(),
        [](const zipped_tuple_t& l, const zipped_tuple_t& r) { return std::get<1>(l) < std::get<1>(r); });

    for (const auto& zipped_value : zipped_values)
    {
      *pdf_first = std::get<0>(zipped_value);
      *ranking_begin = std::get<2>(zipped_value);
      pdf_first++;
      ranking_begin++;
    }

    return S_EXPLORATION_OK;
  }

  uint32_t feature_index_num_bits() { return _num_bits; }

private:
  std::unique_ptr<W> _weights;
  std::string _id;
  std::string _version;
  std::string _command_line_arguments;
  std::vector<std::vector<VW::namespace_index>> _interactions;
  std::vector<std::vector<VW::extent_term>> _unused_extent_interactions;
  VW::details::generate_interactions_object_cache _generate_interactions_object_cache;
  VW::interactions_generator _generate_interactions;
  bool _contains_wildcard;
  std::array<bool, VW::NUM_NAMESPACES> _ignore_linear;
  bool _no_constant;

  vw_predict_exploration _exploration;
  float _minimum_epsilon;
  float _epsilon;
  float _lambda;
  size_t _bag_size;
  uint32_t _num_bits;

  uint32_t _stride_shift;
  bool _model_loaded;
};
}  // namespace vw_slim
