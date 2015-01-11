#include <limits.h>
#include "global_data.h"
#include "vw.h"

namespace MULTICLASS {

  char* bufread_label(label* ld, char* c)
  {
    ld->label = *(uint32_t *)c;
    c += sizeof(ld->label);
    ld->weight = *(float *)c;
    c += sizeof(ld->weight);
    return c;
  }
  
  size_t read_cached_label(shared_data*, void* v, io_buf& cache)
  {
    label* ld = (label*) v;
    char *c;
    size_t total = sizeof(ld->label)+sizeof(ld->weight);
    if (buf_read(cache, c, total) < total) 
      return 0;
    c = bufread_label(ld,c);
    
    return total;
  }
  
  float weight(void* v)
  {
    label* ld = (label*) v;
    return (ld->weight > 0) ? ld->weight : 0.f;
  }
  
  char* bufcache_label(label* ld, char* c)
  {
    *(uint32_t *)c = ld->label;
    c += sizeof(ld->label);
    *(float *)c = ld->weight;
    c += sizeof(ld->weight);
    return c;
  }

  void cache_label(void* v, io_buf& cache)
  {
    char *c;
    label* ld = (label*) v;
    buf_write(cache, c, sizeof(ld->label)+sizeof(ld->weight));
    c = bufcache_label(ld,c);
  }

  void default_label(void* v)
  {
    label* ld = (label*) v;
    ld->label = (uint32_t)-1;
    ld->weight = 1.;
  }

  void delete_label(void* v) {}

  void parse_label(parser* p, shared_data*, void* v, v_array<substring>& words)
  {
    label* ld = (label*)v;

    switch(words.size()) {
    case 0:
      break;
    case 1:
      ld->label = int_of_substring(words[0]);
      ld->weight = 1.0;
      break;
    case 2:
      ld->label = int_of_substring(words[0]);
      ld->weight = float_of_substring(words[1]);
      break;
    default:
      cerr << "malformed example!\n";
      cerr << "words.size() = " << words.size() << endl;
    }
    if (ld->label == 0)
      {
	cout << "label 0 is not allowed for multiclass.  Valid labels are {1,k}" << endl;
	throw exception();
      }
  }

  label_parser mc_label = {default_label, parse_label, 
				  cache_label, read_cached_label, 
				  delete_label, weight, 
				  NULL,
				  sizeof(label)};
  
  void print_update(vw& all, example &ec)
  {
    if (all.sd->weighted_examples >= all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
        label ld = ec.l.multi;
        char label_buf[32];
        if (ld.label == INT_MAX)
          strcpy(label_buf," unknown");
        else
          sprintf(label_buf,"%8ld",(long int)ld.label);
	char pred_buf[32];
	sprintf(pred_buf,"%8lu",(long unsigned int)ec.pred.multiclass);

	all.sd->print_update(all.holdout_set_off, all.current_pass, label_buf, pred_buf, 
			     ec.num_features, all.progress_add, all.progress_arg);
      }
  }

  void finish_example(vw& all, example& ec)
  {
    label ld = ec.l.multi;
    
    size_t loss = 1;
    if (ld.label == (uint32_t)ec.pred.multiclass)
      loss = 0;
    
    all.sd->update(ec.test_only, loss, ld.weight, ec.num_features);

    for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
      all.print(*sink, (float)ec.pred.multiclass, 0, ec.tag);
    
    MULTICLASS::print_update(all, ec);
    VW::finish_example(all, &ec);
  }
}
