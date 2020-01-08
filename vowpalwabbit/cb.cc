// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cfloat>

#include "example.h"
#include "parse_primitives.h"
#include "vw.h"
#include "vw_exception.h"

using namespace LEARNER;

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

size_t read_cached_label(shared_data* s, new_polylabel& v, io_buf& cache)
{
  return CB::read_cached_label(s, v.init_as_cb(), cache);
}

float weight(CB::label& ld) { return ld.weight; }

float weight(new_polylabel& v) { return CB::weight(v.cb()); }

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

void cache_label(new_polylabel& v, io_buf& cache) { CB::cache_label(v.cb(), cache); }

void default_label(CB::label& ld)
{
  ld.costs.clear();
  ld.weight = 1;
}

void default_label(new_polylabel& v)
{
  if (v.get_type() != label_type_t::unset)
  {
    v.reset();
  }
  CB::default_label(v.init_as_cb());
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

bool test_label(new_polylabel& v) { return CB::test_label(v.cb()); }

void parse_label(parser* p, shared_data*, CB::label& ld, v_array<VW::string_view>& words)
{
  ld.costs.clear();
  ld.weight = 1.0;

  for (auto const& word : words)
  {
    cb_class f;
    tokenize(':', word, p->parse_name);

    if (p->parse_name.empty() || p->parse_name.size() > 3)
      THROW("malformed cost specification: " << p->parse_name);

    f.partial_prediction = 0.;
    f.action = (uint32_t)hashstring(p->parse_name[0].begin(), p->parse_name[0].length(), 0);
    f.cost = FLT_MAX;

    if (p->parse_name.size() > 1)
      f.cost = float_of_string(p->parse_name[1]);

    if (std::isnan(f.cost))
      THROW("error NaN cost (" << p->parse_name[1] << " for action: " << p->parse_name[0]);

    f.probability = .0;
    if (p->parse_name.size() > 2)
      f.probability = float_of_string(p->parse_name[2]);

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
      if (p->parse_name.size() == 1)
      {
        f.probability = -1.f;
      }
      else
        std::cerr << "shared feature vectors should not have costs" << std::endl;
    }

    ld.costs.push_back(f);
  }
}

void parse_label(parser* p, shared_data* sd, new_polylabel& v, v_array<VW::string_view>& words)
{
  CB::parse_label(p, sd, v.cb(), words);
}

label_parser cb_label = {default_label, parse_label, cache_label, read_cached_label, weight, test_label, sizeof(label)};

bool ec_is_example_header(example const& ec)  // example headers just have "shared"
{
  if (ec.l.get_type() == label_type_t::cb)
  {
    const v_array<CB::cb_class>& costs = ec.l.cb().costs;
    if (costs.size() != 1)
      return false;
    if (costs[0].probability == -1.f)
      return true;
  }

  return false;
}

void print_update(vw& all, bool is_test, example& ec, multi_ex* ec_seq, bool action_scores)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
  {
    size_t num_features = ec.num_features;

    if (ec_seq != nullptr)
    {
      num_features = 0;
      // TODO: code duplication csoaa.cc LabelDict::ec_is_example_header
      for (size_t i = 0; i < (*ec_seq).size(); i++)
        if (!CB::ec_is_example_header(*(*ec_seq)[i]))
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
      auto& action_scores = ec.pred.action_scores();
      pred_buf << std::setw(shared_data::col_current_predict) << std::right << std::setfill(' ');
      if (!action_scores.empty())
        pred_buf << ec.pred.action_scores()[0].action << ":" << action_scores[0].score << "...";
      else
        pred_buf << "no action";
      all.sd->print_update(all.holdout_set_off, all.current_pass, label_buf, pred_buf.str(), num_features,
          all.progress_add, all.progress_arg);
    }
    else
    {
      size_t pred = ec.pred.multiclass();
      all.sd->print_update(all.holdout_set_off, all.current_pass, label_buf, (uint32_t)pred, num_features,
          all.progress_add, all.progress_arg);
    }
  }
}
}  // namespace CB

namespace CB_EVAL
{
float weight(new_polylabel& v)
{
  auto& ld = v.cb_eval();
  return ld.event.weight;
}

size_t read_cached_label(shared_data* sd, new_polylabel& v, io_buf& cache)
{
  auto& ld = v.cb_eval();
  char* c;
  size_t total = sizeof(uint32_t);
  if (cache.buf_read(c, total) < total)
    return 0;
  ld.action = *(uint32_t*)c;

  return total + CB::read_cached_label(sd, ld.event, cache);
}

void cache_label(new_polylabel& v, io_buf& cache)
{
  char* c;
  auto& ld = v.cb_eval();
  cache.buf_write(c, sizeof(uint32_t));
  *(uint32_t*)c = ld.action;

  CB::cache_label(ld.event, cache);
}

void default_label(new_polylabel& v)
{
  auto& ld = v.init_as_cb_eval();
  CB::default_label(ld.event);
  ld.action = 0;
}

bool test_label(new_polylabel& v)
{
  auto& ld = v.cb_eval();
  return CB::test_label(ld.event);
}

void parse_label(parser* p, shared_data* sd, new_polylabel& v, v_array<VW::string_view>& words)
{
  auto& ld = v.cb_eval();

  if (words.size() < 2)
    THROW("Evaluation can not happen without an action and an exploration");

  ld.action = (uint32_t)hashstring(words[0].begin(), words[0].length(), 0);

  words.begin()++;

  CB::parse_label(p, sd, ld.event, words);

  words.begin()--;
}

label_parser cb_eval = {
    default_label, parse_label, cache_label, read_cached_label, weight, test_label, sizeof(CB_EVAL::label)};
}  // namespace CB_EVAL
