#include <float.h>
#include <math.h>
#include <stdio.h>

#include "csoaa.h"
#include "simple_label.h"
#include "cache.h"
#include "oaa.h"
#include "v_hashmap.h"

using namespace std;
size_t hashstring (substring s, unsigned long h);

namespace CSOAA {

  void name_value(substring &s, v_array<substring>& name, float &v)
  {
    tokenize(':', s, name);
    
    switch (name.index()) {
    case 0:
    case 1:
      v = 1.;
      break;
    case 2:
      v = float_of_substring(name[1]);
      if ( isnan(v))
	{
	  cerr << "error NaN value for: ";
	  cerr.write(name[0].begin, name[0].end - name[0].begin);
	  cerr << " terminating." << endl;
	  exit(1);
	}
      break;
    default:
      cerr << "example with a wierd name.  What is ";
      cerr.write(s.begin, s.end - s.begin);
      cerr << "\n";
    }
  }

  bool is_test_label(label* ld)
  {
    if (ld->costs.index() == 0)
      return true;
    for (size_t i=0; i<ld->costs.index(); i++)
      if (FLT_MAX != ld->costs[i].x)
        return false;
    return true;
  }
  
  char* bufread_label(label* ld, char* c, io_buf& cache)
  {
    uint32_t num = *(uint32_t *)c;
    ld->costs.erase();
    c += sizeof(uint32_t);
    size_t total = sizeof(wclass)*num;
    if (buf_read(cache, c, total) < total) 
      {
        cout << "error in demarshal of cost data" << endl;
        return c;
      }
    for (uint32_t i = 0; i<num; i++)
      {
        wclass temp = *(wclass *)c;
        c += sizeof(wclass);
        push(ld->costs, temp);
      }
  
    return c;
  }

  size_t read_cached_label(shared_data*, void* v, io_buf& cache)
  {
    label* ld = (label*) v;
    ld->costs.erase();
    char *c;
    size_t total = sizeof(uint32_t);
    if (buf_read(cache, c, total) < total) 
      return 0;
    c = bufread_label(ld,c, cache);
  
    return total;
  }

  float weight(void* v)
  {
    return 1.;
  }

  float initial(void* v)
  {
    return 0.;
  }

  char* bufcache_label(label* ld, char* c)
  {
    *(uint32_t *)c = ld->costs.index();
    c += sizeof(uint32_t);
    for (size_t i = 0; i< ld->costs.index(); i++)
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
    buf_write(cache, c, sizeof(uint32_t)+sizeof(wclass)*ld->costs.index());
    bufcache_label(ld,c);
  }

  void default_label(void* v)
  {
  }

  void delete_label(void* v)
  {
    label* ld = (label*)v;
    ld->costs.erase();
    free(ld->costs.begin);
  }

  //nonreentrant
  size_t increment=0;
  v_array<substring> name;

  void parse_label(shared_data* sd, void* v, v_array<substring>& words)
  {
    label* ld = (label*)v;

    ld->costs.erase();
    for (size_t i = 0; i < words.index(); i++)
      {
        wclass f = {0.,0,0.};
        name_value(words[i], name, f.x);
      
        f.weight_index = 0;
        if (name.index() == 1 || name.index() == 2)
          {
            f.weight_index = hashstring(name[0], 0);
            if (f.weight_index == 0 || f.weight_index > sd->k)
              cerr << "invalid cost specification: " << f.weight_index << endl;
          }
        else 
          cerr << "malformed cost specification!" << endl;
      
        if (name.index() == 1)
          f.x = FLT_MAX;
      
        push(ld->costs, f);
      }

    if (words.index() == 0)
      {
        for (size_t i = 1; i <= sd->k; i++)
          {
            wclass f = {f.x, i, 0.};
            push(ld->costs, f);
          }
      }
  }

  void print_update(vw& all, bool is_test, example *ec)
  {
    if (all.sd->weighted_examples > all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
        char label_buf[32];
        if (is_test)
          strcpy(label_buf," unknown");
        else
          sprintf(label_buf," known");

        fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %8i %8lu\n",
                all.sd->sum_loss/all.sd->weighted_examples,
                all.sd->sum_loss_since_last_dump / (all.sd->weighted_examples - all.sd->old_weighted_examples),
                (long int)all.sd->example_number,
                all.sd->weighted_examples,
                label_buf,
                *(OAA::prediction_t*)&ec->final_prediction,
                (long unsigned int)ec->num_features);
     
        all.sd->sum_loss_since_last_dump = 0.0;
        all.sd->old_weighted_examples = all.sd->weighted_examples;
        all.sd->dump_interval *= 2;
      }
  }

  void output_example(vw& all, example* ec)
  {
    label* ld = (label*)ec->ld;
    all.sd->weighted_examples += 1.;
    all.sd->total_features += ec->num_features;
    float loss = 0.;
    if (!is_test_label(ld))
      {//need to compute exact loss
        size_t pred = *(OAA::prediction_t*)&ec->final_prediction;

        float chosen_loss = FLT_MAX;
        float min = FLT_MAX;
        for (wclass *cl = ld->costs.begin; cl != ld->costs.end; cl ++) {
          if (cl->weight_index == pred)
            chosen_loss = cl->x;
          if (cl->x < min)
            min = cl->x;
        }
        if (chosen_loss == FLT_MAX)
          cerr << "warning: csoaa predicted an invalid class" << endl;

        loss = chosen_loss - min;
      }

    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
  
    for (size_t* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
      all.print(*sink, *(OAA::prediction_t*)&(ec->final_prediction), 0, ec->tag);

    if (all.raw_prediction > 0) {
      string outputString;
      stringstream outputStringStream(outputString);
      for (size_t i = 0; i < ld->costs.index(); i++) {
        wclass cl = ld->costs[i];
        if (i > 0) outputStringStream << ' ';
        outputStringStream << cl.weight_index << ':' << cl.partial_prediction;
      }
      outputStringStream << endl;
      all.print_text(all.raw_prediction, outputStringStream.str(), ec->tag);
    }

    all.sd->example_number++;

    print_update(all, is_test_label((label*)ec->ld), ec);
  }

  void (*base_learner)(void*, example*) = NULL;
  void (*base_finish)(void*) = NULL;

  void learn(void* a, example* ec) {
    vw* all = (vw*)a;
    label* ld = (label*)ec->ld;
    float prediction = 1;
    float score = FLT_MAX;
    size_t current_increment = 0;

    for (wclass *cl = ld->costs.begin; cl != ld->costs.end; cl ++)
      {
        size_t i = cl->weight_index;
	
	label_data simple_temp;
	simple_temp.initial = 0.;

	if (cl->x == FLT_MAX)
	  {
            //cerr << "csoaa.learn: test  example" << endl;
	    simple_temp.label = FLT_MAX;
	    simple_temp.weight = 0.;
	  }
	else
	  {
            //cerr << "csoaa.learn: train example" << endl;
	    simple_temp.label = cl->x;
	    simple_temp.weight = 1.;
	  }

	ec->ld = &simple_temp;

        size_t desired_increment = increment * (i-1);
        if (desired_increment != current_increment) {
	  update_example_indicies(all->audit, ec, desired_increment - current_increment);
          current_increment = desired_increment;
        }

	base_learner(all, ec);
        cl->partial_prediction = ec->partial_prediction;
	if (ec->partial_prediction < score) {
          score = ec->partial_prediction;
          prediction = i;
        }
	ec->partial_prediction = 0.;
      }
    ec->ld = ld;
    *(OAA::prediction_t*)&(ec->final_prediction) = prediction;
    if (current_increment != 0)
      update_example_indicies(all->audit, ec, -current_increment);
  }

  void finish(void* a)
  {
    vw* all = (vw*)a;
    if (name.begin != NULL)
      free(name.begin);
    base_finish(all);
  }

  void drive_csoaa(void* in)
  {
    vw* all = (vw*)in;
    example* ec = NULL;
    while ( true )
      {
        if ((ec = get_example(all->p)) != NULL)//semiblocking operation.
          {
            learn(all, ec);
            output_example(*all, ec);
	    VW::finish_example(*all, ec);
          }
        else if (parser_done(all->p))
          {
            //            finish(all);
            return;
          }
        else 
          ;
      }
  }

  void parse_flags(vw& all, std::vector<std::string>&opts, po::variables_map& vm, size_t s)
  {
    *(all.p->lp) = cs_label_parser;
    all.sd->k = s;
    all.driver = drive_csoaa;
    base_learner = all.learn;
    all.learn = learn;
    base_finish = all.finish;
    all.finish = finish;
    increment = (all.length()/all.sd->k) * all.stride;
  }

  int example_is_test(example* ec)
  {
    v_array<CSOAA::wclass> costs = ((label*)ec->ld)->costs;
    if (costs.index() == 0) return true;
    for (size_t j=0; j<costs.index(); j++)
      if (costs[j].x != FLT_MAX) return false;
    return true;    
  }

}

namespace CSOAA_LDF {
  v_array<example*> ec_seq = v_array<example*>();
  size_t read_example_this_loop = 0;
  bool need_to_clear = true;
  bool is_singleline = true;

  void (*base_learner)(void*, example*) = NULL;
  void (*base_finish)(void*) = NULL;

  void do_actual_learning(vw& all)
  {
    if (ec_seq.index() <= 0) return;  // nothing to do

    if (LabelDict::ec_seq_is_label_definition(ec_seq)) {
      for (size_t i=0; i<ec_seq.index(); i++) {
        v_array<feature> features;
        for (feature*f=ec_seq[i]->atomics[ec_seq[i]->indices[0]].begin; f!=ec_seq[i]->atomics[ec_seq[i]->indices[0]].end; f++) {
          feature fnew = { f->x,  f->weight_index };
          push(features, fnew);
        }

        v_array<CSOAA::wclass> costs = ((CSOAA::label*)ec_seq[i]->ld)->costs;
        for (size_t j=0; j<costs.index(); j++) {
          size_t lab = costs[j].weight_index;
          LabelDict::set_label_features(lab, features);
        }
      }
      return;
    }

    int K = ec_seq.index();
    float min_cost = FLT_MAX;
    float min_score = FLT_MAX;
    size_t prediction = 0;
    float prediction_cost = 0.;
    bool isTest = CSOAA::example_is_test(*ec_seq.begin);
    //cerr<< "csoaa_ldf.learn isTest=" << isTest << ", K=" << K << ", pass=" << ec_seq.begin[0]->pass <<endl;
    for (int k=0; k<K; k++) {
      example *ec = ec_seq.begin[k];
      label   *ld = (label*)ec->ld;
      v_array<CSOAA::wclass> costs = ld->costs;

      if (CSOAA::example_is_test(ec) != isTest) {
        isTest = true;
        cerr << "warning: csoaa_ldf got mix of train/test data; assuming test" << endl;
      }

      label_data simple_label;
      for (size_t j=0; j<costs.index(); j++) {
        simple_label.initial = 0.;
        simple_label.label = FLT_MAX;
        simple_label.weight = 0.;
        if (costs[j].x < min_cost) 
          min_cost = costs[j].x;
        ec->partial_prediction = 0.;

        //cerr<< "add_example_namespace " << costs[j].weight_index << ":" << costs[j].x << " oldlen=" << ec->indices.index() << "/" << ec->atomics['l'].index();
        size_t orig_pos = LabelDict::add_example_namespace(ec, costs[j].weight_index);
        //cerr<< " orig_pos=" << orig_pos<< " newlen=" << ec->indices.index() << "/" << ec->atomics['l'].index();

        ec->ld = &simple_label;
        base_learner(&all, ec); // make a prediction
        costs[j].partial_prediction = ec->partial_prediction;
        //cerr<<"pp=" << ec->partial_prediction << " fp=" << ec->final_prediction << " min_score=" << min_score << " wi=" << costs[j].weight_index << " x=" << costs[j].x<<endl;
        if (ec->partial_prediction < min_score) {
          //cerr<<"updated pp"<<endl;
          min_score = ec->partial_prediction;
          prediction = costs[j].weight_index;
          prediction_cost = costs[j].x;
        }

        //cerr<< " predellen=" << ec->indices.index() << "/" << ec->atomics['l'].index();
        LabelDict::del_example_namespace(ec, costs[j].weight_index, orig_pos);
        //cerr<< " postdellen=" << ec->indices.index() << "/" << ec->atomics['l'].index()<<endl;
      }

      ec->ld = ld;
    }
    prediction_cost -= min_cost;
    //cerr<<"prediction="<<prediction<<endl;

    // do actual learning
    for (int k=0; k<K; k++) {
      example *ec = ec_seq.begin[k];
      label   *ld = (label*)ec->ld;
      v_array<CSOAA::wclass> costs = ld->costs;

      // learn
      label_data simple_label;
      bool prediction_is_me = false;
      for (size_t j=0; j<costs.index(); j++) {
        if (all.training && !isTest) {
          simple_label.initial = 0.;
          simple_label.label = costs[j].x;
          simple_label.weight = 1.;
          ec->ld = &simple_label;
          ec->partial_prediction = 0.;
          size_t orig_pos = LabelDict::add_example_namespace(ec, costs[j].weight_index);
          base_learner(&all, ec);
          LabelDict::del_example_namespace(ec, costs[j].weight_index, orig_pos);
        }

        // fill in test predictions
        ec->partial_prediction = costs[j].partial_prediction;
        if (prediction == costs[j].weight_index) prediction_is_me = true;
      }
      *(OAA::prediction_t*)&(ec->final_prediction) = prediction_is_me ? prediction : 0;

      // restore label
      ec->ld = ld;
    }
  }

  void output_example(vw& all, example* ec)
  {
    //cerr<<"output_example"<<endl;
    label* ld = (label*)ec->ld;
    v_array<CSOAA::wclass> costs = ld->costs;

    if (OAA::example_is_newline(ec)) 
      return;
    if (LabelDict::ec_is_label_definition(ec))
      return;

    all.sd->total_features += ec->num_features;

    float loss = 0.;
    size_t final_pred = *(OAA::prediction_t*)&(ec->final_prediction);
    //cerr<<"final_prediction="<<final_pred<<endl;;

    if (!CSOAA::example_is_test(ec)) {
      for (size_t j=0; j<costs.index(); j++) {
        if (final_pred == costs[j].weight_index)
          loss = costs[j].x;
      }

    //    cerr << "ex eit=" << example_is_test(ec) << " pred=" << final_pred << "/" << (ec->final_prediction >= 0.999) << " weight=" << ld->weight << " loss=" << loss << endl;
      all.sd->sum_loss += loss;
      all.sd->sum_loss_since_last_dump += loss;
      assert(loss >= 0);
    }
  
    for (size_t i = 0; i<all.final_prediction_sink.index(); i++) {
      int f = all.final_prediction_sink[i];
      all.print(f, *(OAA::prediction_t*)&ec->final_prediction, 0, ec->tag);
    }

    //cerr<<"print_update"<<endl;
    CSOAA::print_update(all, CSOAA::example_is_test(ec), ec);
  }

  void output_example_seq(vw& all)
  {
    if ((ec_seq.index() > 0) && !LabelDict::ec_seq_is_label_definition(ec_seq)) {
      all.sd->weighted_examples += 1;
      all.sd->example_number++;

      for (example** ecc=ec_seq.begin; ecc!=ec_seq.end; ecc++)
        output_example(all, *ecc);
    }
  }

  void clear_seq(vw& all)
  {
    //cerr << "clear_seq" << endl;
    if (ec_seq.index() > 0) 
      for (example** ecc=ec_seq.begin; ecc!=ec_seq.end; ecc++)
        VW::finish_example(all, *ecc);
    ec_seq.erase();
  }

  void learn_multiline(vw*all, example *ec) {
    if (ec_seq.index() >= all->p->ring_size - 2) { // give some wiggle room
      cerr << "warning: length of sequence at " << ec->example_counter << " exceeds ring size; breaking apart" << endl;
      do_actual_learning(*all);
      need_to_clear = true;
    }

    if (need_to_clear) {
      ec_seq.erase();
      need_to_clear = false;
    }

    if (OAA::example_is_newline(ec)) {
      //cerr << "ein" << endl;
      do_actual_learning(*all);
      if (!LabelDict::ec_seq_is_label_definition(ec_seq))
        global_print_newline(*all);
      push(ec_seq, ec);
      need_to_clear = true;
    } else {
      push(ec_seq, ec);
    }
  }

  void learn_singleline(vw*all, example*ec) {
    ec_seq.erase();
    push(ec_seq, ec);
    do_actual_learning(*all);
    ec_seq.erase();
  }

  void learn(void*a, example*ec) {
    vw* all = (vw*)a;
    if (is_singleline) learn_singleline(all, ec);
    else learn_multiline(all, ec);
  }

  void finish(void* a)
  {
    vw* all = (vw*)a;
    base_finish(all);
    LabelDict::free_label_features();
  }

  void drive_csoaa_ldf_singleline(vw*all) {
    example* ec = NULL;
    while (true) {
      if ((ec = get_example(all->p)) != NULL) { //semiblocking operation.
        v_array<CSOAA::wclass> costs = ((label*)ec->ld)->costs;
        //cerr<<"weights ="; for (size_t j=0; j<costs.index(); j++) //cerr<<" " << costs[j].weight_index << ":"<<costs[j].x; //cerr<<endl;

        learn_singleline(all, ec);
        if (! LabelDict::ec_is_label_definition(ec)) {
          all->sd->weighted_examples += 1;
          all->sd->example_number++;
        }
        output_example(*all, ec);
        VW::finish_example(*all, ec);
      } else if (parser_done(all->p)) {
        return;
      }
    }
  }

  void drive_csoaa_ldf_multiline(vw*all) {
    example* ec = NULL;
    read_example_this_loop = 0;
    need_to_clear = false;
    while (true) {
      if ((ec = get_example(all->p)) != NULL) { // semiblocking operation
        //cerr << "learn" << endl;
        learn_multiline(all, ec);
        //cerr << "ntc=" << need_to_clear << endl;
        if (need_to_clear) {
          output_example_seq(*all);
          clear_seq(*all);
          need_to_clear = false;
        }
      } else if (parser_done(all->p)) {
        do_actual_learning(*all);
        output_example_seq(*all);
        clear_seq(*all);
        if (ec_seq.begin != NULL)
          free(ec_seq.begin);
        return;
      }
    }
  }

  void drive_csoaa_ldf(void*in)
  {
    vw* all =(vw*)in;
    if (is_singleline)
      drive_csoaa_ldf_singleline(all);
    else
      drive_csoaa_ldf_multiline(all);
  }


  void parse_flags(vw& all, std::string ldf_arg, std::vector<std::string>&opts, po::variables_map& vm, size_t s)
  {
    *(all.p->lp) = CSOAA::cs_label_parser;

    all.sd->k = -1;

    if (ldf_arg.compare("singleline") == 0 || ldf_arg.compare("s") == 0)
      is_singleline = true;
    else if (ldf_arg.compare("multiline") == 0 || ldf_arg.compare("m") == 0)
      is_singleline = false;
    else {
      cerr << "csoaa_ldf requires either [s]ingleline or [m]ultiline argument" << endl;
      exit(-1);
    }

    if (all.add_constant) {
      //cerr << "warning: turning off constant for label dependent features; use --noconstant" << endl;
      all.add_constant = false;
    }

    all.driver = drive_csoaa_ldf;
    base_learner = all.learn;
    all.learn = learn;
    base_finish = all.finish;
    all.finish = finish;
  }

  void global_print_newline(vw& all)
  {
    char temp[1];
    temp[0] = '\n';
    for (size_t i=0; i<all.final_prediction_sink.index(); i++) {
      int f = all.final_prediction_sink[i];
      ssize_t t = write(f, temp, 1);
      if (t != 1)
        std::cerr << "write error" << std::endl;
    }
  }

}


namespace LabelDict {
  bool size_t_eq(size_t a, size_t b) { return (a==b); }
  v_hashmap< size_t, v_array<feature> > label_features(256, v_array<feature>(), size_t_eq);

  size_t hash_lab(size_t lab) { return 328051 + 94389193 * lab; }
  
  bool ec_is_label_definition(example*ec)
  {
    //cerr << "[" << ((OAA::mc_label*)ec->ld)->label << " " << ((OAA::mc_label*)ec->ld)->weight << "] ";
    v_array<CSOAA::wclass> costs = ((CSOAA::label*)ec->ld)->costs;
    for (size_t j=0; j<costs.index(); j++)
      if (costs[j].x >= 0.) return false;
    if (ec->indices.index() == 0) return false;
    if (ec->indices.index() >  2) return false;
    if (ec->indices[0] != 'l') return false;
    return true;    
  }

  bool ec_seq_is_label_definition(v_array<example*>ec_seq)
  {
    //cerr << "ec_seq_is_label_definition: " << ec_seq.index() << endl;
    if (ec_seq.index() == 0) return false;
    bool is_lab = ec_is_label_definition(ec_seq[0]);
    //cerr << "is_lab=" << is_lab << endl;
    for (size_t i=1; i<ec_seq.index(); i++) {
      if (is_lab != ec_is_label_definition(ec_seq[i])) {
        if (!((i == ec_seq.index()-1) && (OAA::example_is_newline(ec_seq[i])))) {
          cerr << "error: mixed label definition and examples in ldf data!" << endl;
          exit(-1);
        }
      }
    }
    //cerr << "is_lab=" << is_lab << endl << endl;
    return is_lab;
  }

  size_t add_example_namespace(example*ec, size_t lab) {
    size_t lab_hash = hash_lab(lab);
    v_array<feature> features = label_features.get(lab, lab_hash);
    if (features.index() == 0) return 0;

    bool has_l = false;
    for (size_t i=0; i<ec->indices.index(); i++) {
      if (ec->indices[i] == 'l') {
        has_l = true;
        break;
      }
    }
    size_t original_index = 0;
    if (has_l) {
      original_index = ec->atomics['l'].index();
      ec->total_sum_feat_sq -= ec->sum_feat_sq['l'];
    } else {
      push(ec->indices, (size_t)'l');
    }

    for (feature*f=features.begin; f!=features.end; f++) {
      ec->sum_feat_sq['l'] += f->x * f->x;
      push(ec->atomics['l'], *f);
    }

    ec->num_features += features.index();
    ec->total_sum_feat_sq += ec->sum_feat_sq['l'];
    return original_index;
  }

  void del_example_namespace(example* ec, size_t lab, size_t original_index) {
    size_t lab_hash = hash_lab(lab);
    v_array<feature> features = label_features.get(lab, lab_hash);
    if (features.index() == 0) return;

    ec->num_features -= features.index();

    if (original_index == 0) {  // did NOT have "l"
      assert(ec->indices.index() > 0);
      ec->indices.pop();
      ec->total_sum_feat_sq -= ec->sum_feat_sq['l'];
      ec->atomics['l'].erase();
    } else { // DID have "l"
      for (feature*f=features.begin; f!=features.end; f++) {
        ec->sum_feat_sq['l'] -= f->x * f->x;
        ec->atomics['l'].pop();
      }
    }
  }

  void set_label_features(size_t lab, v_array<feature>features) {
    size_t lab_hash = hash_lab(lab);
    if (label_features.contains(lab, lab_hash)) { return; }
    /*      v_array<feature> features2 = label_features.get(lab, lab_hash);
      features2.erase();
      free(features2.begin);
      } */
    label_features.put_after_get(lab, lab_hash, features);
  }

  void free_label_features() {
    void* label_iter = LabelDict::label_features.iterator();
    while (label_iter != NULL) {
      v_array<feature> features = LabelDict::label_features.iterator_get_value(label_iter);
      features.erase();
      free(features.begin);

      label_iter = LabelDict::label_features.iterator_next(label_iter);
    }
  }
}
