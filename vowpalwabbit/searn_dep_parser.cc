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
  };
namespace DepParserTask {
  using namespace Searn;
  uint32_t max_label = 0;

  void initialize(searn& srn, size_t& num_actions, po::variables_map& vm) {
    task_data * my_task_data = new task_data();
    my_task_data->num_actions = num_actions;
    srn.set_options( 0 );
    srn.set_num_learners(1);
    srn.set_task_data<task_data>(my_task_data);
  }

  void finish(searn& srn) { }    // if we had task data, we'd want to free it here

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



  void extract_features(searn& srn, uint32_t i,  vector<example*> &ec, v_array<uint32_t> & stack, v_array<uint32_t> & heads, ezexample &ex){
    // be careful: indices in ec starts from 0, but i is starts from 1
    size_t n = ec.size();
    // get examples in stake
    map<string, example*> ec_buf;
    size_t nfs = ec[0]->indices.size();

    // feature based on examples in stack s?=the top ? element in stack
    // todo: add features based on leftmost and rightmost child: rc?: the ? rightmost child, lc?: the ? leftmost child
    ec_buf["s3"]=ec_buf["s2"]=ec_buf["s1"]=NULL;
    if(stack.size()>=3 && stack[stack.size()-3]!=0)
      ec_buf["s3"] = ec[stack[stack.size()-3]-1];
    if(stack.size()>=2 && stack[stack.size()-2]!=0)
      ec_buf["s2"] = ec[stack[stack.size()-2]-1];
    if(stack.size()>=1 && stack.last()!=0){
      ec_buf["s1"] = ec[stack.last()-1];
    }

    // features based on examples in buffer
    // todo: add features based on leftmost child in buffer
    ec_buf["b3"]=ec_buf["b2"]=ec_buf["b1"]=NULL;
    if(i+3 <= n)
      ec_buf["b3"] = ec[i+3-1];
    if(i+2 <= n)
      ec_buf["b2"] = ec[i+2-1];
    if(i+1 <= n)
      ec_buf["b1"] = ec[i+1-1];

    size_t stack_depth = stack.size();
    size_t dis = 0;
    if(!stack.empty())
      dis = min(5, i - stack.last());

    cdep << "start generate feature";
    // unigram features
    int fs_idx = 1;
    for(map<string, example*>::iterator p = ec_buf.begin(); p!=ec_buf.end(); p++){
      fs_idx++;
      cdep << fs_idx;
      if(p->second == NULL)
        continue;
      int fs_idx_inner = 0;
      // add unigram of all feature_template
      // consider mixing/not mixing all feature spaces
      for (unsigned char* i = ec[0]->indices.begin; i != ec[0]->indices.end; i++){
         cdep << "add ";
         ex(*(p->second),*i, fs_idx_inner)(*(p->second),*i, fs_idx*nfs+fs_idx_inner);
         cdep << "done"<<endl;
         fs_idx_inner++;
      }
    }
  }

  void get_valid_actions(v_array<uint32_t> & valid_action, uint32_t i, uint32_t n, uint32_t stack_depth) {
    valid_action.erase();
    // SHIFT
    if(i<=n)
      valid_action.push_back(1);

    // RIGHT
    if(stack_depth >=2)
      valid_action.push_back(2);

    // LEFT
    if(stack_depth >=1)
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
    uint32_t n = ec.size();
    uint32_t idx = 1;
    v_array<uint32_t> valid_actions;
    v_array<uint32_t> gold_heads; // gold labels
    v_array<uint32_t> gold_actions;
    v_array<uint32_t> stack; // stack for transition based parser
    v_array<uint32_t> heads; // output array

    cdep<<"create an ezexample for feature engineering" << endl;
    example* an_example = alloc_examples(sizeof(COST_SENSITIVE::label),1);
    ezexample ex(srn.all, an_example);

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
      /*
      srn.snapshot(count, 1, &count, sizeof(count), true);
      srn.snapshot(count, 2, &idx, sizeof(idx), true);
      srn.snapshot(count, 3, &stack, sizeof(stack[0])*stack.size(), true);
      srn.snapshot(count, 4, &heads, sizeof(heads[0])*n, true);
      */
      cdep << "before transition: idx=" << idx << " n=" << n << " ";
      cdep << "stack = [";for(size_t i=0; i<stack.size(); i++){cdep << stack[i] << " ";} cdep << "]" << endl;
      cdep << "buffer = [";for(size_t i=idx; i<=ec.size(); i++){cdep << i << " ";} cdep << "]" << endl;
      cdep << "heads:[";for(size_t i=0; i<ec.size()+1; i++){cdep << heads[i] << " ";}cdep <<"]"<<endl;

      cdep << "extracting features"<<endl;
      ex.clear_features();   
      extract_features(srn, idx, ec, stack, heads, ex);

      cdep << "setup valid and gold actions"<<endl;
      get_valid_actions(valid_actions, idx, n, stack.size());
      get_gold_actions(idx, n, stack, heads, valid_actions, gold_heads, gold_actions);
      cdep << "valid_action=[";for(size_t i=0; i<valid_actions.size(); i++){cdep << valid_actions[i] << " ";}cdep << "]";
      cdep << "gold_action=["; for(size_t i=0; i<gold_actions.size(); i++){ cdep << gold_actions[i] << " ";} cdep << "]"<<endl;     

      cdep << "make prediction"<<endl;
      uint32_t prediction = srn.predict(ex.get_example(), &gold_actions, &valid_actions);
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
    dealloc_example(NULL, *an_example);
  }
}
