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

  void parse_label(parser* p, shared_data*, void* v, v_array<substring>& words)
  {
    CCB::label* ld = static_cast<CCB::label*>(v);

    /*for (size_t i = 0; i < words.size(); i++)
    {
      cb_class f;
      tokenize(':', words[i], p->parse_name);

      if (p->parse_name.size() < 1 || p->parse_name.size() > 3)
        THROW("malformed cost specification: " << p->parse_name);

      f.partial_prediction = 0.;
      f.action = (uint32_t)hashstring(p->parse_name[0], 0);
      f.cost = FLT_MAX;

      if (p->parse_name.size() > 1)
        f.cost = float_of_substring(p->parse_name[1]);

      if (nanpattern(f.cost))
        THROW("error NaN cost (" << p->parse_name[1] << " for action: " << p->parse_name[0]);

      f.probability = .0;
      if (p->parse_name.size() > 2)
        f.probability = float_of_substring(p->parse_name[2]);

      if (nanpattern(f.probability))
        THROW("error NaN probability (" << p->parse_name[2] << " for action: " << p->parse_name[0]);

      if (f.probability > 1.0)
      {
        cerr << "invalid probability > 1 specified for an action, resetting to 1." << endl;
        f.probability = 1.0;
      }
      if (f.probability < 0.0)
      {
        cerr << "invalid probability < 0 specified for an action, resetting to 0." << endl;
        f.probability = .0;
      }
      if (substring_eq(p->parse_name[0], "shared"))
      {
        if (p->parse_name.size() == 1)
        {
          f.probability = -1.f;
        }
        else
          cerr << "shared feature vectors should not have costs" << endl;
      }

      ld->costs.push_back(f);
    }*/
  }


  label_parser ccb_label_parser = { default_label, parse_label,
                           cache_label, read_cached_label,
                           delete_label, ccb_weight,
                           copy_label,
                           test_label,
                           sizeof(CCB::label)
  };

}
