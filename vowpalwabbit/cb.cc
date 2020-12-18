// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cfloat>
#include <algorithm>

#include "example.h"
#include "parse_primitives.h"
#include "vw.h"
#include "vw_exception.h"
#include "example.h"
#include "cb_label_parser.h"
#include "vw_string_view.h"

using namespace VW::LEARNER;

namespace CB
{
char* bufread_label(CB::label& ld, char* c, io_buf& cache)
{
  size_t num = *(size_t*)c;
  ld.costs.clear();
  c += sizeof(size_t);
  size_t total = sizeof(cb_class) * num + sizeof(ld.weight);
  if (cache.buf_read(c, total) < total)
  {
    std::cout << "error in demarshal of cost data" << std::endl;
    return c;
  }
  for (size_t i = 0; i < num; i++)
  {
    cb_class temp = *(cb_class*)c;
    c += sizeof(cb_class);
    ld.costs.push_back(temp);
  }
  memcpy(&ld.weight, c, sizeof(ld.weight));
  c += sizeof(ld.weight);
  return c;
}

size_t read_cached_label(shared_data*, CB::label& ld, io_buf& cache)
{
  ld.costs.clear();
  char* c;
  size_t total = sizeof(size_t);
  if (cache.buf_read(c, total) < total)
    return 0;
  bufread_label(ld, c, cache);

  return total;
}

float weight(CB::label& ld)
{
  return ld.weight;
}

char* bufcache_label(CB::label& ld, char* c)
{
  *(size_t*)c = ld.costs.size();
  c += sizeof(size_t);
  for (auto const& cost : ld.costs)
  {
    *(cb_class*)c = cost;
    c += sizeof(cb_class);
  }
  memcpy(c, &ld.weight, sizeof(ld.weight));
  c += sizeof(ld.weight);
  return c;
}

void cache_label(CB::label& ld, io_buf& cache)
{
  char* c;
  cache.buf_write(c, sizeof(size_t) + sizeof(cb_class) * ld.costs.size() + sizeof(ld.weight));
  bufcache_label(ld, c);
}

void default_label(CB::label& ld)
{
  ld.costs.clear();
  ld.weight = 1;
}

bool test_label(CB::label& ld)
{
  if (ld.costs.empty())
    return true;
  for (auto const& cost : ld.costs)
    if (FLT_MAX != cost.cost && cost.probability > 0.)
      return false;
  return true;
}

void delete_label(CB::label& ld)
{
  ld.costs.delete_v();
}

void copy_label(CB::label& dst, CB::label& src)
{
  copy_array(dst.costs, dst.costs);
  dst.weight = src.weight;
}

void parse_label(parser* p, shared_data*, CB::label& ld, std::vector<VW::string_view>& words)
{
  ld.weight = 1.0;

  for (auto const& word : words)
  {
    cb_class f;
    tokenize(':', word, p->parse_name);

    if (p->parse_name.empty() || p->parse_name.size() > 3) { THROW("malformed cost specification: " << word); }

    f.partial_prediction = 0.;
    f.action = (uint32_t)hashstring(p->parse_name[0].begin(), p->parse_name[0].length(), 0);
    f.cost = FLT_MAX;

    if (p->parse_name.size() > 1) f.cost = float_of_string(p->parse_name[1]);

    if (std::isnan(f.cost)) THROW("error NaN cost (" << p->parse_name[1] << " for action: " << p->parse_name[0]);

    f.probability = .0;
    if (p->parse_name.size() > 2) f.probability = float_of_string(p->parse_name[2]);

    if (std::isnan(f.probability))
      THROW("error NaN probability (" << p->parse_name[2] << " for action: " << p->parse_name[0]);

    if (f.probability > 1.0)
    {
      std::cerr << "invalid probability > 1 specified for an action, resetting to 1." << std::endl;
      f.probability = 1.0;
    }
    if (f.probability < 0.0)
    {
      std::cerr << "invalid probability < 0 specified for an action, resetting to 0." << std::endl;
      f.probability = .0;
    }
    if (p->parse_name[0] == "shared")
    {
      if (p->parse_name.size() == 1) { f.probability = -1.f; }
      else
        std::cerr << "shared feature vectors should not have costs" << std::endl;
    }

    ld.costs.push_back(f);
  }
}

// clang-format off
label_parser cb_label = {
  // default_label
  [](polylabel* v) { CB::default_label(v->cb); },
  // parse_label
  [](parser* p, shared_data* sd, polylabel* v, std::vector<VW::string_view>& words) {
    CB::parse_label(p, sd, v->cb, words);
  },
  // cache_label
  [](polylabel* v, io_buf& cache) { CB::cache_label(v->cb, cache); },
  // read_cached_label
  [](shared_data* sd, polylabel* v, io_buf& cache) { return CB::read_cached_label(sd, v->cb, cache); },
  // delete_label
  [](polylabel* v) { CB::delete_label(v->cb); },
   // get_weight
  [](polylabel*) { return 1.f; },
  // copy_label
  [](polylabel* dst, polylabel* src) {
    if (dst && src) {
      CB::copy_label(dst->cb, src->cb);
    }
  },
  // test_label
  [](polylabel* v) { return CB::test_label(v->cb); },
};
// clang-format on

bool ec_is_example_header(example const& ec)  // example headers just have "shared"
{
  const auto& costs = ec.l.cb.costs;
  if (costs.size() != 1) return false;
  if (costs[0].probability == -1.f) return true;
  return false;
}

void print_update(vw& all, bool is_test, example& ec, multi_ex* ec_seq, bool action_scores)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.logger.quiet && !all.bfgs)
  {
    size_t num_features = ec.num_features;

    size_t pred = ec.pred.multiclass;
    if (ec_seq != nullptr)
    {
      num_features = 0;
      // TODO: code duplication csoaa.cc LabelDict::ec_is_example_header
      for (size_t i = 0; i < (*ec_seq).size(); i++)
        if (!CB::ec_is_example_header(*(*ec_seq)[i])) num_features += (*ec_seq)[i]->num_features;
    }
    std::string label_buf;
    if (is_test)
      label_buf = " unknown";
    else
      label_buf = " known";

    if (action_scores)
    {
      std::ostringstream pred_buf;
      pred_buf << std::setw(shared_data::col_current_predict) << std::right << std::setfill(' ');
      if (!ec.pred.a_s.empty())
        pred_buf << ec.pred.a_s[0].action << ":" << ec.pred.a_s[0].score << "...";
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
}  // namespace CB

namespace CB_EVAL
{
float weight(CB_EVAL::label& ld)
{
  return ld.event.weight;
}

size_t read_cached_label(shared_data* sd, CB_EVAL::label& ld, io_buf& cache)
{
  char* c;
  size_t total = sizeof(uint32_t);
  if (cache.buf_read(c, total) < total)
    return 0;
  ld.action = *(uint32_t*)c;

  return total + CB::read_cached_label(sd, ld.event, cache);
}

void cache_label(CB_EVAL::label& ld, io_buf& cache)
{
  char* c;
  cache.buf_write(c, sizeof(uint32_t));
  *(uint32_t*)c = ld.action;

  CB::cache_label(ld.event, cache);
}

void default_label(CB_EVAL::label& ld)
{
  CB::default_label(ld.event);
  ld.action = 0;
}

bool test_label(CB_EVAL::label& ld)
{
  return CB::test_label(ld.event);
}

void delete_label(CB_EVAL::label& ld)
{
  CB::delete_label(ld.event);
}

void copy_label(CB_EVAL::label& dst, CB_EVAL::label& src)
{
  CB::copy_label(dst.event, src.event);
  dst.action = src.action;
}

void parse_label(parser* p, shared_data* sd, CB_EVAL::label& ld, std::vector<VW::string_view>& words)
{
  if (words.size() < 2) THROW("Evaluation can not happen without an action and an exploration");

  ld.action = (uint32_t)hashstring(words[0].begin(), words[0].length(), 0);

  // Removing the first element of a vector is not efficient at all, every element must be copied/moved.
  const auto stashed_first_token = std::move(words[0]);
  words.erase(words.begin());
  CB::parse_label(p, sd, ld.event, words);
  words.insert(words.begin(), std::move(stashed_first_token));
}

// clang-format off
label_parser cb_eval = {
  // default_label
  [](polylabel* v) { CB_EVAL::default_label(v->cb_eval); },
  // parse_label
  [](parser* p, shared_data* sd, polylabel* v, std::vector<VW::string_view>& words) {
    CB_EVAL::parse_label(p, sd, v->cb_eval, words);
  },
  // cache_label
  [](polylabel* v, io_buf& cache) { CB_EVAL::cache_label(v->cb_eval, cache); },
  // read_cached_label
  [](shared_data* sd, polylabel* v, io_buf& cache) { return CB_EVAL::read_cached_label(sd, v->cb_eval, cache); },
  // delete_label
  [](polylabel* v) { CB_EVAL::delete_label(v->cb_eval); },
   // get_weight
  [](polylabel*) { return 1.f; },
  // copy_label
  [](polylabel* dst, polylabel* src) {
    if (dst && src) {
      CB_EVAL::copy_label(dst->cb_eval, src->cb_eval);
    }
  },
  // test_label
  [](polylabel* v) { return CB_EVAL::test_label(v->cb_eval); },
};
// clang-format on

}  // namespace CB_EVAL
