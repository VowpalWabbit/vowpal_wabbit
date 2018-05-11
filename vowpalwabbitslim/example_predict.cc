#include "example_predict.h"
#include "hash.h"

#include <string.h>
#include <algorithm>

safe_example_predict::safe_example_predict()
{ indices = v_init<namespace_index>();
  ft_offset = 0;
  // feature_space is initialized through constructors
}

safe_example_predict::~safe_example_predict()
{
  indices.delete_v();
  for (size_t i=0;i<256;i++)
    feature_space[i].delete_v();
}

void safe_example_predict::clear()
{
	for (auto ns : indices)
		feature_space[ns].clear();
	indices.clear();
}
