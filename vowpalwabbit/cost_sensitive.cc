#include "float.h"
#include "cost_sensitive.h"
#include "gd.h"
#include "vw.h"

namespace COST_SENSITIVE {

  void name_value(substring &s, v_array<substring>& name, float &v)
  {
    tokenize(':', s, name);
    
    switch (name.size()) {
    case 0:
    case 1:
      v = 1.;
      break;
    case 2:
      v = float_of_substring(name[1]);
      if ( nanpattern(v))
	{
	  cerr << "error NaN value for: ";
	  cerr.write(name[0].begin, name[0].end - name[0].begin);
	  cerr << " terminating." << endl;
	  throw exception();
	}
      break;
    default:
      cerr << "example with a wierd name.  What is '";
      cerr.write(s.begin, s.end - s.begin);
      cerr << "'?\n";
    }
  }

  bool is_test_label(label& ld)
  {
    if (ld.costs.size() == 0)
      return true;
    for (unsigned int i=0; i<ld.costs.size(); i++)
      if (FLT_MAX != ld.costs[i].x)
        return false;
    return true;
  }
  
  char* bufread_label(label* ld, char* c, io_buf& cache)
  {
    size_t num = *(size_t *)c;
    ld->costs.erase();
    c += sizeof(size_t);
    size_t total = sizeof(wclass)*num;
    if (buf_read(cache, c, (int)total) < total) 
      {
        cout << "error in demarshal of cost data" << endl;
        return c;
      }
    for (size_t i = 0; i<num; i++)
      {
        wclass temp = *(wclass *)c;
        c += sizeof(wclass);
        ld->costs.push_back(temp);
      }
  
    return c;
  }

  size_t read_cached_label(shared_data*, void* v, io_buf& cache)
  {
    label* ld = (label*) v;
    ld->costs.erase();
    char *c;
    size_t total = sizeof(size_t);
    if (buf_read(cache, c, (int)total) < total) 
      return 0;
    c = bufread_label(ld,c, cache);
  
    return total;
  }

  float weight(void* v)
  {
    return 1.;
  }

  char* bufcache_label(label* ld, char* c)
  {
    *(size_t *)c = ld->costs.size();
    c += sizeof(size_t);
    for (unsigned int i = 0; i< ld->costs.size(); i++)
      {
        *(wclass *)c = ld->costs[i];
        c += sizeof(wclass);
      }
    return c;
  }

  void cache_label(void* v, io_buf& cache)
  {
    char *c;
    label* ld = (label*) v;
    buf_write(cache, c, sizeof(size_t)+sizeof(wclass)*ld->costs.size());
    bufcache_label(ld,c);
  }

  void default_label(void* v)
  {
    label* ld = (label*) v;
    ld->costs.erase();
  }

  void delete_label(void* v)
  {
    label* ld = (label*)v;
    if (ld) ld->costs.delete_v();
  }

  void copy_label(void*dst, void*src)
  {
    if (dst && src) {
      label* ldD = (label*)dst;
      label* ldS = (label*)src;
      copy_array(ldD->costs, ldS->costs);
    }
  }

  bool substring_eq(substring ss, const char* str) {
    size_t len_ss  = ss.end - ss.begin;
    size_t len_str = strlen(str);
    if (len_ss != len_str) return false;
    return (strncmp(ss.begin, str, len_ss) == 0);
  }

  void parse_label(parser* p, shared_data* sd, void* v, v_array<substring>& words)
  {
    label* ld = (label*)v;

    ld->costs.erase();
    for (unsigned int i = 0; i < words.size(); i++) {
      wclass f = {0.,0,0.,0.};
      name_value(words[i], p->parse_name, f.x);
      
      if (p->parse_name.size() == 0)
        cerr << "invalid cost: specification -- no names!" << endl;
      else {
        if (substring_eq(p->parse_name[0], "shared")) {
          if (p->parse_name.size() == 1) {
            f.class_index = 0;
            f.x = -1.f;
          } else
            cerr << "shared feature vectors should not have costs" << endl;
        } else if (substring_eq(p->parse_name[0], "label")) {
          if (p->parse_name.size() == 2) {
            f.class_index = 0;
            // f.x is already set properly
          } else
            cerr << "label feature vectors must have label ids" << endl;
        } else {
          if (p->parse_name.size() == 1 || p->parse_name.size() == 2 || p->parse_name.size() == 3) {
            f.class_index = (uint32_t)hashstring(p->parse_name[0], 0);
            if (p->parse_name.size() == 1 && f.x >= 0)  // test examples are specified just by un-valued class #s
              f.x = FLT_MAX;
          } else 
            cerr << "malformed cost specification on '" << (p->parse_name[0].begin) << "'" << endl;
        }
        ld->costs.push_back(f);
      }
    }

    if (words.size() == 0) {
      if (sd->k != (uint32_t)-1) {
        for (uint32_t i = 1; i <= sd->k; i++) {
          wclass f = {FLT_MAX, i, 0., 0.};
          ld->costs.push_back(f);
        }
      } else {
        //cerr << "ldf test examples must have possible labels listed" << endl;
        //throw exception();
      }
    }
  }

  label_parser cs_label = {default_label, parse_label, 
				  cache_label, read_cached_label, 
				  delete_label, weight, 
				  copy_label,
				  sizeof(label)};

  void print_update(vw& all, bool is_test, example& ec, const v_array<example*>* ec_seq)
  {
    if (all.sd->weighted_examples >= all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
        size_t num_current_features = ec.num_features;
        // for csoaa_ldf we want features from the whole (multiline example),
        // not only from one line (the first one) represented by ec
        if (ec_seq != NULL)
	  {
          num_current_features = 0;
          // If the first example is "shared", don't include its features.
          // These should be already included in each example (TODO: including quadratic and cubic).
          // TODO: code duplication csoaa.cc LabelDict::ec_is_example_header
          example** ecc=ec_seq->begin;
          example& first_ex = **ecc;
	  
	  v_array<COST_SENSITIVE::wclass> costs = first_ex.l.cs.costs;
	  if (costs.size() == 1 && costs[0].class_index == 0 && costs[0].x < 0) ecc++;
	  
          for (; ecc!=ec_seq->end; ecc++)
	    num_current_features += (*ecc)->num_features;
        }

        char label_buf[32];
        if (is_test)
          strcpy(label_buf," unknown");
        else
          sprintf(label_buf," known");

        if(!all.holdout_set_off && all.current_pass >= 1)
        {
          if(all.sd->holdout_sum_loss == 0. && all.sd->weighted_holdout_examples == 0.)
            fprintf(stderr, " unknown   ");
          else
            fprintf(stderr, "%-10.6f " , all.sd->holdout_sum_loss/all.sd->weighted_holdout_examples);

          if(all.sd->holdout_sum_loss_since_last_dump == 0. && all.sd->weighted_holdout_examples_since_last_dump == 0.)
            fprintf(stderr, " unknown   ");
          else
            fprintf(stderr, "%-10.6f " , all.sd->holdout_sum_loss_since_last_dump/all.sd->weighted_holdout_examples_since_last_dump);

          fprintf(stderr, "%8ld %8.1f   %s %8lu %8lu h\n",
                (long int)all.sd->example_number,
                all.sd->weighted_examples,
                label_buf,
                (long unsigned int)ec.pred.multiclass,
                (long unsigned int)num_current_features);

          all.sd->weighted_holdout_examples_since_last_dump = 0;
          all.sd->holdout_sum_loss_since_last_dump = 0.0;
        }
        else
          fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %8lu %8lu\n",
                all.sd->sum_loss/all.sd->weighted_examples,
                all.sd->sum_loss_since_last_dump / (all.sd->weighted_examples - all.sd->old_weighted_examples),
                (long int)all.sd->example_number,
                all.sd->weighted_examples,
                label_buf,
                (long unsigned int)ec.pred.multiclass,
                (long unsigned int)num_current_features);
     
	all.sd->sum_loss_since_last_dump = 0.0;
        all.sd->old_weighted_examples = all.sd->weighted_examples;
        VW::update_dump_interval(all);
      }
  }

  void output_example(vw& all, example& ec)
  {
    label& ld = ec.l.cs;

    float loss = 0.;
    if (!is_test_label(ld))
      {//need to compute exact loss
        size_t pred = (size_t)ec.pred.multiclass;

        float chosen_loss = FLT_MAX;
        float min = FLT_MAX;
        for (wclass *cl = ld.costs.begin; cl != ld.costs.end; cl ++) {
          if (cl->class_index == pred)
            chosen_loss = cl->x;
          if (cl->x < min)
            min = cl->x;
        }
        if (chosen_loss == FLT_MAX)
          cerr << "warning: csoaa predicted an invalid class" << endl;

        loss = chosen_loss - min;
      }

    if(ec.test_only)
      {
        all.sd->weighted_holdout_examples += 1;//test weight seen
        all.sd->weighted_holdout_examples_since_last_dump += 1;
        all.sd->weighted_holdout_examples_since_last_pass += 1;
        all.sd->holdout_sum_loss += loss;
        all.sd->holdout_sum_loss_since_last_dump += loss;
        all.sd->holdout_sum_loss_since_last_pass += loss;//since last pass
     }
    else
      {
        all.sd->weighted_examples += 1.;
        all.sd->total_features += ec.num_features;
        all.sd->sum_loss += loss;
        all.sd->sum_loss_since_last_dump += loss;    
        all.sd->example_number++;
      }

    for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
      all.print((int)*sink, (float)ec.pred.multiclass, 0, ec.tag);

    if (all.raw_prediction > 0) {
      string outputString;
      stringstream outputStringStream(outputString);
      for (unsigned int i = 0; i < ld.costs.size(); i++) {
        wclass cl = ld.costs[i];
        if (i > 0) outputStringStream << ' ';
        outputStringStream << cl.class_index << ':' << cl.partial_prediction;
      }
      //outputStringStream << endl;
      all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
    }

    print_update(all, is_test_label(ec.l.cs), ec, NULL);
  }

  bool example_is_test(example& ec)
  {
    v_array<COST_SENSITIVE::wclass> costs = ec.l.cs.costs;
    if (costs.size() == 0) return true;
    for (size_t j=0; j<costs.size(); j++)
      if (costs[j].x != FLT_MAX) return false;
    return true;    
  }
}
