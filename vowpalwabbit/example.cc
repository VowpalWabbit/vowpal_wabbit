/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <stdint.h>
#include "parse_primitives.h"
#include "v_array.h"
#include "example.h"
#include "simple_label.h"  
#include "gd.h"  
#include "global_data.h"  
  
void vec_store(vw& all, void* p, float fx, uint32_t fi) {  
 feature *f = (feature*)calloc(1,sizeof(feature));  
 f->x = fx;  
 f->weight_index = fi;  
 (*(v_array<feature>*) p).push_back(*f);  
 free(f);  
}  
  
int compare_feature(const void* p1, const void* p2) {  
  feature* f1 = (feature*) p1;  
  feature* f2 = (feature*) p2;  
  return (f1->weight_index - f2->weight_index);  
}  
  
float collision_cleanup(v_array<feature>& feature_map) {  
    
 int pos = 0;  
 float sum_sq = 0.;  
  
 for(uint32_t i = 1;i < feature_map.size();i++) {  
    if(feature_map[i].weight_index == feature_map[pos].weight_index)   
      feature_map[pos].x += feature_map[i].x;  
    else {  
      sum_sq += feature_map[pos].x*feature_map[pos].x;  
      feature_map[++pos] = feature_map[i];            
    }  
  }  
  sum_sq += feature_map[pos].x*feature_map[pos].x;  
  feature_map.end = &(feature_map[pos]);    
  feature_map.end++;  
  return sum_sq;  
}  

flat_example flatten_example(vw& all, example *ec) {  
  flat_example* fec = (flat_example*) calloc(1,sizeof(flat_example));  
  fec->ld = calloc(1,sizeof(label_data));  
  fec->final_prediction = ec->final_prediction;  
  fec->tag = ec->tag;  
  fec->example_counter = ec->example_counter;  
  
  fec->num_features = ec->num_features;  
  fec->ft_offset = ec->ft_offset;  
  fec->partial_prediction = ec->partial_prediction;  
  fec->topic_predictions = ec->topic_predictions;  
  fec->loss = ec->loss;  
  fec->eta_round = ec->eta_round;  
  fec->eta_global = ec->eta_global;  
  fec->global_weight = ec->global_weight;  
  fec->example_t = ec->example_t;  
  fec->total_sum_feat_sq = ec->total_sum_feat_sq;  
  fec->revert_weight = ec->revert_weight;  
  fec->end_pass = ec->end_pass;  
  fec->sorted = 1;  
  fec->in_use = ec->in_use;  
  fec->done = ec->done;  
    
  GD::foreach_feature<vec_store>(all, ec, &fec->feature_map);  
  qsort(fec->feature_map.begin, fec->feature_map.size(), sizeof(feature), compare_feature);  
  fec->total_sum_feat_sq = collision_cleanup(fec->feature_map);  
    
  return *fec;  
}  

namespace VW {

flat_example_ex* flatten_example_ex(vw& all, example *ec) 
{  
    if (command_example(&all, ec))
	{
		return 0;
	}

	flat_example_ex* fec = (flat_example_ex*) calloc(1,sizeof(flat_example_ex));  
	fec->ld = calloc(1,sizeof(label_data));  
	fec->final_prediction = ec->final_prediction;  

	fec->tag_len = ec->tag.size();
	if (fec->tag_len >0)
	{
		fec->tag = (char*)calloc(ec->tag.size(), sizeof(char*));
		memcpy(fec->tag, ec->tag.begin, ec->tag.size());
	}

	fec->example_counter = ec->example_counter;  
	fec->num_features = ec->num_features;  
	fec->ft_offset = ec->ft_offset;  
	fec->partial_prediction = ec->partial_prediction; 

	fec->topic_predictions_len = ec->topic_predictions.size();
	if (fec->topic_predictions_len > 0)
	{
		fec->topic_predictions = (float*)calloc(ec->topic_predictions.size(), sizeof(float));
		memcpy(fec->topic_predictions, ec->topic_predictions.begin, ec->topic_predictions.size()*sizeof(float));
	}

	fec->loss = ec->loss;  
	fec->eta_round = ec->eta_round;  
	fec->eta_global = ec->eta_global;  
	fec->global_weight = ec->global_weight;  
	fec->example_t = ec->example_t;  
	fec->total_sum_feat_sq = ec->total_sum_feat_sq;  
	fec->revert_weight = ec->revert_weight;  
	fec->end_pass = ec->end_pass;  
	fec->sorted = 1;  
	fec->in_use = ec->in_use;  
	fec->done = ec->done;  
    
	v_array<feature> feature_map; //map to store sparse feature vectors  
	GD::foreach_feature<vec_store>(all, ec, &feature_map); 
	qsort(feature_map.begin, feature_map.size(), sizeof(feature), compare_feature);  
	fec->total_sum_feat_sq = collision_cleanup(feature_map);  
	fec->sum_feat_sq_len = 0;
    
	fec->feature_map_len = feature_map.size();
	if (fec->feature_map_len > 0)
	{
		fec->feature_map = (feature*)calloc(feature_map.size(), sizeof(feature));
		memcpy(fec->feature_map, feature_map.begin, feature_map.size()*sizeof(feature));
	}
	feature_map.delete_v();

	return fec;  
}

void free_flatten_example_ex(flat_example_ex* fec) 
{  
	if (!fec)
		return;

	if (fec->ld)
		free(fec->ld);
	if (fec->tag)
		free(fec->tag);
	if (fec->topic_predictions)
		free(fec->topic_predictions);
	if (fec->feature_map)
		free(fec->feature_map);

	free(fec);
}

}

example *alloc_example(size_t label_size)
{
  example* ec = (example*)calloc(1, sizeof(example));
  if (ec == NULL) return NULL;
  ec->ld = calloc(1, label_size);
  ec->in_use = true;
  ec->ft_offset = 0;
  //  std::cerr << "  alloc_example.indices.begin=" << ec->indices.begin << " end=" << ec->indices.end << " // ld = " << ec->ld << "\t|| me = " << ec << std::endl;
  return ec;
}

void dealloc_example(void(*delete_label)(void*), example&ec)
{
  // std::cerr << "dealloc_example.indices.begin=" << ec.indices.begin << " end=" << ec.indices.end << " // ld = " << ec.ld << "\t|| me = " << &ec << std::endl;
  if (delete_label) {
    delete_label(ec.ld);
  }
  ec.tag.delete_v();
      
  ec.topic_predictions.delete_v();

  free(ec.ld);
  for (size_t j = 0; j < 256; j++)
    {
      ec.atomics[j].delete_v();

      if (ec.audit_features[j].begin != ec.audit_features[j].end_array)
        {
          for (audit_data* temp = ec.audit_features[j].begin; 
               temp != ec.audit_features[j].end; temp++)
            if (temp->alloced) {
              free(temp->space);
              free(temp->feature);
              temp->alloced = false;
            }
	  ec.audit_features[j].delete_v();
        }
    }
  ec.indices.delete_v();
}

feature copy_feature(feature src) {
  feature f = { src.x, src.weight_index };
  return f;
}

namespace VW {
  void copy_example_data(example* &dst, example* src, size_t label_size, void(*copy_label)(void*&,void*))
{
  if (!src->ld) {
    if (dst->ld) free(dst->ld);  // TODO: this should be a delete_label, really
    dst->ld = NULL;
  } else {
    if ((label_size == 0) && (copy_label == NULL)) {
      if (dst->ld) free(dst->ld);  // TODO: this should be a delete_label, really
      dst->ld = NULL;
    } else if (copy_label) {
      copy_label(dst->ld, src->ld);
    } else {
      memcpy(dst->ld, src->ld, label_size);
    }
  }

  dst->final_prediction = src->final_prediction;

  copy_array(dst->tag, src->tag);
  dst->example_counter = src->example_counter;

  copy_array(dst->indices, src->indices);
  for (size_t i=0; i<256; i++)
    copy_array(dst->atomics[i], src->atomics[i], copy_feature);
  dst->ft_offset = src->ft_offset;

  dst->num_features = src->num_features;
  dst->partial_prediction = src->partial_prediction;
  copy_array(dst->topic_predictions, src->topic_predictions);
  dst->loss = src->loss;
  dst->eta_round = src->eta_round;
  dst->eta_global = src->eta_global;
  dst->global_weight = src->global_weight;
  dst->example_t = src->example_t;
  for (size_t i=0; i<256; i++)
    dst->sum_feat_sq[i] = src->sum_feat_sq[i];
  dst->total_sum_feat_sq = src->total_sum_feat_sq;
  dst->revert_weight = src->revert_weight;
  dst->sorted = src->sorted;
  dst->in_use = src->in_use;
  dst->done = src->done;
}
}

void update_example_indicies(bool audit, example* ec, uint32_t amount) { 
  ec->ft_offset += amount; }

#include "global_data.h"
void save_predictor(vw& all, string reg_name, size_t current_pass);

bool command_example(void* a, example* ec) 
{
  vw* all=(vw*)a;
  if(ec->end_pass) // the end-of-pass example
    return true;

  if (ec->indices.size() > 1) // one nonconstant feature.
    return false;

  if (ec->tag.size() >= 4 && !strncmp((const char*) ec->tag.begin, "save", 4) && all->current_command != ec->example_counter)
    {//save state
      string final_regressor_name = all->final_regressor_name;
      
      if ((ec->tag).size() >= 6 && (ec->tag)[4] == '_')
	final_regressor_name = string(ec->tag.begin+5, (ec->tag).size()-5);
      
      if (!all->quiet)
	cerr << "saving regressor to " << final_regressor_name << endl;
      save_predictor(*all, final_regressor_name, 0);
      
      all->current_command = ec->example_counter;
      
      return true;
    }
  return false;
}
