/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>

#include "example.h"
#include "parse_primitives.h"
#include "vw.h"
#include "vw_exception.h"
#include "cb_continuous.h"

using namespace LEARNER;
using namespace std;

namespace VW { namespace cb_continuous
{
  char* bufread_label(continuous_label* ld, char* c, io_buf& cache)
  {
    size_t num = *(size_t*)c;
    ld->costs.clear();
    c += sizeof(size_t);
    size_t total = sizeof(continuous_label_elm) * num;
    if (cache.buf_read(c, total) < total)
    {
      cout << "error in demarshal of cost data" << endl;
      return c;
    }
    for (size_t i = 0; i < num; i++)
    {
      continuous_label_elm temp = *(continuous_label_elm*)c;
      c += sizeof(continuous_label_elm);
      ld->costs.push_back(temp);
    }

    return c;
  }

  size_t read_cached_label(shared_data*, void* v, io_buf& cache)
  {
    continuous_label* ld = (continuous_label*)v;
    ld->costs.clear();
    char* c;
    size_t total = sizeof(size_t);
    if (cache.buf_read(c, total) < total)
      return 0;
    bufread_label(ld, c, cache);

    return total;
  }

  float weight(void*) { return 1.; }

  char* bufcache_label(continuous_label* ld, char* c)
  {
    *(size_t*)c = ld->costs.size();
    c += sizeof(size_t);
    for (size_t i = 0; i < ld->costs.size(); i++)
    {
      *(continuous_label_elm*)c = ld->costs[i];
      c += sizeof(continuous_label_elm);
    }
    return c;
  }

  void cache_label(void* v, io_buf& cache)
  {
    char* c;
    continuous_label* ld = (continuous_label*)v;
    cache.buf_write(c, sizeof(size_t) + sizeof(continuous_label_elm) * ld->costs.size());
    bufcache_label(ld, c);
  }

  void default_label(void* v)
  {
    continuous_label* ld = (continuous_label*)v;
    ld->costs.clear();
  }

  bool test_label(void* v)
  {
    continuous_label* ld = (continuous_label*)v;
    if (ld->costs.size() == 0)
      return true;
    for (size_t i = 0; i < ld->costs.size(); i++)
      if (FLT_MAX != ld->costs[i].cost && ld->costs[i].probability > 0.)
        return false;
    return true;
  }

  void delete_label(void* v)
  {
    continuous_label* ld = (continuous_label*)v;
    ld->costs.delete_v();
  }

  void copy_label(void* dst, void* src)
  {
    continuous_label* ldD = (continuous_label*)dst;
    continuous_label* ldS = (continuous_label*)src;
    copy_array(ldD->costs, ldS->costs);
  }

  void parse_label(parser* p, shared_data*, void* v, v_array<substring>& words)
  {
    continuous_label* ld = (continuous_label*)v;
    ld->costs.clear();
    for (size_t i = 0; i < words.size(); i++)
    {
      continuous_label_elm f;
      tokenize(':', words[i], p->parse_name);

      if (p->parse_name.size() < 1 || p->parse_name.size() > 3)
        THROW("malformed cost specification: " << p->parse_name);

      f.partial_prediction = 0.;
      f.action = (uint32_t)hashstring(p->parse_name[0], 0); //todo
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
      if (substring_equal(p->parse_name[0], "shared"))
      {
        if (p->parse_name.size() == 1)
        {
          f.probability = -1.f;
        }
        else
          cerr << "shared feature vectors should not have costs" << endl;
      }

      ld->costs.push_back(f);
    }
  }

  label_parser cb_cont_label = {default_label, parse_label, cache_label, read_cached_label, delete_label, weight, copy_label,
      test_label, sizeof(continuous_label)};

  bool ec_is_example_header(example& ec)  // example headers just have "shared"
  {
    v_array<continuous_label_elm> costs = ec.l.cb_cont.costs;
    if (costs.size() != 1)
      return false;
    if (costs[0].probability == -1.f)
      return true;
    return false;
  }

  void print_update(vw& all, bool is_test, example& ec, multi_ex* ec_seq, bool action_scores)
  {
    if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
    {
      size_t num_features = ec.num_features;

      size_t pred = ec.pred.multiclass; // TODO: what is pred here? the same?
      if (ec_seq != nullptr)
      {
        num_features = 0;
        // TODO: code duplication csoaa.cc LabelDict::ec_is_example_header
        for (size_t i = 0; i < (*ec_seq).size(); i++)
          if (!ec_is_example_header(*(*ec_seq)[i]))
            num_features += (*ec_seq)[i]->num_features;
      }
      std::string label_buf;
      if (is_test)
        label_buf = " unknown";
      else
        label_buf = " known";

      if (action_scores)
      {
        std::ostringstream pred_buf;
        pred_buf << std::setw(all.sd->col_current_predict) << std::right << std::setfill(' ');
        if (ec.pred.prob_dist.size() > 0)
          pred_buf <<  ec.pred.prob_dist[0].left << "," << ec.pred.prob_dist[0].right << ":" << ec.pred.prob_dist[0].pdf_value << "..."; //TODO: changed a_s to p_d, correct?!
        else
          pred_buf << "no action";
        all.sd->print_update(all.holdout_set_off, all.current_pass, label_buf, pred_buf.str(), num_features,
            all.progress_add, all.progress_arg);
      }
      else
        all.sd->print_update(all.holdout_set_off, all.current_pass, label_buf, (uint32_t)pred, num_features,
            all.progress_add, all.progress_arg);
    }
  }

  std::string to_string(const continuous_label_elm& elm)
  {
    std::stringstream strm;
    strm << "{" << elm.action << "," << elm.cost << "," << elm.probability << "}";
    return strm.str();
  }
}}  // namespace VW::cb_continuous
