// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cfloat>
#include "gd.h"
#include "vw.h"

namespace MULTILABEL
{
char* bufread_label(labels* ld, char* c, io_buf& cache)
{
  size_t num = *(size_t*)c;
  ld->label_v.clear();
  c += sizeof(size_t);
  size_t total = sizeof(uint32_t) * num;
  if (cache.buf_read(c, (int)total) < total)
  {
    std::cout << "error in demarshal of cost data" << std::endl;
    return c;
  }
  for (size_t i = 0; i < num; i++)
  {
    uint32_t temp = *(uint32_t*)c;
    c += sizeof(uint32_t);
    ld->label_v.push_back(temp);
  }

  return c;
}

size_t read_cached_label(shared_data*, void* v, io_buf& cache)
{
  labels* ld = (labels*)v;
  ld->label_v.clear();
  char* c;
  size_t total = sizeof(size_t);
  if (cache.buf_read(c, (int)total) < total)
    return 0;
  bufread_label(ld, c, cache);

  return total;
}

float weight(void*) { return 1.; }

char* bufcache_label(labels* ld, char* c)
{
  *(size_t*)c = ld->label_v.size();
  c += sizeof(size_t);
  for (unsigned int i = 0; i < ld->label_v.size(); i++)
  {
    *(uint32_t*)c = ld->label_v[i];
    c += sizeof(uint32_t);
  }
  return c;
}

void cache_label(void* v, io_buf& cache)
{
  char* c;
  labels* ld = (labels*)v;
  cache.buf_write(c, sizeof(size_t) + sizeof(uint32_t) * ld->label_v.size());
  bufcache_label(ld, c);
}

void default_label(void* v)
{
  labels* ld = (labels*)v;
  ld->label_v.clear();
}

bool test_label(void* v)
{
  labels* ld = (labels*)v;
  return ld->label_v.size() == 0;
}

void delete_label(void* v)
{
  labels* ld = (labels*)v;
  if (ld)
    ld->label_v.delete_v();
}

void copy_label(void* dst, void* src)
{
  if (dst && src)
  {
    labels* ldD = (labels*)dst;
    labels* ldS = (labels*)src;
    copy_array(ldD->label_v, ldS->label_v);
  }
}

void parse_label(parser* p, shared_data*, void* v, v_array<VW::string_view>& words)
{
  labels* ld = (labels*)v;

  ld->label_v.clear();
  switch (words.size())
  {
    case 0:
      break;
    case 1:
      tokenize(',', words[0], p->parse_name);

      for (const auto & parse_name : p->parse_name)
      {
        uint32_t n = int_of_string(parse_name);
        ld->label_v.push_back(n);
      }
      break;
    default:
      std::cerr << "example with an odd label, what is ";
      for (const auto & word : words) std::cerr << word << " ";
      std::cerr << std::endl;
  }
}

label_parser multilabel = {default_label, parse_label, cache_label, read_cached_label, delete_label, weight, copy_label,
    test_label, sizeof(labels)};

void print_update(vw& all, bool is_test, example& ec)
{
  if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
  {
    std::stringstream label_string;
    if (is_test)
      label_string << " unknown";
    else
      for (size_t i = 0; i < ec.l.multilabels.label_v.size(); i++) label_string << " " << ec.l.multilabels.label_v[i];

    std::stringstream pred_string;
    for (size_t i = 0; i < ec.pred.multilabels.label_v.size(); i++)
      pred_string << " " << ec.pred.multilabels.label_v[i];

    all.sd->print_update(all.holdout_set_off, all.current_pass, label_string.str(), pred_string.str(), ec.num_features,
        all.progress_add, all.progress_arg);
  }
}

void output_example(vw& all, example& ec)
{
  labels& ld = ec.l.multilabels;

  float loss = 0.;
  if (!test_label(&ld))
  {
    // need to compute exact loss
    labels preds = ec.pred.multilabels;
    labels given = ec.l.multilabels;

    uint32_t preds_index = 0;
    uint32_t given_index = 0;

    while (preds_index < preds.label_v.size() && given_index < given.label_v.size())
    {
      if (preds.label_v[preds_index] < given.label_v[given_index])
      {
        preds_index++;
        loss++;
      }
      else if (preds.label_v[preds_index] > given.label_v[given_index])
      {
        given_index++;
        loss++;
      }
      else
      {
        preds_index++;
        given_index++;
      }
    }
    loss += given.label_v.size() - given_index;
    loss += preds.label_v.size() - preds_index;
  }

  all.sd->update(ec.test_only, !test_label(&ld), loss, 1.f, ec.num_features);

  for (int sink : all.final_prediction_sink)
    if (sink >= 0)
    {
      std::stringstream ss;

      for (size_t i = 0; i < ec.pred.multilabels.label_v.size(); i++)
      {
        if (i > 0)
          ss << ',';
        ss << ec.pred.multilabels.label_v[i];
      }
      ss << ' ';
      all.print_text_by_ref(sink, ss.str(), ec.tag);
    }

  print_update(all, test_label(&ec.l.multilabels), ec);
}
}  // namespace MULTILABEL
