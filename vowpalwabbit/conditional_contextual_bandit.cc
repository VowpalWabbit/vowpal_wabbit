#include "conditional_contextual_bandit.h"

#include "example.h"
#include "global_data.h"

bool CCB::ec_is_example_header(example& ec)
{
  return ec.l.conditional_contextual_bandit.type == example_type::shared;
}

void CCB::print_update(vw& all, bool is_test, example& ec, multi_ex* ec_seq, bool action_scores)
{
  // TODO: Implement for CCB
  throw std::runtime_error("CCB:print_update not implemented");
}

//char* bufread_label(CB::label* ld, char* c, io_buf& cache)
//{
//  size_t num = *(size_t*)c;
//  ld->costs.clear();
//  c += sizeof(size_t);
//  size_t total = sizeof(cb_class) * num;
//  if (cache.buf_read(c, total) < total)
//  {
//    cout << "error in demarshal of cost data" << endl;
//    return c;
//  }
//  for (size_t i = 0; i < num; i++)
//  {
//    cb_class temp = *(cb_class*)c;
//    c += sizeof(cb_class);
//    ld->costs.push_back(temp);
//  }
//
//  return c;
//}

namespace CCB {
  size_t read_cached_label(shared_data*, void* v, io_buf& cache)
  {
    /*CCB::label* ld = static_cast<CCB::label*>(v);
    ld->costs.clear();
    char* c;
    size_t total = sizeof(size_t);
    if (cache.buf_read(c, total) < total)
      return 0;
    bufread_label(ld, c, cache);

    return total;*/
    return 0;
  }

  float ccb_weight(void*)
  {
    return 1.;
  }

  //char* bufcache_label(CB::label * ld, char* c)
  //{
  //  *(size_t*)c = ld->costs.size();
  //  c += sizeof(size_t);
  //  for (size_t i = 0; i < ld->costs.size(); i++)
  //  {
  //    *(cb_class*)c = ld->costs[i];
  //    c += sizeof(cb_class);
  //  }
  //  return c;
  //}

  void cache_label(void* v, io_buf& cache)
  {
    /*char* c;
    CCB::label* ld = static_cast<CCB::label*>(v);
    cache.buf_write(c, sizeof(size_t) + sizeof(cb_class) * ld->costs.size());
    bufcache_label(ld, c);*/
  }

  void default_label(void* v)
  {
    CCB::label* ld = static_cast<CCB::label*>(v);
    ld->outcomes.clear();
    ld->excluded_actions.clear();
  }

  bool test_label(void* v)
  {
    CCB::label* ld = static_cast<CCB::label*>(v);
    return ld->outcomes.size() == 0;
  }

  void delete_label(void* v)
  {
    CCB::label* ld = static_cast<CCB::label*>(v);
    ld->outcomes.delete_v();
  }

  void copy_label(void* dst, void* src)
  {
    CCB::label* ldD = static_cast<CCB::label*>(dst);
    CCB::label* ldS = static_cast<CCB::label*>(src);

    copy_array(ldD->outcomes, ldS->outcomes);
    ldD->type = ldS->type;
  }

  CCB::conditional_contexual_bandit_outcome parse_outcome(v_array<substring>& split_outcome)
  {
    if (split_outcome.size() != 3)
      THROW("malformed cost specification: " << split_outcome);

    CCB::conditional_contexual_bandit_outcome ccb_outcome;

    ccb_outcome.action_id = int_of_substring(split_outcome[0]);
    ccb_outcome.cost = float_of_substring(split_outcome[1]);
    ccb_outcome.probability = float_of_substring(split_outcome[2]);

    if (nanpattern(ccb_outcome.cost))
      THROW("error NaN cost: " << split_outcome[1]);

    if (nanpattern(ccb_outcome.probability))
      THROW("error NaN probabilit: " << split_outcome[1]);

    if (ccb_outcome.probability > 1.0)
    {
      std::cerr << "invalid probability > 1 specified for an outcome, resetting to 1.\n";
      ccb_outcome.probability = 1.0;
    }
    if (ccb_outcome.probability < 0.0)
    {
      std::cerr << "invalid probability < 0 specified for an outcome, resetting to 0.\n";
      ccb_outcome.probability = .0;
    }

    return ccb_outcome;
  }

  void parse_exclusions(CCB::label* ld, v_array<substring>& split_exclusions)
  {
    for (const auto& exclusion : split_exclusions)
    {
      ld->excluded_actions.push_back(int_of_substring(exclusion));
    }
  }

  void parse_label(parser* p, shared_data*, void* v, v_array<substring>& words)
  {
    CCB::label* ld = static_cast<CCB::label*>(v);

    if(words.size() < 1) THROW("ccb labels may not be empty");

    if (substring_equal(words[0], "shared"))
    {
      if(words.size() > 1) THROW("shared labels may not have a cost");
      ld->type = CCB::example_type::shared;
    }
    else if (substring_equal(words[0], "action"))
    {
      if (words.size() > 1) THROW("action labels may not have a cost");
      ld->type = CCB::example_type::action;
    }
    else if (substring_equal(words[0], "decision"))
    {
      if (words.size() > 3) THROW("decision label can only have a type cost and exclude list");
      ld->type = CCB::example_type::decision;

      // Skip the first word as that is the type.
      for (auto i = 1; i < words.size(); i++)
      {
        tokenize(':', words[i], p->parse_name);
        if (p->parse_name.size() > 1)
        {
          ld->outcomes.push_back(parse_outcome(p->parse_name));
        }
        else
        {
          tokenize(',', words[i], p->parse_name);
          parse_exclusions(ld, p->parse_name);
        }
      }
    }
    else
    {
      THROW("unknown label type: " << words[0]);
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
