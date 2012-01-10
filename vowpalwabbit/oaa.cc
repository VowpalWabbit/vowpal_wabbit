#include <float.h>
#include <math.h>

#include "oaa.h"
#include "simple_label.h"
#include "cache.h"

using namespace std;

char* bufread_oaa_label(oaa_data* ld, char* c)
{
  ld->label = *(uint32_t *)c;
  c += sizeof(ld->label);
  ld->weight = *(float *)c;
  c += sizeof(ld->weight);
  return c;
}

size_t read_cached_oaa_label(void* v, io_buf& cache)
{
  oaa_data* ld = (oaa_data*) v;
  char *c;
  size_t total = sizeof(ld->label)+sizeof(ld->weight);
  if (buf_read(cache, c, total) < total) 
    return 0;
  c = bufread_oaa_label(ld,c);

  return total;
}

float oaa_weight(void* v)
{
  oaa_data* ld = (oaa_data*) v;
  return ld->weight;
}

float oaa_initial(void* v)
{
  return 0.;
}

char* bufcache_oaa_label(oaa_data* ld, char* c)
{
  *(uint32_t *)c = ld->label;
  c += sizeof(ld->label);
  *(float *)c = ld->weight;
  c += sizeof(ld->weight);
  return c;
}

void cache_oaa_label(void* v, io_buf& cache)
{
  char *c;
  oaa_data* ld = (oaa_data*) v;
  buf_write(cache, c, sizeof(ld->label)+sizeof(ld->weight));
  c = bufcache_oaa_label(ld,c);
}

void default_oaa_label(void* v)
{
  oaa_data* ld = (oaa_data*) v;
  ld->label = -1;
  ld->weight = 1.;
}

void delete_oaa_label(void* v)
{
}

void parse_oaa_label(void* v, v_array<substring>& words)
{
  oaa_data* ld = (oaa_data*)v;

  switch(words.index()) {
  case 0:
    break;
  case 1:
    ld->label = float_of_substring(words[0]);
    break;
  case 2:
    ld->label = float_of_substring(words[0]);
    ld->weight = float_of_substring(words[1]);
    break;
  default:
    cerr << "malformed example!\n";
    cerr << "words.index() = " << words.index() << endl;
  }
}

size_t k=0;
label_data simple_temp;
oaa_data* oaa_label_data;
size_t increment=0;
size_t counter = 0;
example* current_example=NULL;

void parse_oaa_flag(size_t s)
{
  *(global.lp) = oaa_label;
  global.get_example = get_oaa_example;
  global.return_example = return_oaa_example;
  k = s;
  counter = 1;
  increment = global.length()/k;
}

void return_oaa_example(example* ec)
{
  if (ec==NULL)
    {
      free_example(ec);
      current_example = NULL;
      counter = 1;
    }
  if (counter == k)
    {
      ec->ld = oaa_label_data;
      free_example(ec);
      current_example = NULL;
      counter = 1;
    }
  else
    {
      counter++;
      current_example = ec;
    }
}

void update_indicies(example* ec, size_t amount)
{
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      feature* end = ec->atomics[*i].end;
      for (feature* f = ec->atomics[*i].begin; f!= end; f++)
	f->weight_index += amount;
    }
}

example* get_oaa_example()
{
  if (current_example == NULL) {
    current_example=get_example();
  }
  
  if (current_example == NULL)
    return NULL;
  if (counter == 1)
    oaa_label_data = (oaa_data*)current_example->ld;
  else
    current_example->partial_prediction = 0.;
  if (oaa_label_data->label == counter)
    simple_temp.label = 1;
  else
    simple_temp.label = -1;
  
  simple_temp.weight = oaa_label_data->weight;
  current_example->ld = &simple_temp;
  if (counter != 1)
    update_indicies(current_example, increment);
  return current_example;
}
