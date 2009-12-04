#include <float.h>

#include "simple_label.h"
#include "cache.h"

char* bufread_simple_label(label_data* ld, char* c)
{
  ld->label = *(double *)c;
  c += sizeof(ld->label);
  ld->weight = *(float *)c;
  c += sizeof(ld->weight);
  return c;
}

size_t read_cached_simple_label(void* v, io_buf& cache)
{
  label_data* ld = (label_data*) v;
  char *c;
  size_t total = sizeof(ld->label)+sizeof(ld->weight);
  if (buf_read(cache, c, total) < total) 
    return 0;
  c = bufread_simple_label(ld,c);

  return total;
}

char* bufcache_simple_label(label_data* ld, char* c)
{
  *(double *)c = ld->label;
  c += sizeof(ld->label);
  *(float *)c = ld->weight;
  c += sizeof(ld->weight);
  return c;
}

void cache_simple_label(void* v, io_buf& cache)
{
  char *c;
  label_data* ld = (label_data*) v;
  buf_write(cache, c, sizeof(ld->label)+sizeof(ld->weight));
  c = bufcache_simple_label(ld,c);
}

void default_simple_label(void* v)
{
  label_data* ld = (label_data*) v;
  ld->label = FLT_MAX;
  ld->weight = 1.;
  ld->undo = false;
}

void delete_simple_label(void* v)
{
}

void parse_simple_label(void* v, v_array<substring>& words)
{
  label_data* ld = (label_data*)v;

  switch(words.index()) {
  case 0:
    break;
  case 1:
    ld->label = double_of_substring(words[0]);
    break;
  case 2:
    ld->label = double_of_substring(words[0]);
    ld->weight = float_of_substring(words[1]);
    break;
  default:
    cerr << "malformed example!\n";
    cerr << "words.index() = " << words.index() << endl;
  }
}

