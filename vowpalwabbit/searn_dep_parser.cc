/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "searn_dep_parser.h"
#include "multiclass.h"
#include "memory.h"
#include "example.h"
#include "gd.h"
#include "ezexample.h"

#define cdep cerr
#undef cdep
#define cdep if (1) {} else cerr

namespace DepParserTask         {  Searn::searn_task task = { "dep_parser", initialize, finish, structured_predict };  }

  struct task_data {
    size_t num_actions;
    ezexample* ex;
    bool no_quadratic_features;
    bool no_cubic_features;
    bool init_feature_template_flag;
  };
namespace DepParserTask {
  using namespace Searn;
  uint32_t max_label = 0;

  void initialize(searn& srn, size_t& num_actions, po::variables_map& vm) {
    task_data * my_task_data = new task_data();
    my_task_data->num_actions = num_actions;
    my_task_data->init_feature_template_flag = false;
    my_task_data->ex = new ezexample(srn.all, alloc_examples(sizeof(COST_SENSITIVE::label), 1), false);
    srn.set_options( 0 );
    srn.set_num_learners(1);
    srn.set_task_data<task_data>(my_task_data);
    po::options_description sspan_opts("dependency parser options");
    sspan_opts.add_options()
      ("dparser_no_quad", "Don't use qudaratic features")
      ("dparser_no_cubic","Don't use cubic features");
    vm = add_options(*srn.all, sspan_opts);
	  
	// setup entity and relation labels
	// Entity label 1:E_Other 2:E_Peop 3:E_Org 4:E_Loc
	// Relation label 5:R_Live_in 6:R_OrgBased_in 7:R_Located_in 8:R_Work_For 9:R_Kill 10:R_None
    my_task_data->no_quadratic_features = (vm.count("dparser_no_quad"))?true:false;
    my_task_data->no_cubic_features =(vm.count("dparser_no_cubic"))?true:false;
  }

  void finish(searn& srn) {
      task_data *data = srn.get_task_data<task_data>();
      dealloc_example(COST_SENSITIVE::cs_label.delete_label, *(data->ex->get_example()));
      free(data->ex->get_example());
      delete data->ex;
      delete data;
  }    // if we had task data, we'd want to free it here

  // arc-hybrid System.
  uint32_t transition_hybrid(searn& srn, uint32_t a_id, uint32_t i, v_array<uint32_t> & stack, v_array<uint32_t> & heads, v_array<uint32_t> & gold_heads){
    switch(a_id) {
    //SHIFT
    case 1:
  		stack.push_back(i);
		  return i+1;

    //RIGHT
    case 2:
  		heads[stack.last()] = stack[stack.size()-2];
      cdep << "make a right link" << stack[stack.size()-2] << " ====> " << (stack.last()) << endl;
      if(stack.last()!=0)
        srn.loss((gold_heads[stack.last()] != heads[stack.last()])/((float)gold_heads.size()));
		  stack.pop();
		  return i;

    //LEFT
    case 3:
  		heads[stack.last()] = i;
      cdep << "make a left link" << stack.last() << "<==== " << i << endl;
      if(stack.last()!=0)
        srn.loss((gold_heads[stack.last()] != heads[stack.last()])/((float)gold_heads.size()));
		  stack.pop();
		  return i;
    }
    cerr << "Unknown action (searn_dep_parser.cc).";
  	return i;
  }

  // We only call this function once (keep readability)
  void initialize_feature_template(searn& srn, size_t num_base_feature_space, v_array<example*> ec_buf){
    task_data *data = srn.get_task_data<task_data>();
    vector<string> newpairs;
    vector<string> newtriples;
    if(data->init_feature_template_flag)
      return;
    data->init_feature_template_flag = true;
    map<string, char> fs_idx_map;
    fs_idx_map["s1"]=0, fs_idx_map["s2"]=1, fs_idx_map["s3"]=2;
    fs_idx_map["b1"]=3, fs_idx_map["b2"]=4, fs_idx_map["b3"]=5;

    string quadratic_feature_template = "s1-s2 s1-b1 s1-s1 s2-s2 s3-s3 b1-b1 b2-b3 b3-b3 ENDQ";
    string cubic_feature_template = "b1-b2-b3 s1-b1-b2 s1-s2-b1 s1-s2-s3 ENDC";
    size_t pos = 0;
    
    // parse quadratic features
    while ((pos = quadratic_feature_template.find(" ")) != std::string::npos) {
      string token = quadratic_feature_template.substr(0, pos);
      char first_fs_idx = fs_idx_map[token.substr(0,token.find("-"))];
      char second_fs_idx = fs_idx_map[token.substr(token.find("-")+1,token.size())];
      for(size_t i=0; i< num_base_feature_space; i++){
        for(size_t j=0; j< num_base_feature_space; j++){
          newpairs.push_back(string(1,(char)(first_fs_idx*num_base_feature_space+i))+\
            string(1,(char)(second_fs_idx*num_base_feature_space+j)));
        }
      }
      quadratic_feature_template.erase(0, pos + 1);
    }

    // parse cubic features
    while ((pos = cubic_feature_template.find(" ")) != std::string::npos) {
      string token = cubic_feature_template.substr(0, pos);
      char first_fs_idx = fs_idx_map[token.substr(0,token.find("-"))];
      token.erase(0, first_fs_idx+1);
      char second_fs_idx = fs_idx_map[token.substr(0,token.find("-"))];
      char third_fs_idx = fs_idx_map[token.substr(token.find("-")+1,token.size())];
      for(size_t i=0; i< num_base_feature_space; i++){
        for(size_t j=0; j< num_base_feature_space; j++){
          for(size_t k=0; k< num_base_feature_space; k++){
            newtriples.push_back(string(1,(char)(first_fs_idx*num_base_feature_space+i))+\
              string(1,(char)(second_fs_idx*num_base_feature_space+j))+\
              string(1,(char)(third_fs_idx*num_base_feature_space+k)));
          }
        }
      }
      cubic_feature_template.erase(0, pos + 1);
    }
    if(!data->no_quadratic_features)
      srn.all->pairs.swap(newpairs);
    if(!data->no_cubic_features)
      srn.all->triples.swap(newtriples);
  }

  // This function needs to be very fast
  void extract_features(searn& srn, uint32_t idx,  vector<example*> &ec, v_array<uint32_t> & stack, v_array<uint32_t> & heads, ezexample &ex){
    // be careful: indices in ec starts from 0, but i is starts from 1
    size_t n = ec.size();
    // use this buffer to collect the examples, default value: NULL
    v_array<example*> ec_buf;
    ec_buf.resize(6, true);

    // feature based on top three examples in stack 
    // ec_buf[0]: s1, ec_buf[1]: s2, ec_buf[2]: s3
    // todo: add features based on leftmost and rightmost child: rc?: the ? rightmost child, lc?: the ? leftmost child
    for(size_t i=1; i<=3; i++){
      if(stack.size()>=i && *(stack.end-i)!=0)
        ec_buf[i-1] = ec[*(stack.end-i)-1];
    }

    // features based on examples in string buffer
    // ec_buf[3]: b1, ec_buf[4]: b2, ec_buf[5]: b3
    // todo: add features based on leftmost child in buffer
    for(size_t i=1; i<=3; i++){
      if(idx+i <= n)
        ec_buf[i+2] = ec[idx+i-1];
    }

    size_t dis = 0;
    if(!stack.empty())
      dis = min(5, idx - stack.last());

    cdep << "start generating features";

    // unigram features
    size_t nfs = ec[0]->indices.size();
    for(size_t i=0; i<6; i++){
      if(!ec_buf[i])
        continue;
      size_t fs_idx_inner = 0;
      // add unigram of all feature_template
      for (unsigned char* j = ec[0]->indices.begin; j != ec[0]->indices.end; j++,fs_idx_inner++){
         ex(*ec_buf[i],*j, (char)(i*nfs+fs_idx_inner));
      }
    }
    //ex(vw_namespace((char)(6*nfs)))
//      (dis);
  
    //bigram features and trigram featuers are automatllcally generated based on feature template
    initialize_feature_template(srn, nfs, ec_buf);
  } 

  void get_valid_actions(v_array<uint32_t> & valid_action, uint32_t idx, uint32_t n, uint32_t stack_depth) {
    valid_action.erase();
    // SHIFT
    if(idx<=n)
      valid_action.push_back(1);

    // RIGHT
    if(stack_depth >=2)
      valid_action.push_back(2);

    // LEFT
    if(stack_depth >=1 && idx<=n)
      valid_action.push_back(3);
  }

  bool is_valid(uint32_t action, v_array<uint32_t> valid_actions){
    for(size_t i=0; i< valid_actions.size(); i++)
      if(valid_actions[i]==action)
        return true;
    return false;
  }

  bool has_dependency(uint32_t target, v_array<uint32_t> others, v_array<uint32_t> gold_heads) {
   for(uint32_t idx = 0; idx<others.size(); idx++)
     if(gold_heads[others[idx]] == target || gold_heads[target] == others[idx])
       return true;
   return false;
  }

  void get_gold_actions(uint32_t idx, uint32_t n, v_array<uint32_t> & stack, v_array<uint32_t> heads, v_array<uint32_t> valid_actions, v_array<uint32_t> gold_heads, v_array<uint32_t> & gold_actions){
    gold_actions.erase();
    cdep << "valid_action=[";for(size_t i=0; i<valid_actions.size(); i++){cdep << valid_actions[i] << " ";}cdep << "]";
    cdep << is_valid(2,valid_actions);
    // gold=SHIFT
    if(is_valid(1,valid_actions) && (stack.empty() || gold_heads[idx] == stack.last())){
      gold_actions.push_back(1);
      return;
    }

    // gold=LEFT
    if(is_valid(3,valid_actions) && gold_heads[stack.last()] == idx){
      gold_actions.push_back(3);
     return;
    }
    
    // gold contains SHIFT
    if(is_valid(1,valid_actions) && !has_dependency(idx, stack, gold_heads))
      gold_actions.push_back(1);

    // gold contains LEFT or RIGHT
    v_array<uint32_t> temp;
    for(uint32_t i=idx+1; i<n; i++)
      temp.push_back(i);

    if(!has_dependency(stack.last(), temp, gold_heads)){
      if(is_valid(2,valid_actions))
        gold_actions.push_back(2);
      if(is_valid(3,valid_actions) && !(stack.size()>=2 && gold_heads[stack.last()] == stack[stack.size()-2]))
        gold_actions.push_back(3);
    }
  }

  void structured_predict(searn& srn, vector<example*> ec) {
    cdep << "start structured predict"<<endl;
    task_data *data = srn.get_task_data<task_data>();
    uint32_t n = ec.size();
    uint32_t idx = 1;
    v_array<uint32_t> valid_actions;
    v_array<uint32_t> gold_heads; // gold labels
    v_array<uint32_t> gold_actions;
    v_array<uint32_t> stack; // stack for transition based parser
    v_array<uint32_t> heads; // output array
    
    

    cdep<<"create an ezexample for feature engineering" << endl;
    //example an_example();


    // initialization
    heads.push_back(0);
    gold_heads.push_back(0);
    for(size_t i=0; i<ec.size(); i++){ 
      gold_heads.push_back(MULTICLASS::get_example_label(ec[i])-1);
      heads.push_back(0);
    }
    stack.push_back(0);

    int count=0;
    cdep << "start decoding"<<endl;
    while(stack.size()>1 || idx <= n){
      srn.snapshot(count, 1, &count, sizeof(count), true);
      srn.snapshot(count, 2, &idx, sizeof(idx), true);
      srn.snapshot(count, 3, &stack, sizeof(stack[0])*stack.size(), true);
      srn.snapshot(count, 4, &heads, sizeof(heads[0])*n, true);
      cdep << "before transition: idx=" << idx << " n=" << n << " ";
      cdep << "stack = [";for(size_t i=0; i<stack.size(); i++){cdep << stack[i] << " ";} cdep << "]" << endl;
      cdep << "buffer = [";for(size_t i=idx; i<=ec.size(); i++){cdep << i << " ";} cdep << "]" << endl;
      cdep << "heads:[";for(size_t i=0; i<ec.size()+1; i++){cdep << heads[i] << " ";}cdep <<"]"<<endl;
      cdep << "extracting features"<<endl;
      VW::copy_example_data(false, data->ex->get_example(), ec[0]);  // copy but leave label alone!
      data->ex->clear_features();   
      extract_features(srn, idx, ec, stack, heads, *(data->ex));

      cdep << "setup valid and gold actions"<<endl;
      get_valid_actions(valid_actions, idx, n, stack.size());
      get_gold_actions(idx, n, stack, heads, valid_actions, gold_heads, gold_actions);
      cdep << "valid_action=[";for(size_t i=0; i<valid_actions.size(); i++){cdep << valid_actions[i] << " ";}cdep << "]";
      cdep << "gold_action=["; for(size_t i=0; i<gold_actions.size(); i++){ cdep << gold_actions[i] << " ";} cdep << "]"<<endl;     

      cdep << "make prediction"<<endl;
      uint32_t prediction = srn.predict(data->ex->get_example(), &gold_actions, &valid_actions);
      idx = transition_hybrid(srn, prediction, idx, stack, heads, gold_heads);
      cdep << "after taking action"<<prediction << " idx="<<idx <<" stack = [";for(size_t i=0; i<stack.size(); i++){cdep << stack[i] << " ";}cdep <<"]"<<endl;
      cdep << "stack = [";for(size_t i=0; i<stack.size(); i++){cdep << stack[i] << " ";} cdep << "]" << endl;
      cdep << "buffer = [";for(size_t i=idx; i<=ec.size(); i++){cdep << i << " ";} cdep << "]" << endl;
      cdep << "heads:[";for(size_t i=0; i<ec.size()+1; i++){cdep << heads[i] << " ";}cdep <<"]"<<endl;
      cdep << "gold_heads:[";for(size_t i=0; i<ec.size()+1; i++){cdep << gold_heads[i] << " ";}cdep <<"]"<<endl;
      cdep << endl;
      count++;
    }
    if (srn.output().good())
      for(size_t i=1; i<=n; i++){
        cdep <<heads[i] << " ";
        srn.output() << heads[i] << " ";
      }
    cdep << "end structured predict"<<endl;
    valid_actions.delete_v();
    gold_heads.delete_v();
    gold_actions.delete_v();
    stack.delete_v();
    heads.delete_v();
  }
}
