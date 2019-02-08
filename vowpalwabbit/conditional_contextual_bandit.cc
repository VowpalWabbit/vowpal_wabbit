#include "conditional_contextual_bandit.h"
#include "reductions.h"
#include "example.h"
#include "global_data.h"
#include "cache.h"

#include <numeric>
#include <algorithm>

using namespace LEARNER;
using namespace VW;
using namespace VW::config;

bool CCB::ec_is_example_header(example& ec)
{
  return ec.l.conditional_contextual_bandit.type == example_type::shared;
}

void CCB::print_update(vw& all, bool is_test, example& ec, multi_ex* ec_seq, bool action_scores)
{
  // TODO: Implement for CCB
  throw std::runtime_error("CCB:print_update not implemented");
}

template <bool is_learn>
void do_actual_learning(CCB::ccb& data, multi_learner& base, multi_ex& examples)
{
  std::cout << "CCB:do_actual_learning: " << (is_learn ? "train" : "test") << std::endl;
  if (is_learn)
    multiline_learn_or_predict<true>(base, examples, (uint64_t)0);
  else
    multiline_learn_or_predict<false>(base, examples, (uint64_t)0);
}

base_learner* CCB::ccb_explore_adf_setup(options_i& options, vw& all)
{
  free_ptr<ccb> data = scoped_calloc_or_throw<ccb>();
  bool ccb_explore_adf_option = false;
  option_group_definition new_options("Conditional Contextual Bandit Exploration with Action Dependent Features");
  new_options
    .add(make_option("ccb_explore_adf", ccb_explore_adf_option)
      .keep()
      .help("Do Conditional Contextual Bandit learning with multiline action dependent features."));
  options.add_and_parse(new_options);

  if (!ccb_explore_adf_option)
    return nullptr;

  if (!options.was_supplied("cb_explore_adf"))
  {
    options.insert("cb_explore_adf", "");
    options.add_and_parse(new_options);
  }

  multi_learner* base = as_multiline(setup_base(options, all));
  all.p->lp = CB::cb_label;
  all.label_type = label_type::cb;

  // Extract from lower level reductions.
  learner<ccb, multi_ex>& l =
      init_learner(data, base, do_actual_learning<true>, do_actual_learning<false>, 1, prediction_type::decision_probs);

  return make_base(l);
}

namespace CCB {
  size_t read_cached_label(shared_data*, void* v, io_buf& cache)
  {
    CCB::label* ld = static_cast<CCB::label*>(v);
    size_t read_count = 0;

    ld->type = static_cast<example_type>(read_object<uint8_t>(cache));
    read_count += sizeof(uint8_t);

    bool is_outcome_present = read_object<bool>(cache);
    read_count += sizeof(bool);

    if (is_outcome_present)
    {
      ld->outcome = new CCB::conditional_contexual_bandit_outcome();
      ld->outcome->probabilities = v_init<ACTION_SCORE::action_score>();

      ld->outcome->cost = read_object<float>(cache);
      read_count += sizeof(float);
      auto size_probs = read_object<uint32_t>(cache);
      read_count += sizeof(uint32_t);

      for(auto i = 0; i < size_probs; i++)
      {
        ld->outcome->probabilities.push_back(read_object<ACTION_SCORE::action_score>(cache));
        read_count += sizeof(ACTION_SCORE::action_score);
      }
    }

    auto size_includes = read_object<uint32_t>(cache);
    read_count += sizeof(uint32_t);

    for (auto i = 0; i < size_includes; i++)
    {
      ld->explicit_included_actions.push_back(read_object<uint32_t>(cache));
      read_count += sizeof(uint32_t);
    }

    return read_count;
  }

  float ccb_weight(void*)
  {
    return 1.;
  }

  void cache_label(void* v, io_buf& cache)
  {
    char* c;
    CCB::label* ld = static_cast<CCB::label*>(v);
    size_t size =
      sizeof(uint8_t) // type
      + sizeof(bool) // outcome exists?
      + (ld->outcome == nullptr ? 0 : sizeof(ld->outcome->cost) // cost
        + sizeof(uint32_t) // probabilities size
        + sizeof(ACTION_SCORE::action_score) * ld->outcome->probabilities.size()) //probabilities
        + sizeof(uint32_t)  // explicit_included_actions size
      + sizeof(uint32_t) * ld->explicit_included_actions.size();

    cache.buf_write(c, size);

    *(uint8_t*)c = static_cast<uint8_t>(ld->type);
    c += sizeof(uint8_t);

    *(bool*)c = ld->outcome != nullptr;
    c += sizeof(bool);

    if (ld->outcome != nullptr)
    {
      *(float*)c = ld->outcome->cost;
      c += sizeof(float);

      *(uint32_t*)c = convert(ld->outcome->probabilities.size());
      c += sizeof(uint32_t);

      for (const auto& score : ld->outcome->probabilities)
      {
        *(ACTION_SCORE::action_score*)c = score;
        c += sizeof(ACTION_SCORE::action_score);
      }
    }

    *(uint32_t*)c = convert(ld->explicit_included_actions.size());
    c += sizeof(uint32_t);

    for (const auto& included_action : ld->explicit_included_actions)
    {
      *(uint32_t*)c = included_action;
      c += sizeof(uint32_t);
    }
  }

  void default_label(void* v)
  {
    CCB::label* ld = static_cast<CCB::label*>(v);
    ld->outcome = nullptr;
    ld->explicit_included_actions = v_init<uint32_t>();
    ld->type = example_type::unset;
  }

  bool test_label(void* v)
  {
    CCB::label* ld = static_cast<CCB::label*>(v);
    return ld->outcome == nullptr;
  }

  void delete_label(void* v)
  {
    CCB::label* ld = static_cast<CCB::label*>(v);
    if (ld->outcome)
    {
      ld->outcome->probabilities.delete_v();
      delete ld->outcome;
      ld->outcome = nullptr;
    }
    ld->explicit_included_actions.delete_v();
  }

  void copy_label(void* dst, void* src)
  {
    CCB::label* ldDst = static_cast<CCB::label*>(dst);
    CCB::label* ldSrc = static_cast<CCB::label*>(src);

    if (ldSrc->outcome)
    {
      ldDst->outcome = new CCB::conditional_contexual_bandit_outcome();
      ldDst->outcome->probabilities = v_init<ACTION_SCORE::action_score>();

      ldDst->outcome->cost = ldSrc->outcome->cost;
      copy_array(ldDst->outcome->probabilities, ldSrc->outcome->probabilities);
    }

    copy_array(ldDst->explicit_included_actions, ldSrc->explicit_included_actions);
    ldDst->type = ldSrc->type;
  }

  ACTION_SCORE::action_score convert_to_score(const substring& action_id_str, const substring& probability_str)
  {
    auto action_id = static_cast<uint32_t>(int_of_substring(action_id_str));
    auto probability = float_of_substring(probability_str);
    if (nanpattern(probability))
      THROW("error NaN probability: " << probability_str);

    if (probability > 1.0)
    {
      std::cerr << "invalid probability > 1 specified for an outcome, resetting to 1.\n";
      probability = 1.0;
    }
    if (probability < 0.0)
    {
      std::cerr << "invalid probability < 0 specified for an outcome, resetting to 0.\n";
      probability = .0;
    }

    return { action_id , probability };
  }

  //<action>:<probability>:<cost>,<action>:<probability,<action>:<probability>,…
  CCB::conditional_contexual_bandit_outcome* parse_outcome(substring& outcome)
  {
    auto& ccb_outcome = *(new CCB::conditional_contexual_bandit_outcome());

    auto split_commas = v_init<substring>();
    tokenize(',', outcome, split_commas);

    auto split_colons = v_init<substring>();
    tokenize(':', split_commas[0], split_colons);

    if (split_colons.size() != 3)
      THROW("Malformed ccb label");

    ccb_outcome.probabilities = v_init<ACTION_SCORE::action_score>();
    ccb_outcome.probabilities.push_back(convert_to_score(split_colons[0], split_colons[1]));

    ccb_outcome.cost = float_of_substring(split_colons[2]);
    if (nanpattern(ccb_outcome.cost))
      THROW("error NaN cost: " << split_colons[2]);

    split_colons.clear();

    for (int i = 1; i < split_commas.size(); i++)
    {
      tokenize(':', split_commas[i], split_colons);
      if (split_colons.size() != 2)
        THROW("Must be action probability pairs");
      ccb_outcome.probabilities.push_back(convert_to_score(split_colons[0], split_colons[1]));
    }

    split_colons.delete_v();
    split_commas.delete_v();

    return &ccb_outcome;
  }

  void parse_explicit_inclusions(CCB::label* ld, v_array<substring>& split_inclusions)
  {
    for (const auto& inclusion : split_inclusions)
    {
      ld->explicit_included_actions.push_back(int_of_substring(inclusion));
    }
  }

  void parse_label(parser* p, shared_data*, void* v, v_array<substring>& words)
  {
    CCB::label* ld = static_cast<CCB::label*>(v);

    if(words.size() < 2) THROW("ccb labels may not be empty");
    if (!substring_equal(words[0], "ccb"))
    {
      THROW("ccb labels require the first word to be ccb");
    }

    auto type = words[1];
    if (substring_equal(type, "shared"))
    {
      if(words.size() > 2) THROW("shared labels may not have a cost");
      ld->type = CCB::example_type::shared;
    }
    else if (substring_equal(type, "action"))
    {
      if (words.size() > 2) THROW("action labels may not have a cost");
      ld->type = CCB::example_type::action;
    }
    else if (substring_equal(type, "decision"))
    {
      if (words.size() > 4) THROW("ccb decision label can only have a type cost and exclude list");
      ld->type = CCB::example_type::decision;

      // Skip the first two words "ccb <type>"
      for (auto i = 2; i < words.size(); i++)
      {
        auto is_outcome = std::find(words[i].begin, words[i].end, ':');
        if (is_outcome != words[i].end)
        {
          if (ld->outcome != nullptr)
          {
            THROW("There may be only 1 outcome associated with a decision.")
          }

          ld->outcome = parse_outcome(words[i]);
        }
        else
        {
          tokenize(',', words[i], p->parse_name);
          parse_explicit_inclusions(ld, p->parse_name);
        }
      }

      // If a full distribution has been given, check if it sums to 1, otherwise throw.
      if (ld->outcome && ld->outcome->probabilities.size() > 1)
      {
        float total_pred =
          std::accumulate(ld->outcome->probabilities.begin(),
            ld->outcome->probabilities.end(),
            0.f,
            [](float result_so_far, ACTION_SCORE::action_score action_pred)
            {
              return result_so_far + action_pred.score;
            });

        // TODO do a proper comparison here.
        if (total_pred > 1.1f || total_pred < 0.9f)
        {
          THROW("When providing all predicition probabilties they must add up to 1.f");
        }
      }
    }
    else
    {
      THROW("unknown label type: " << type);
    }
  }

  // Export the definition of this label parser.
  label_parser ccb_label_parser = {
    default_label,
    parse_label,
    cache_label,
    read_cached_label,
    delete_label,
    ccb_weight,
    copy_label,
    test_label,
    sizeof(CCB::label)
  };
}
