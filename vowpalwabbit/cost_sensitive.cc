#include "float.h"
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
	  stringstream msg;
	  msg << "error NaN value for: ";
	  msg.write(name[0].begin, name[0].end - name[0].begin);
	  cerr << msg.str() << " terminating." << endl;
	  throw runtime_error(msg.str().c_str());
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

  float weight(void*)
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

  void parse_label(parser* p, shared_data*sd, void* v, v_array<substring>& words)
  {
    label* ld = (label*)v;
    ld->costs.erase();

    // handle shared and label first
    if (words.size() == 1) {
      float fx;
      name_value(words[0], p->parse_name, fx);
      bool eq_shared = substring_eq(p->parse_name[0], "***shared***");
      bool eq_label  = substring_eq(p->parse_name[0], "***label***");
      if (! sd->ldict) {
        eq_shared |= substring_eq(p->parse_name[0], "shared");
        eq_label  |= substring_eq(p->parse_name[0], "label");
      }
      if (eq_shared || eq_label) {
        if (p->parse_name.size() != 1) cerr << "shared feature vectors and label feature vectors should not have costs on: " << words[0] << endl;
        else {
          wclass f = { eq_shared ? -1.f : 0.f, 0, 0., 0.};
          ld->costs.push_back(f);
          return;
        }
      }
    }

    // otherwise this is a "real" example
    for (unsigned int i = 0; i < words.size(); i++) {
      wclass f = {0.,0,0.,0.};
      name_value(words[i], p->parse_name, f.x);
      
      if (p->parse_name.size() == 0) {
        cerr << "invalid cost: specification -- no names on: " << words[i] << endl;
        throw exception();
      }

      if (p->parse_name.size() == 1 || p->parse_name.size() == 2 || p->parse_name.size() == 3) {
        f.class_index = sd->ldict ? sd->ldict->get(p->parse_name[0]) : (uint32_t)hashstring(p->parse_name[0], 0);
        if (p->parse_name.size() == 1 && f.x >= 0)  // test examples are specified just by un-valued class #s
          f.x = FLT_MAX;
      } else {
        cerr << "malformed cost specification on '" << (p->parse_name[0].begin) << "'" << endl;
        throw exception();
      }
      ld->costs.push_back(f);
    }
  }

  label_parser cs_label = {default_label, parse_label, 
				  cache_label, read_cached_label, 
				  delete_label, weight, 
				  copy_label,
				  sizeof(label)};

  void print_update(vw& all, bool is_test, example& ec, const v_array<example*>* ec_seq, bool multilabel)
  {
    if (all.sd->weighted_examples >= all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
        size_t num_current_features = ec.num_features;
        // for csoaa_ldf we want features from the whole (multiline example),
        // not only from one line (the first one) represented by ec
        if (ec_seq != nullptr)
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

	std::string label_buf;
        if (is_test)
          label_buf = " unknown";
        else
          label_buf = " known";

	if (multilabel || all.sd->ldict) {
	  std::ostringstream pred_buf;
	  
	  pred_buf << std::setw(all.sd->col_current_predict) << std::right << std::setfill(' ');
          if (all.sd->ldict) {
            if (multilabel) pred_buf << all.sd->ldict->get(ec.pred.multilabels.label_v[0]);
            else            pred_buf << all.sd->ldict->get(ec.pred.multiclass);
          } else            pred_buf << ec.pred.multilabels.label_v[0];
          if (multilabel) pred_buf <<".....";
	  all.sd->print_update(all.holdout_set_off, all.current_pass, label_buf, pred_buf.str(), 
			       num_current_features, all.progress_add, all.progress_arg);;
	}
	else
	  all.sd->print_update(all.holdout_set_off, all.current_pass, label_buf, ec.pred.multiclass, 
			       num_current_features, all.progress_add, all.progress_arg);
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

    all.sd->update(ec.test_only, loss, 1.f, ec.num_features);
    
    for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
      if (! all.sd->ldict)
        all.print(*sink, (float)ec.pred.multiclass, 0, ec.tag);
      else {
        substring ss_pred = all.sd->ldict->get(ec.pred.multiclass);
        all.print_text(*sink, string(ss_pred.begin, ss_pred.end - ss_pred.begin), ec.tag);
      }

    if (all.raw_prediction > 0) {
      stringstream outputStringStream;
      for (unsigned int i = 0; i < ld.costs.size(); i++) {
        wclass cl = ld.costs[i];
        if (i > 0) outputStringStream << ' ';
        outputStringStream << cl.class_index << ':' << cl.partial_prediction;
      }
      all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
    }

    print_update(all, is_test_label(ec.l.cs), ec, nullptr);
  }

  bool example_is_test(example& ec)
  {
    v_array<COST_SENSITIVE::wclass> costs = ec.l.cs.costs;
    if (costs.size() == 0) return true;
    for (size_t j=0; j<costs.size(); j++)
      if (costs[j].x != FLT_MAX) return false;
    return true;
  }

  bool ec_is_example_header(example& ec)  // example headers look like "0:-1" or just "shared"
  {
    v_array<COST_SENSITIVE::wclass> costs = ec.l.cs.costs;
    if (costs.size() != 1) return false;
    if (costs[0].class_index != 0) return false;
    if (costs[0].x >= 0) return false;
    return true;    
  }
}
