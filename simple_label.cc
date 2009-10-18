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
  size_t tag_size = 0;
  size_t total = sizeof(ld->label)+sizeof(ld->weight)+sizeof(tag_size);
  if (buf_read(cache, c, total) < total) 
    return 0;
  c = bufread_simple_label(ld,c);
  
  tag_size = *(size_t*)c;
  c += sizeof(tag_size);

  cache.set(c);
  if (buf_read(cache, c, tag_size) < tag_size) 
    return 0;

  ld->tag.erase();
  push_many(ld->tag, c, tag_size); 
  return total+tag_size;
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
  buf_write(cache, c, sizeof(ld->label)+sizeof(ld->weight)+sizeof(ld->tag.index())+ld->tag.index());
  c = bufcache_simple_label(ld,c);

  *(size_t*)c = ld->tag.index();
  c += sizeof(ld->tag.index());

  memcpy(c,ld->tag.begin,ld->tag.index());
  c += ld->tag.index();

  cache.set(c);
}

void default_simple_label(void* v)
{
  label_data* ld = (label_data*) v;
  ld->label = FLT_MAX;
  ld->weight = 1.;
  ld->undo = false;
  ld->tag.erase();
}

void delete_simple_label(void* v)
{
  label_data* ld = (label_data*) v;
  if (ld->tag.end_array != ld->tag.begin)
    {
      free(ld->tag.begin);
      ld->tag.end_array = ld->tag.begin;
    }
}

void parse_simple_label(void* v, substring label_space, v_array<substring>& words)
{
  label_data* ld = (label_data*)v;
  char* tab_location = safe_index(label_space.start,'\t',label_space.end);
  if (tab_location != label_space.end)
    label_space.start = tab_location+1;
  
  tokenize(' ',label_space, words);
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
  case 3:
    ld->label = double_of_substring(words[0]);
    
    ld->weight = float_of_substring(words[1]);
    push_many(ld->tag, words[2].start, 
	      words[2].end - words[2].start);
    if (ld->tag.index() == 4 && ld->tag[0] == 'u' && ld->tag[1] == 'n' && ld->tag[2] == 'd' && ld->tag[3] == 'o')
      ld->undo = true;
    break;
  default:
    cerr << "malformed example!\n";
    cerr << "words.index() = " << words.index() << endl;
  }
}

