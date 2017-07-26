#include <cstring>
#include <climits>
#include "global_data.h"
#include "vw.h"
#include "vw_exception.h"

#ifndef _WIN32
#define sprintf_s snprintf
#endif

using namespace std;

namespace MULTICLASS
{

char* bufread_label(label_t* ld, char* c)
{ memcpy(&ld->label, c, sizeof(ld->label));
  c += sizeof(ld->label);
  memcpy(&ld->weight, c, sizeof(ld->weight));
  c += sizeof(ld->weight);
  return c;
}

size_t read_cached_label(shared_data*, void* v, io_buf& cache)
{ label_t* ld = (label_t*) v;
  char *c;
  size_t total = sizeof(ld->label)+sizeof(ld->weight);
  if (buf_read(cache, c, total) < total)
    return 0;
  bufread_label(ld,c);

  return total;
}

float weight(void* v)
{ label_t* ld = (label_t*) v;
  return (ld->weight > 0) ? ld->weight : 0.f;
}

char* bufcache_label(label_t* ld, char* c)
{ memcpy(c, &ld->label, sizeof(ld->label));
  c += sizeof(ld->label);
  memcpy(c, &ld->weight, sizeof(ld->weight));
  c += sizeof(ld->weight);
  return c;
}

void cache_label(void* v, io_buf& cache)
{ char *c;
  label_t* ld = (label_t*) v;
  buf_write(cache, c, sizeof(ld->label)+sizeof(ld->weight));
  bufcache_label(ld,c);
}

void default_label(void* v)
{ label_t* ld = (label_t*) v;
  ld->label = (uint32_t)-1;
  ld->weight = 1.;
}

void delete_label(void*) {}

void parse_label(parser*, shared_data*sd, void* v, v_array<substring>& words)
{ label_t* ld = (label_t*)v;

  switch(words.size())
  { case 0:
      break;
    case 1:
      ld->label = sd->ldict ? (uint32_t)sd->ldict->get(words[0]) : int_of_substring(words[0]);
      ld->weight = 1.0;
      break;
    case 2:
      ld->label = sd->ldict ? (uint32_t)sd->ldict->get(words[0]) : int_of_substring(words[0]);
      ld->weight = float_of_substring(words[1]);
      break;
    default:
      cerr << "malformed example!\n";
      cerr << "words.size() = " << words.size() << endl;
  }
  if (ld->label == 0)
    THROW("label 0 is not allowed for multiclass.  Valid labels are {1,k}" << (sd->ldict ? "\nthis likely happened because you specified an invalid label with named labels" : ""));
}

label_parser mc_label = {default_label, parse_label,
                         cache_label, read_cached_label,
                         delete_label, weight,
                         nullptr,
                         sizeof(label_t)
                        };

void print_label_pred(vw& all, example& ec, uint32_t prediction)
{ substring ss_label = all.sd->ldict->get(ec.l.multi.label);
  substring ss_pred  = all.sd->ldict->get(prediction);
  all.sd->print_update(all.holdout_set_off, all.current_pass,
                       !ss_label.begin ? "unknown" : string(ss_label.begin, ss_label.end - ss_label.begin),
                       !ss_pred.begin  ? "unknown" : string(ss_pred.begin, ss_pred.end - ss_pred.begin),
                       ec.num_features, all.progress_add, all.progress_arg);
}

void print_probability(vw& all, example& ec, uint32_t prediction)
{ char temp_str[10];
  sprintf_s(temp_str, 10, "%d(%2.0f%%)", prediction, 100 * ec.pred.scalars[prediction - 1]);

  char label_str[512];
  sprintf_s(label_str, 512, "%u", ec.l.multi.label);

  all.sd->print_update(all.holdout_set_off, all.current_pass, label_str, temp_str,
                       ec.num_features, all.progress_add, all.progress_arg);
}

void print_score(vw& all, example& ec, uint32_t prediction)
{ char temp_str[10];
  sprintf_s(temp_str, 10, "%d", prediction);

  char label_str[512];
  sprintf_s(label_str, 512, "%u", ec.l.multi.label);

  all.sd->print_update(all.holdout_set_off, all.current_pass, label_str, temp_str,
                       ec.num_features, all.progress_add, all.progress_arg);
}

void direct_print_update(vw& all, example& ec, uint32_t prediction)
{ all.sd->print_update(all.holdout_set_off, all.current_pass, ec.l.multi.label, prediction,
                       ec.num_features, all.progress_add, all.progress_arg);
}

template<void (*T)(vw&, example&, uint32_t)>
void print_update(vw& all, example &ec, uint32_t prediction)
{ if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
  { if (! all.sd->ldict)
      T(all, ec, prediction);
    else
      print_label_pred(all, ec, ec.pred.multiclass);
  }
}

void print_update_with_probability(vw& all, example& ec, uint32_t pred) {print_update<print_probability>(all, ec, pred);}
void print_update_with_score(vw& all, example& ec, uint32_t pred) {print_update<print_score>(all, ec, pred);}

void finish_example(vw& all, example& ec)
{ float loss = 0;
  if (ec.l.multi.label != (uint32_t)ec.pred.multiclass && ec.l.multi.label != (uint32_t)-1)
    loss = ec.l.multi.weight;

  all.sd->update(ec.test_only, ec.l.multi.label != (uint32_t)-1, loss, ec.l.multi.weight, ec.num_features);

  for (int sink : all.final_prediction_sink)
    if (! all.sd->ldict)
      all.print(sink, (float)ec.pred.multiclass, 0, ec.tag);
    else
    { substring ss_pred = all.sd->ldict->get(ec.pred.multiclass);
      all.print_text(sink, string(ss_pred.begin, ss_pred.end - ss_pred.begin), ec.tag);
    }

  MULTICLASS::print_update<direct_print_update>(all, ec, ec.pred.multiclass);
  VW::finish_example(all, &ec);
}
}
