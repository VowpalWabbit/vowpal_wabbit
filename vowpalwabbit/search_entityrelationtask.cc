/*
  CoPyright (c) by respective owners including Yahoo!, Microsoft, and
  individual contributors. All rights reserved.  Released under a BSD (revised)
  license as described in the file LICENSE.
*/
#include "search_entityrelationtask.h"
#include "multiclass.h"
#include "memory.h"
#include "example.h"
#include "gd.h"
#include "cost_sensitive.h"

#define R_NONE 10 // label for NONE relation
#define LABEL_SKIP 11 // label for SKIP

namespace EntityRelationTask { Search::search_task task = { "entity_relation", run, initialize, finish, NULL, NULL };  }


namespace EntityRelationTask {
  namespace CS = COST_SENSITIVE;

  void update_example_indicies(bool audit, example* ec, uint32_t mult_amount, uint32_t plus_amount);
  //enum SearchOrder { EntityFirst, Mix, Skip };

  struct task_data {
    float relation_none_cost;
    float entity_cost;
    float relation_cost;
    float skip_cost;
    bool constraints;
    bool allow_skip;
    v_array<uint32_t> y_allowed_entity;
    v_array<uint32_t> y_allowed_relation;
    int search_order;
    example* ldf_entity;
    example* ldf_relation;
    //SearchOrder search_order;
  };


  void initialize(Search::search& sch, size_t& num_actions, po::variables_map& vm) {
    task_data * my_task_data = new task_data();
    po::options_description sspan_opts("entity relation options");
    sspan_opts.add_options()
        ("relation_cost", po::value<float>(&(my_task_data->relation_cost))->default_value(1.0), "Relation Cost")
        ("entity_cost", po::value<float>(&(my_task_data->entity_cost))->default_value(1.0), "Entity Cost")
        ("constraints", "Use Constraints")
        ("relation_none_cost", po::value<float>(&(my_task_data->relation_none_cost))->default_value(0.5), "None Relation Cost")
        ("skip_cost", po::value<float>(&(my_task_data->skip_cost))->default_value(0.01f), "Skip Cost (only used when search_order = skip")
        ("search_order", po::value<int>(&(my_task_data->search_order))->default_value(0), "Search Order 0: EntityFirst 1: Mix 2: Skip 3: EntityFirst(LDF)" );
    sch.add_program_options(vm, sspan_opts);
    
    // setup entity and relation labels
    // Entity label 1:E_Other 2:E_Peop 3:E_Org 4:E_Loc
    // Relation label 5:R_Live_in 6:R_OrgBased_in 7:R_Located_in 8:R_Work_For 9:R_Kill 10:R_None
    my_task_data->constraints = vm.count("constraints") > 0;

    for(int i=1; i<5; i++)
      my_task_data->y_allowed_entity.push_back(i);

    for(int i=5; i<11; i++)
      my_task_data->y_allowed_relation.push_back(i);

    my_task_data->allow_skip = false;

    if(my_task_data->search_order != 3 && my_task_data->search_order != 4 ) {
      sch.set_options(0);
    } else {
      example* ldf_examples = alloc_examples(sizeof(CS::label), 10);
      CS::wclass default_wclass = { 0., 0, 0., 0. };
      for (size_t a=0; a<10; a++) {
        /*CS::wclass* default_wclass = new CS::wclass();
          default_wclass->x = 0.f;
          default_wclass->class_index = (uint32_t)a+1;
          default_wclass->partial_prediction = 0.f;
          default_wclass->wap_value = 0.f;
          CS::label* lab = (CS::label*)ldf_examples[a].ld;
          lab->costs.push_back(*default_wclass);*/
        CS::label* lab = (CS::label*)ldf_examples[a].ld;
        lab->costs.push_back(default_wclass);
      }
      my_task_data->ldf_entity = ldf_examples;
      my_task_data->ldf_relation = ldf_examples+4;
      sch.set_options(Search::IS_LDF);
    }
   
    sch.set_num_learners(2);
    if(my_task_data->search_order == 4)
      sch.set_num_learners(3);
    sch.set_task_data<task_data>(my_task_data);
  }

  void finish(Search::search& sch) {
    task_data * my_task_data = sch.get_task_data<task_data>();
    my_task_data->y_allowed_entity.delete_v();
    my_task_data->y_allowed_relation.delete_v();
    if(my_task_data->search_order == 3) {
      for (size_t a=0; a<10; a++)
        dealloc_example(CS::cs_label.delete_label, my_task_data->ldf_entity[a]);
      free(my_task_data->ldf_entity);
    }
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
  
  size_t predict_entity(Search::search&sch, example* ex, v_array<size_t>& predictions, ptag my_tag, bool isLdf=false){
    task_data* my_task_data = sch.get_task_data<task_data>();
    size_t prediction;
    if(my_task_data->allow_skip){
      v_array<uint32_t> star_labels;
      star_labels.push_back(MULTICLASS::get_example_label(ex));
      star_labels.push_back(LABEL_SKIP);
      my_task_data->y_allowed_entity.push_back(LABEL_SKIP);
      prediction = Search::predictor(sch, my_tag).set_input(*ex).set_oracle(star_labels).set_allowed(my_task_data->y_allowed_entity).set_learner_id(1).predict();
      //prediction = sch.predict(ex, &star_labels, &(my_task_data->y_allowed_entity),1);
      my_task_data->y_allowed_entity.pop();
    } else {
      if(isLdf) {
        for(size_t a=0; a<4; a++){
          VW::copy_example_data(false, &my_task_data->ldf_entity[a], ex);
          update_example_indicies(true, &my_task_data->ldf_entity[a], quadratic_constant, cubic_constant * (uint32_t)(a+1));
          CS::label* lab = (CS::label*)my_task_data->ldf_entity[a].ld;
          lab->costs[0].x = 0.f;
          lab->costs[0].class_index = (uint32_t)a;
          lab->costs[0].partial_prediction = 0.f;
          lab->costs[0].wap_value = 0.f;
        }
        //prediction  = sch.predictLDF(my_task_data->ldf_entity, 4, MULTICLASS::get_example_label(ex)-1, NULL, 1)+1;
        prediction = Search::predictor(sch, my_tag).set_input(my_task_data->ldf_entity, 4).set_oracle(MULTICLASS::get_example_label(ex)-1).set_learner_id(1).predict() + 1;
      } else {
        //prediction = sch.predict(ex, MULTICLASS::get_example_label(ex), &(my_task_data->y_allowed_entity),0);
        prediction = Search::predictor(sch, my_tag).set_input(*ex).set_oracle(MULTICLASS::get_example_label(ex)).set_allowed(my_task_data->y_allowed_entity).set_learner_id(0).predict();
      }
    }

    // record loss
    float loss = 0.0;
    if(prediction == LABEL_SKIP){
      loss = my_task_data->skip_cost;
    } else if(prediction !=  MULTICLASS::get_example_label(ex))
      loss= my_task_data->entity_cost;
    sch.loss(loss);
    return prediction;
  }
  size_t predict_relation(Search::search&sch, example* ex, v_array<size_t>& predictions, ptag my_tag, bool isLdf=false){
    char type; 
    int id1, id2;
    task_data* my_task_data = sch.get_task_data<task_data>();
    uint32_t* hist = new uint32_t[2];
    decode_tag(ex->tag, type, id1, id2);
    v_array<uint32_t> constrained_relation_labels;
    if(my_task_data->constraints && predictions[id1]!=0 &&predictions[id2]!=0){
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

    // HAL: I suspect we want to add_condition for other predictions, but i'm not sure what they should be...
    size_t prediction;
    if(my_task_data->allow_skip){
      v_array<uint32_t> star_labels;
      star_labels.push_back(MULTICLASS::get_example_label(ex));
      star_labels.push_back(LABEL_SKIP);
      constrained_relation_labels.push_back(LABEL_SKIP);
      //prediction = sch.predict(ex, &star_labels, &constrained_relation_labels,2);
      prediction = Search::predictor(sch, my_tag).set_input(*ex).set_oracle(star_labels).set_allowed(constrained_relation_labels).set_learner_id(2).add_condition(id1, 'a').add_condition(id2, 'b').predict();
      constrained_relation_labels.pop();
    } else {
      if(isLdf) {
        int correct_label = -1;
        for(size_t a=0; a<constrained_relation_labels.size(); a++){
          VW::copy_example_data(false, &my_task_data->ldf_relation[a], ex);
          update_example_indicies(true, &my_task_data->ldf_relation[a], quadratic_constant, cubic_constant * (uint32_t)(constrained_relation_labels[a]));
          CS::label* lab = (CS::label*)my_task_data->ldf_relation[a].ld;
          lab->costs[0].x = 0.f;
          lab->costs[0].class_index = (uint32_t)constrained_relation_labels[a];
          lab->costs[0].partial_prediction = 0.f;
          lab->costs[0].wap_value = 0.f;
          //cerr << constrained_relation_labels[a]<<" ";
          if(constrained_relation_labels[a] == MULTICLASS::get_example_label(ex)){
            correct_label = a;
          }
        }
        //cerr << correct_label<<endl;
        //cerr << MULTICLASS::get_example_label(ex)<<endl ;
        assert(correct_label>=0);
        //int pred_pos  = sch.predictLDF(my_task_data->ldf_relation, constrained_relation_labels.size(), correct_label , NULL, 2);
        size_t pred_pos = Search::predictor(sch, my_tag).set_input(my_task_data->ldf_relation, constrained_relation_labels.size()).set_oracle(correct_label).set_learner_id(2).predict();
        prediction = constrained_relation_labels[pred_pos];
        //cerr << prediction<<" " << correct_label << " " << MULTICLASS::get_example_label(ex)<<endl ;
      } else {
        //prediction = sch.predict(ex, MULTICLASS::get_example_label(ex), &constrained_relation_labels, 1);
        prediction = Search::predictor(sch, my_tag).set_input(*ex).set_oracle(MULTICLASS::get_example_label(ex)).set_allowed(constrained_relation_labels).set_learner_id(1).predict();
      }
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
    sch.loss(loss);
    delete hist;
    return prediction;
  }

  void entity_first_decoding(Search::search& sch, vector<example*> ec, v_array<size_t>& predictions, bool isLdf=false) {
    // ec.size = #entity + #entity*(#entity-1)/2
    size_t n_ent = (size_t)(sqrt(ec.size()*8+1)-1)/2;
    //
    // Do entity recognition first
    for (size_t i=0; i<ec.size(); i++) {
      if(i< n_ent)
        predictions[i] = predict_entity(sch, ec[i], predictions, (ptag)i, isLdf);
      else
        predictions[i] = predict_relation(sch, ec[i], predictions, (ptag)i, isLdf);
    }
  }

  void er_mixed_decoding(Search::search& sch, vector<example*> ec, v_array<size_t>& predictions) {
    // ec.size = #entity + #entity*(#entity-1)/2

    size_t n_ent = (size_t)(sqrt(ec.size()*8+1)-1)/2;
    for(size_t t=0; t<ec.size(); t++){
      // Do entity recognition first
      size_t count = 0;
      for (size_t i=0; i<n_ent; i++) {
                
        if(count ==t){
          predictions[i] = predict_entity(sch, ec[i], predictions, i);
          break;
        }
        count ++;
        for(size_t j=0; j<i; j++) {
          if(count ==t){
            size_t rel_index = n_ent + (2*n_ent-j-1)*j/2 + i-j-1;
            predictions[rel_index] = predict_relation(sch, ec[rel_index], predictions, rel_index);
            break;
          }
          count++;
        }
      }
    }

  }

  void er_allow_skip_decoding(Search::search& sch, vector<example*> ec, v_array<size_t>& predictions) {
    task_data* my_task_data = sch.get_task_data<task_data>();
    // ec.size = #entity + #entity*(#entity-1)/2
    size_t n_ent = (size_t)(sqrt(ec.size()*8+1)-1)/2;

    bool must_predict = false;
    size_t n_predicts = 0;
    size_t p_n_predicts = 0;
    my_task_data->allow_skip = true;
             
    // loop until all the entity and relation types are predicted
    for(size_t t=0; ; t++){
      size_t i = t % ec.size();
      //cerr << t <<" " <<n_predicts << " " << must_predict <<" " << ec.size()<< " " << p_n_predicts <<endl;
      if(n_predicts == ec.size())
        break;
      
      if(predictions[i] == 0){
        if(must_predict) {
          my_task_data->allow_skip = false;
        }
        size_t prediction = 0;
        if(i < n_ent) {// do entity recognition
          prediction = predict_entity(sch, ec[i], predictions, i);
        } else { // do relation recognition
          prediction = predict_relation(sch, ec[i], predictions, i);
        }

        if(prediction != LABEL_SKIP){
          predictions[i] = prediction;
          n_predicts++;
        }

        if(must_predict) {
          my_task_data->allow_skip = true;
          must_predict = false;
        }
      } 

      if(i == ec.size()-1) {
        if(n_predicts == p_n_predicts){
          must_predict = true;
        }
        p_n_predicts = n_predicts;
      }
    }
  }
  
  void er_important_first_decoding(Search::search& sch, vector<example*> ec, v_array<size_t>& predictions) {
    // ec.size = #entity + #entity*(#entity-1)/2
    size_t n_ent = (size_t)(sqrt(ec.size()*8+1)-1)/2;
    size_t position = -1;     
    for(size_t t=0; t < ec.size()*2; t++){
      //cerr << "t" << t<< "position"<<position ;
      if (t % 2 ==0){
        size_t num_remain = ec.size() - t/2;
        example* pos_ldf_examples = alloc_examples(sizeof(CS::label), num_remain);
        CS::wclass default_wclass = { 0., 0, 0., 0. };
        for (size_t a=0; a<num_remain; a++) {
          CS::label* lab = (CS::label*)pos_ldf_examples[a].ld;
          lab->costs.push_back(default_wclass);
        }
        int idx = 0;
        v_array<uint32_t> star_labels;
        v_array<uint32_t> position_labels;
        for(size_t a=0; a<ec.size(); a++){
          if(predictions[a] >0)
            continue;
          star_labels.push_back(idx);
          position_labels.push_back(a);
          VW::copy_example_data(false, &pos_ldf_examples[idx], ec[a]);

          // seperate relation & entity features
          if(a < n_ent)
            update_example_indicies(true, &pos_ldf_examples[idx], quadratic_constant, cubic_constant * 2);
          CS::label* lab = (CS::label*)pos_ldf_examples[idx].ld;
          lab->costs[0].x = 0.f;
          lab->costs[0].class_index = a;
          lab->costs[0].partial_prediction = 0.f;
          lab->costs[0].wap_value = 0.f;
          idx++;
        }
        //cerr << "idx" << idx << endl; 
        //int pred = sch.predictLDF(pos_ldf_examples, idx, &star_labels , NULL, 2);
        ptag my_tag = 0; // HAL: this is obviously wrong, but I don't know the right answer...
        size_t pred = Search::predictor(sch, my_tag).set_input(pos_ldf_examples, idx).set_oracle(star_labels).set_learner_id(2).predict();
        //cerr << "pred" << pred << endl;
        position  = position_labels[pred];
        //cerr << "position" << position << endl;
      } else {
        //    cerr<<"position" <<position<< " t" << t <<endl;
              
        assert(predictions[position] == 0);
        if(position < n_ent) {// do entity recognition
          predictions[position] = predict_entity(sch, ec[position], predictions, true);
        } else { // do relation recognition
          predictions[position] = predict_relation(sch, ec[position], predictions, true);
        }
      }
    }
  }

  void run(Search::search& sch, vector<example*>& ec) {
    task_data* my_task_data = sch.get_task_data<task_data>();
    
    v_array<size_t> predictions;
    for(size_t i=0; i<ec.size(); i++){
      predictions.push_back(0);
    }
        
    switch(my_task_data->search_order) {
      case 0:
        entity_first_decoding(sch, ec, predictions, false);
        break;
      case 1:
        er_mixed_decoding(sch, ec, predictions);
        break;
      case 2:
        er_allow_skip_decoding(sch, ec, predictions);
        break;
      case 3:
        entity_first_decoding(sch, ec, predictions, true); //LDF = true
        break;
      case 4:
        er_important_first_decoding(sch, ec, predictions); //LDF = true
        break;
      default:
        cerr << "search order " << my_task_data->search_order << "is undefined." << endl;
    }

    
    for(size_t i=0; i<ec.size(); i++){
      if (sch.output().good())
        sch.output() << predictions[i] << ' ';
    }
  }
  // this is totally bogus for the example -- you'd never actually do this!
  void update_example_indicies(bool audit, example* ec, uint32_t mult_amount, uint32_t plus_amount) {
    for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++)
      for (feature* f = ec->atomics[*i].begin; f != ec->atomics[*i].end; ++f)
        f->weight_index = (f->weight_index * mult_amount) + plus_amount;
    if (audit)
      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
        if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
          for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; ++f)
            f->weight_index = (f->weight_index * mult_amount) + plus_amount;
  }
}
