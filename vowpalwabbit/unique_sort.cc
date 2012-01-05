#include "unique_sort.h"

int order_features(const void* first, const void* second)
{
  return ((feature*)first)->weight_index - ((feature*)second)->weight_index;
}

int order_audit_features(const void* first, const void* second)
{
  return ((audit_data*)first)->weight_index - ((audit_data*)second)->weight_index;
}

void unique_features(v_array<feature> &features)
{
  if (features.empty())
    return;
  feature* last = features.begin;
  for (feature* current = features.begin+1; 
       current != features.end; current++)
    if (current->weight_index != last->weight_index) 
      *(++last) = *current;
  features.end = ++last;
}

void unique_audit_features(v_array<audit_data> &features)
{
  if (features.empty())
    return;
  audit_data* last = features.begin;
  for (audit_data* current = features.begin+1; 
       current != features.end; current++)
    if (current->weight_index != last->weight_index) 
      *(++last) = *current;
  features.end = ++last;
}

void unique_sort_features(example* ae)
{
  ae->sorted=true;
  bool audit = global.audit;
  for (size_t* b = ae->indices.begin; b != ae->indices.end; b++)
    {
      qsort(ae->atomics[*b].begin, ae->atomics[*b].index(), sizeof(feature), 
	    order_features);
      unique_features(ae->atomics[*b]);
      
      if (audit)
	{
	  qsort(ae->audit_features[*b].begin, ae->audit_features[*b].index(), sizeof(audit_data), 
		order_audit_features);
	  unique_audit_features(ae->audit_features[*b]);
	}
    }
}

