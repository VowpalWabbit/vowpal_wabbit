/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#include "example.h"
#include "unique_sort.h"

void unique_features(features& fs, int max)
{ if (fs.indicies.empty())
    return;
  size_t last_index = 0;
  size_t num_features = fs.indicies.size();
  if (max > 0)
    num_features = min(num_features, max);
  for (size_t i = 1; i < num_features; i++)
    if (fs.indicies[i] != fs.indicies[last_index])
      if (i != ++last_index)
	{
	  fs.values[last_index] = fs.values[i];
	  fs.indicies[last_index] = fs.indicies[i];
	  if (fs.space_names.size() > 0)
	    fs.space_names[last_index] = fs.space_names[i];
	}
  fs.values.end = fs.values.begin + (++last_index);
  fs.indicies.end = fs.indicies.begin + last_index;
  if (fs.space_names.size() > 0)
    fs.space_names.end = fs.space_names.begin + last_index;
}

void unique_sort_features(bool audit, uint64_t parse_mask, example* ae)
{ 
  for (unsigned char* b = ae->indices.begin; b != ae->indices.end; b++)
    { features& fs = ae->feature_space[*b];
            
      if (fs.indicies.size() == 0)
	continue;
      v_array<feature_slice> slice = v_init<feature_slice>();
      for (size_t i = 0; i < fs.indicies.size(); i++)
	{
	  feature_slice temp = {fs.values[i], fs.indicies[i] & parse_mask, pair<char*, char*>(nullptr, nullptr)};
	  if (fs.space_names.size() != 0)
	    temp.space_name = fs.space_names[i];
	  slice.push_back(temp);
	}
      qsort(slice.begin, slice.size(), sizeof(feature_slice), order_features);
      for (size_t i = 0; i < slice.size(); i++)
	{
	  fs.values[i] = slice[i].v;
	  fs.indicies[i] = slice[i].i;
	  if (fs.space_names.size() > 0)
	    fs.space_names[i] = slice[i].space_name;
	}
      slice.delete_v();
      unique_features(fs);
    }
  ae->sorted=true;
}
