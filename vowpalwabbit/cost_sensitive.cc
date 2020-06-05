// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cfloat>
#include "gd.h"
#include "vw.h"
#include "vw_exception.h"
#include <cmath>
#include "vw_string_view.h"

namespace COST_SENSITIVE
{
void name_value(VW::string_view& s, v_array<VW::string_view>& name, float& v)
{
  tokenize(':', s, name);

  switch (name.size())
  {
    case 0:
    case 1:
      v = 1.;
      break;
    case 2:
      v = float_of_string(name[1]);
      if (std::isnan(v))
        THROW("error NaN value for: " << name[0]);
      break;
    default:
      std::cerr << "example with a wierd name.  What is '" << s << "'?\n";
  }
}

char* bufread_label(label* ld, char* c, io_buf& cache)
{
  size_t num = *(size_t*)c;
  ld->costs.clear();
  c += sizeof(size_t);
  size_t total = sizeof(wclass) * num;
  if (cache.buf_read(c, (int)total) < total)
  {
    std::cout << "error in demarshal of cost data" << std::endl;
    return c;
  }
  for (size_t i = 0; i < num; i++)
  {
    wclass temp = *(wclass*)c;
    c += sizeof(wclass);
    ld->costs.push_back(temp);
  }

  return c;
}

size_t read_cached_label(shared_data*, void* v, io_buf& cache)
{
  label* ld = (label*)v;
  ld->costs.clear();
  char* c;
  size_t total = sizeof(size_t);
  if (cache.buf_read(c, (int)total) < total)
    return 0;
  bufread_label(ld, c, cache);

  return total;
}

float weight(void*) { return 1.; }

char* bufcache_label(label* ld, char* c)
{
  *(size_t*)c = ld->costs.size();
  c += sizeof(size_t);
  for (unsigned int i = 0; i < ld->costs.size(); i++)
  {
    *(wclass*)c = ld->costs[i];
    c += sizeof(wclass);
  }
  return c;
}

void cache_label(void* v, io_buf& cache)
{
  char* c;
  label* ld = (label*)v;
  cache.buf_write(c, sizeof(size_t) + sizeof(wclass) * ld->costs.size());
  bufcache_label(ld, c);
}

void default_label(void* v)
{
  label* ld = (label*)v;
  ld->costs.clear();
}

bool test_label(void* v)
{
  label* ld = (label*)v;
  if (ld->costs.size() == 0)
    return true;
  for (unsigned int i = 0; i < ld->costs.size(); i++)
    if (FLT_MAX != ld->costs[i].x)
      return false;
  return true;
}

void delete_label(void* v)
{
  label* ld = (label*)v;
  if (ld)
    ld->costs.delete_v();
}

void copy_label(void* dst, void* src)
{
  if (dst && src)
  {
    label* ldD = (label*)dst;
    label* ldS = (label*)src;
    copy_array(ldD->costs, ldS->costs);
  }
}


void parse_label(parser*, shared_data* sd, void* v, v_array<VW::string_view>& words, v_array<VW::string_view>& parse_name_localcpy)
{
  label* ld = (label*)v;
  ld->costs.clear();

  // handle shared and label first
  if (words.size() == 1)
  {
    float fx;
    name_value(words[0], parse_name_localcpy, fx);
    bool eq_shared = parse_name_localcpy[0] == "***shared***";
    bool eq_label = parse_name_localcpy[0] == "***label***";
    if (!sd->ldict)
    {
      eq_shared |= parse_name_localcpy[0] == "shared";
      eq_label |= parse_name_localcpy[0] == "label";
    }
    if (eq_shared || eq_label)
    {
      if (eq_shared)
      {
        if (parse_name_localcpy.size() != 1)
          std::cerr << "shared feature vectors should not have costs on: " << words[0] << std::endl;
        else
        {
          wclass f = {-FLT_MAX, 0, 0., 0.};
          ld->costs.push_back(f);
        }
      }
      if (eq_label)
      {
        if (parse_name_localcpy.size() != 2)
          std::cerr << "label feature vectors should have exactly one cost on: " << words[0] << std::endl;
        else
        {
          wclass f = {float_of_string(parse_name_localcpy[1]), 0, 0., 0.};
          ld->costs.push_back(f);
        }
      }
      return;
    }
  }

  // otherwise this is a "real" example
  for (unsigned int i = 0; i < words.size(); i++)
  {
    wclass f = {0., 0, 0., 0.};
    name_value(words[i], parse_name_localcpy, f.x);

    if (parse_name_localcpy.size() == 0)
      THROW(" invalid cost: specification -- no names on: " << words[i]);

    if (parse_name_localcpy.size() == 1 || parse_name_localcpy.size() == 2 || parse_name_localcpy.size() == 3)
    {
      f.class_index = sd->ldict ? (uint32_t)sd->ldict->get(parse_name_localcpy[0])
                                : (uint32_t)hashstring(parse_name_localcpy[0].begin(), parse_name_localcpy[0].length(), 0);
      if (parse_name_localcpy.size() == 1 && f.x >= 0)  // test examples are specified just by un-valued class #s
        f.x = FLT_MAX;
    }
    else
      THROW("malformed cost specification on '" << (parse_name_localcpy[0]) << "'");

    ld->costs.push_back(f);
  }
}

label_parser cs_label = {default_label, parse_label, cache_label, read_cached_label, delete_label, weight, copy_label,
    test_label, sizeof(label)};

void print_update(vw& all, bool is_test, example& ec, multi_ex* ec_seq, bool action_scores, uint32_t prediction)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.logger.quiet && !all.bfgs)
  {
    size_t num_current_features = ec.num_features;
    // for csoaa_ldf we want features from the whole (multiline example),
    // not only from one line (the first one) represented by ec
    if (ec_seq != nullptr)
    {
      num_current_features = 0;
      // TODO: including quadratic and cubic.
      for (auto& ecc : *ec_seq) num_current_features += ecc->num_features;
    }

    std::string label_buf;
    if (is_test)
      label_buf = " unknown";
    else
      label_buf = " known";

    if (action_scores || all.sd->ldict)
    {
      std::ostringstream pred_buf;

      pred_buf << std::setw(all.sd->col_current_predict) << std::right << std::setfill(' ');
      if (all.sd->ldict)
      {
        if (action_scores)
          pred_buf << all.sd->ldict->get(ec.pred.a_s[0].action);
        else
          pred_buf << all.sd->ldict->get(prediction);
      }
      else
        pred_buf << ec.pred.a_s[0].action;
      if (action_scores)
        pred_buf << ".....";
      all.sd->print_update(all.holdout_set_off, all.current_pass, label_buf, pred_buf.str(), num_current_features,
          all.progress_add, all.progress_arg);
      ;
    }
    else
      all.sd->print_update(all.holdout_set_off, all.current_pass, label_buf, prediction, num_current_features,
          all.progress_add, all.progress_arg);
  }
}

void output_example(vw& all, example& ec)
{
  label& ld = ec.l.cs;

  float loss = 0.;
  if (!test_label(&ld))
  {
    // need to compute exact loss
    size_t pred = (size_t)ec.pred.multiclass;

    float chosen_loss = FLT_MAX;
    float min = FLT_MAX;
    for (auto& cl : ld.costs)
    {
      if (cl.class_index == pred)
        chosen_loss = cl.x;
      if (cl.x < min)
        min = cl.x;
    }
    if (chosen_loss == FLT_MAX)
      std::cerr << "warning: csoaa predicted an invalid class. Are all multi-class labels in the {1..k} range?"
                << std::endl;

    loss = (chosen_loss - min) * ec.weight;
    // TODO(alberto): add option somewhere to allow using absolute loss instead?
    // loss = chosen_loss;
  }

  all.sd->update(ec.test_only, !test_label(&ld), loss, ec.weight, ec.num_features);

  for (auto& sink : all.final_prediction_sink)
  {
    if (!all.sd->ldict)
    {
      all.print_by_ref(sink.get(), (float)ec.pred.multiclass, 0, ec.tag);
    }
    else
    {
      VW::string_view sv_pred = all.sd->ldict->get(ec.pred.multiclass);
      all.print_text_by_ref(sink.get(), sv_pred.to_string(), ec.tag);
    }
  }

  if (all.raw_prediction != nullptr)
  {
    std::stringstream outputStringStream;
    for (unsigned int i = 0; i < ld.costs.size(); i++)
    {
      wclass cl = ld.costs[i];
      if (i > 0)
        outputStringStream << ' ';
      outputStringStream << cl.class_index << ':' << cl.partial_prediction;
    }
    all.print_text_by_ref(all.raw_prediction.get(), outputStringStream.str(), ec.tag);
  }

  print_update(all, test_label(&ec.l.cs), ec, nullptr, false, ec.pred.multiclass);
}

void finish_example(vw& all, example& ec)
{
  output_example(all, ec);
  VW::finish_example(all, ec);
}

bool example_is_test(example& ec)
{
  v_array<COST_SENSITIVE::wclass> costs = ec.l.cs.costs;
  if (costs.size() == 0)
    return true;
  for (size_t j = 0; j < costs.size(); j++)
    if (costs[j].x != FLT_MAX)
      return false;
  return true;
}

bool ec_is_example_header(example const& ec)  // example headers look like "shared"
{
  v_array<COST_SENSITIVE::wclass> costs = ec.l.cs.costs;
  if (costs.size() != 1)
    return false;
  if (costs[0].class_index != 0)
    return false;
  if (costs[0].x != -FLT_MAX)
    return false;
  return true;
}
}  // namespace COST_SENSITIVE
