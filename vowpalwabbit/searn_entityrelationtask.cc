/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "searn_entityrelationtask.h"
#include "multiclass.h"
#include "memory.h"
#include "example.h"
#include "gd.h"

#define R_NONE 10 // label for NONE relation
#define LABEL_SKIP 11 // label for NONE relation

namespace EntityRelationTask     {  Searn::searn_task task = { "entity_relation",     initialize, finish, structured_predict };  }


namespace EntityRelationTask {
  using namespace Searn;

  //enum SearchOrder { EntityFirst, Mix, Skip };

  struct task_data {
    float relation_none_cost;
    float entity_cost;
    float relation_cost;
    float skip_cost;
    bool history_features;
    bool constraints;
    v_array<uint32_t> y_allowed_entity;
    v_array<uint32_t> y_allowed_relation;
    int search_order;
    //SearchOrder search_order;
  };


  void initialize(searn& srn, size_t& num_actions, po::variables_map& vm) {
    task_data * my_task_data = new task_data();
    po::options_description sspan_opts("entity relation options");
    sspan_opts.add_options()
      ("relation_cost", po::value<float>(&(my_task_data->relation_cost))->default_value(1.0), "Relation Cost")
      ("entity_cost", po::value<float>(&(my_task_data->entity_cost))->default_value(1.0), "Entity Cost")
      ("history_features", "Use History Features")
      ("constraints", "Use Constraints")
      ("relation_none_cost", po::value<float>(&(my_task_data->relation_none_cost))->default_value(0.5), "None Relation Cost")
      ("skip_cost", po::value<float>(&(my_task_data->skip_cost))->default_value(0.01f), "Skip Cost (only used when search_order = skip")
      ("search_order", po::value<int>(&(my_task_data->search_order))->default_value(0), "Search Order 0: EntityFirst 1: Mix 2: Skip" );
    vm = add_options(*srn.all, sspan_opts);
	  
	// setup entity and relation labels
	// Entity label 1:E_Other 2:E_Peop 3:E_Org 4:E_Loc
	// Relation label 5:R_Live_in 6:R_OrgBased_in 7:R_Located_in 8:R_Work_For 9:R_Kill 10:R_None
    if (vm.count("history_features"))
      my_task_data->history_features = true;
    else
      my_task_data->history_features = false;      

	if (vm.count("constraints"))
      my_task_data->constraints = true;
    else
      my_task_data->constraints = false;

	for(int i=1; i<5; i++){
	  my_task_data->y_allowed_entity.push_back(i);
	}
	for(int i=5; i<11; i++){
	  my_task_data->y_allowed_relation.push_back(i);
	}

    srn.set_task_data<task_data>(my_task_data);
    srn.set_options(0);
  }

  void finish(searn& srn) {
    task_data * my_task_data = srn.get_task_data<task_data>();
    my_task_data->y_allowed_entity.delete_v();
    my_task_data->y_allowed_relation.delete_v();
    delete my_task_data;
  }    // if we had task data, we'd want to free it here

  bool check_constraints(int ent1_id, int ent2_id, int rel_id){
	  int valid_ent1_id [] = {2,3,4,2,2}; // encode the valid entity-relation combinations 
	  int valid_ent2_id [] = {4,4,4,3,2};
	  if(rel_id - 5 == 5)
		  return true;
	  if(valid_ent1_id[rel_id-5] == ent1_id && valid_ent2_id[rel_id-5] == ent2_id)
		  return true;
	  return false;
  }

  void decode_tag(v_array<char> tag, char& type, int& id1, int& id2){
      string s1;
      string s2;
      type = tag[0];
      uint32_t idx = 2;
      while(idx < tag.size() && tag[idx] != '_' && tag[idx] != '\0'){
    	  s1.push_back(tag[idx]);		  
    	  idx++;
      }
      id1 = atoi(s1.c_str());
      idx++;
      if(type == 'R'){
    	while(idx < tag.size() && tag[idx] != '_' && tag[idx] != '\0'){
    	  s2.push_back(tag[idx]);		  
    	  idx++;
    	}
    	id2 = atoi(s2.c_str());
      }
  }
  
  size_t predict_entity(searn&srn, example* ex, v_array<size_t>& predictions){
    task_data* my_task_data = srn.get_task_data<task_data>();
    size_t prediction = srn.predict(ex, MULTICLASS::get_example_label(ex), &(my_task_data->y_allowed_entity));

    // record loss
    float loss = 0.0;
    if(prediction == LABEL_SKIP){
      loss = my_task_data->skip_cost;
    } else if(prediction !=  MULTICLASS::get_example_label(ex))
      loss= my_task_data->entity_cost;
    srn.loss(loss);
    return prediction;
  }
  size_t predict_relation(searn&srn, example* ex, history_info* hinfo, v_array<size_t>& predictions){
    char type; 
    int id1, id2;
    task_data* my_task_data = srn.get_task_data<task_data>();
    history hist = new uint32_t[2];
    decode_tag(ex->tag, type, id1, id2);
    v_array<uint32_t> constrained_relation_labels;
    if((my_task_data->constraints || my_task_data->history_features) && predictions[id1]!=0 &&predictions[id2]!=0){
      hist[0] = (uint32_t)predictions[id1];
      hist[1] = (uint32_t)predictions[id2];
    } else {
      hist[0] = 0;
    }
    for(size_t j=0; j< my_task_data->y_allowed_relation.size(); j++){
      if(!my_task_data->constraints || hist[0] == 0  || check_constraints(hist[0], hist[1], my_task_data->y_allowed_relation[j])){
        constrained_relation_labels.push_back(my_task_data->y_allowed_relation[j]);
      }
    }

    // add history feature to relation example
    if(my_task_data->history_features && hist[0]!= 0) {
      add_history_to_example(*(srn.all), *hinfo , ex, hist, 0);
    }	

    size_t prediction = srn.predict(ex, MULTICLASS::get_example_label(ex),&constrained_relation_labels);

    if(my_task_data->history_features && hist[0] != 0) {
      remove_history_from_example(*(srn.all), *hinfo , ex);
    }

    float loss = 0.0;
    if(prediction == LABEL_SKIP){
      loss = my_task_data->skip_cost;
    } else if(prediction !=  MULTICLASS::get_example_label(ex)) {
      if(MULTICLASS::get_example_label(ex) == R_NONE){
        loss = my_task_data->relation_none_cost;
      } else {
        loss= my_task_data->relation_cost;
      }
    }
    srn.loss(loss);
    delete hist;
    return prediction;
  }

  void entity_first_decoding(searn& srn, vector<example*> ec, history_info* hinfo, v_array<size_t>& predictions) {
    // ec.size = #entity + #entity*(#entity-1)/2
    size_t n_ent = (size_t)(sqrt(ec.size()*8+1)-1)/2;
    // Do entity recognition first
    for (size_t i=0; i<n_ent; i++) {
      predictions[i] = predict_entity(srn, ec[i], predictions);
    }

    // Now do relation recognition task;
    for (size_t i=n_ent; i<ec.size(); i++) {
      predictions[i] = predict_relation(srn, ec[i], hinfo, predictions);
    }
  }

  void er_mixed_decoding(searn& srn, vector<example*> ec, history_info* hinfo, v_array<size_t>& predictions) {
    // ec.size = #entity + #entity*(#entity-1)/2
    size_t n_ent = (size_t)(sqrt(ec.size()*8+1)-1)/2;
    // Do entity recognition first
    for (size_t i=0; i<n_ent; i++) {
      predictions[i] = predict_entity(srn, ec[i], predictions);
      // When the entity types of the two entities invovled in a relation are resolved,
      // we predict the relation.
      for(size_t j=0; j<i; j++) {
        size_t rel_index = n_ent + (2*n_ent-j-1)*j/2 + i-j-1;
        predictions[rel_index] = predict_relation(srn, ec[rel_index], hinfo, predictions);
      }
    }

    // Now do relation recognition task;
    for (size_t i=n_ent; i<ec.size(); i++) { //save state for optimization
      example* ex = ec[i];	
      predictions[i] = predict_relation(srn, ex, hinfo, predictions);
    }
  }

  void er_allow_skip_decoding(searn& srn, vector<example*> ec, history_info* hinfo, v_array<size_t>& predictions) {
    task_data* my_task_data = srn.get_task_data<task_data>();
    // ec.size = #entity + #entity*(#entity-1)/2
    size_t n_ent = (size_t)(sqrt(ec.size()*8+1)-1)/2;
    bool must_predict = false;
    size_t n_predicts = 0;
    size_t p_n_predicts = 0;
    my_task_data->y_allowed_relation.push_back(LABEL_SKIP);
    my_task_data->y_allowed_entity.push_back(LABEL_SKIP);

    // loop until all the entity and relation types are predicted
    while(1){
      //cerr <<n_predicts << " " << ec.size() << endl;
      if(n_predicts == ec.size())
        break;
      for (size_t i=0; i<ec.size(); i++) {
        if(predictions[i] != 0)
          continue;
        if(must_predict) {
          my_task_data->y_allowed_relation.pop();
	  my_task_data->y_allowed_entity.pop();
	}
        size_t prediction = 0;
        if(i < n_ent) {// do entity recognition
          prediction = predict_entity(srn, ec[i], predictions);
        } else { // do relation recognition
          prediction = predict_relation(srn, ec[i], hinfo, predictions);
        }

        if(prediction != LABEL_SKIP){
          predictions[i] = prediction;
	  n_predicts++;
        }

        if(must_predict) {
          my_task_data->y_allowed_relation.push_back(LABEL_SKIP);
	  my_task_data->y_allowed_entity.push_back(LABEL_SKIP);
          must_predict = false;
	}
      }
      if(n_predicts == p_n_predicts){
        must_predict = true;
      }
      p_n_predicts = n_predicts;
    }
    my_task_data->y_allowed_relation.pop();
    my_task_data->y_allowed_entity.pop();
  }
  void structured_predict(searn& srn, vector<example*> ec) {
    task_data* my_task_data = srn.get_task_data<task_data>();
    
    v_array<size_t> predictions;
    for(size_t i=0; i<ec.size(); i++){
      predictions.push_back(0);
    }
        
    // setup history 
    history_info* hinfo = new history_info();
    default_info(hinfo);
    hinfo->length = 2;
    
    switch(my_task_data->search_order) {
      case 0:
        entity_first_decoding(srn, ec, hinfo, predictions);
        break;
      case 1:
        er_mixed_decoding(srn, ec, hinfo, predictions);
        break;
      case 2:
        er_allow_skip_decoding(srn, ec, hinfo, predictions);
        break;
      default:
        cerr << "search order " << my_task_data->search_order << "is undefined." << endl;
    }

    
    for(size_t i=0; i<ec.size(); i++){
      if (srn.output().good())
        srn.output() << predictions[i] << ' ';
    }
    delete hinfo;
}
}

