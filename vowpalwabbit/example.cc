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
#include "memory.h"
  
int compare_feature(const void* p1, const void* p2) {  
  feature* f1 = (feature*) p1;  
  feature* f2 = (feature*) p2;  
  if(f1->weight_index < f2->weight_index) return -1;
  else if(f1->weight_index > f2->weight_index) return 1;
  else return 0;
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
  
audit_data copy_audit_data(audit_data &src) {
  audit_data dst;
  dst.space = (char*)calloc_or_die(strlen(src.space)+1, sizeof(char));
  strcpy(dst.space, src.space);
  dst.feature = (char*)calloc_or_die(strlen(src.feature)+1, sizeof(char));
  strcpy(dst.feature, src.feature);
  dst.weight_index = src.weight_index;
  dst.x = src.x;
  dst.alloced = src.alloced;
  return dst;
}

namespace VW {
void copy_example_label(example* dst, example* src, size_t label_size, void(*copy_label)(void*&,void*)) {
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
      //dst->ld = (void*)malloc(label_size);
      memcpy(dst->ld, src->ld, label_size);
    }
  }
}

void copy_example_data(bool audit, example* dst, example* src)
{
  //std::cerr << "copy_example_data dst = " << dst << std::endl;
  copy_array(dst->tag, src->tag);
  dst->example_counter = src->example_counter;

  copy_array(dst->indices, src->indices);
  for (size_t i=0; i<256; i++)
    copy_array(dst->atomics[i], src->atomics[i]);
  dst->ft_offset = src->ft_offset;

  if (audit)
    for (size_t i=0; i<256; i++)
      copy_array(dst->audit_features[i], src->audit_features[i], copy_audit_data);
  
  dst->num_features = src->num_features;
  dst->partial_prediction = src->partial_prediction;
  copy_array(dst->topic_predictions, src->topic_predictions);
  dst->loss = src->loss;
  dst->example_t = src->example_t;
  memcpy(dst->sum_feat_sq, src->sum_feat_sq, 256 * sizeof(float));
  dst->total_sum_feat_sq = src->total_sum_feat_sq;
  dst->revert_weight = src->revert_weight;
  dst->test_only = src->test_only;
  dst->end_pass = src->end_pass;
  dst->sorted = src->sorted;
  dst->in_use = src->in_use;}

void copy_example_data(bool audit, example* dst, example* src, size_t label_size, void(*copy_label)(void*&,void*)) {
  copy_example_data(audit, dst, src);
  copy_example_label(dst, src, label_size, copy_label);
}
}

struct features_and_source 
{
  v_array<feature> feature_map; //map to store sparse feature vectors  
  uint32_t stride_shift;
  uint32_t mask;
  weight* base;
};

void vec_store(vw& all, features_and_source& p, float fx, uint32_t fi) {  
  uint32_t mask = (uint32_t)all.reg.weight_mask >> all.reg.stride_shift;
  feature f = {fx, (fi >> all.reg.stride_shift) & mask};
  p.feature_map.push_back(f);
}  

namespace VW {
  feature* get_features(vw& all, example* ec, size_t& feature_map_len, float& total_sum_sq)
{
	features_and_source fs;
	fs.stride_shift = all.reg.stride_shift;
	fs.mask = (uint32_t)all.reg.weight_mask >> all.reg.stride_shift;
	fs.base = all.reg.weight_vector;
	GD::foreach_feature<features_and_source, vec_store>(all, *ec, fs); 	
	qsort(fs.feature_map.begin, fs.feature_map.size(), sizeof(feature), compare_feature);  
	total_sum_sq = collision_cleanup(fs.feature_map);
	feature_map_len = fs.feature_map.size();
	return fs.feature_map.begin;
}

void return_features(feature* f)
{
	if (f != NULL)
		free(f);
}
}

flat_example* flatten_example(vw& all, example *ec) 
{
	flat_example* fec = (flat_example*) calloc_or_die(1,sizeof(flat_example));  
	fec->ld = (label_data*)calloc_or_die(1, sizeof(label_data));
	memcpy(fec->ld, ec->ld, sizeof(label_data));

	fec->tag_len = ec->tag.size();
	if (fec->tag_len >0)
	  {
	    fec->tag = (char*)calloc_or_die(fec->tag_len+1, sizeof(char));
	    memcpy(fec->tag,ec->tag.begin, fec->tag_len);
	  }

	fec->example_counter = ec->example_counter;  
	fec->ft_offset = ec->ft_offset;  
	fec->num_features = ec->num_features;  
        
	fec->feature_map = VW::get_features(all, ec, fec->feature_map_len, fec->total_sum_feat_sq);

	return fec;  
}

void free_flatten_example(flat_example* fec) 
{  //note: The label memory should be freed by by freeing the original example.
  if (fec)
    {
      if (fec->feature_map_len > 0)
	free(fec->feature_map);
      if (fec->tag_len > 0)
	free(fec->tag);
      free(fec->ld);
      free(fec);
    }
}

example *alloc_examples(size_t label_size, size_t count=1)
{
  example* ec = (example*)calloc_or_die(count, sizeof(example));
  if (ec == NULL) return NULL;
  for (size_t i=0; i<count; i++) {
    ec[i].ld = calloc_or_die(1, label_size);
    if (ec[i].ld == NULL) {
      for (size_t j=0; j<i; j++) free(ec[j].ld);
      free(ec);
      return NULL;
    }
    ec[i].in_use = true;
    ec[i].ft_offset = 0;
    //  std::cerr << "  alloc_example.indices.begin=" << ec->indices.begin << " end=" << ec->indices.end << " // ld = " << ec->ld << "\t|| me = " << ec << std::endl;
  }
  return ec;
}

void dealloc_example(void(*delete_label)(void*), example&ec)
{
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

