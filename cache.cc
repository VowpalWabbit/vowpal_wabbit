/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include "cache.h"

size_t neg_1 = 1;
size_t general = 2;

char* run_len_decode(char *p, size_t& i)
{// read an int 7 bits at a time.
  size_t count = 0;
  while(*p & 128)\
    i = i | ((*(p++) & 127) << 7*count++);
  i = i | (*(p++) << 7*count);
  return p;
}

int read_cached_features(parser* p, example* ae)
{
  size_t mask = p->source->global->mask;

  size_t total = p->lp->read_cached_label(ae->ld, p->source->cache);
  if (total == 0)
    return 0;

  char* c;
  size_t num_indices = 0;
  if (buf_read(p->source->cache, c, int_size) < int_size) 
    return 0;
  c = run_len_decode(c, num_indices);
  p->source->cache.set(c);

  for (;num_indices > 0; num_indices--)
    {
      size_t temp;
      if((temp = buf_read(p->source->cache,c,int_size + sizeof(size_t))) < char_size + sizeof(size_t)) {
	cerr << "truncated example! " << temp << " " << char_size +sizeof(size_t) << endl;
	return 0;
      }

      size_t index = 0;
      c = run_len_decode(c, index);
      push(ae->indices, index);
      v_array<feature>* ours = ae->atomics+index;
      size_t storage = *(size_t *)c;
      c += sizeof(size_t);
      p->source->cache.set(c);
      total += storage;
      if (buf_read(p->source->cache,c,storage) < storage) {
	cerr << "truncated example!" << endl;
	return 0;
      }

      char *end = c+storage;

      size_t last = 0;
      
      for (;c!= end;)
	{	  
	  feature f = {1., 0};
	  size_t temp = f.weight_index;
	  c = run_len_decode(c,temp);
	  f.weight_index = temp;
	  if (f.weight_index & neg_1) 
	    f.x = -1.;
	  else if (f.weight_index & general)	    {
	      f.x = *(float *)c;
	      c += sizeof(float);
	    }
	  f.weight_index = last + (f.weight_index >> 2);
	  last = f.weight_index;
	  f.weight_index = f.weight_index & mask;
	  push(*ours, f);
	}
      p->source->cache.set(c);
    }

  return total;
}

char* run_len_encode(char *p, size_t i)
{// store an int 7 bits at a time.
  while (i >= 128)
    {
      *(p++) = (i & 127) | 128;
      i = i >> 7;
    }
  *(p++) = (i & 127);
  return p;
}

void cache_features(parser* p, example* ae)
{
  p->lp->cache_label(ae->ld,p->source->cache);
  char *c;
  
  buf_write(p->source->cache, c, int_size);
  c = run_len_encode(c, ae->indices.index());
  p->source->cache.set(c);

  for (size_t* b = ae->indices.begin; b != ae->indices.end; b++)
    {
      size_t storage = ae->atomics[*b].index() * int_size;
      feature* end = ae->atomics[*b].end;
      for (feature* i = ae->atomics[*b].begin; i != end; i++)
	if (i->x != 1. && i->x != -1.)
	  storage+=sizeof(float);

      buf_write(p->source->cache, c, int_size + storage + sizeof(size_t));
      c = run_len_encode(c, *b);
      char *storage_size_loc = c;
      c += sizeof(size_t);

      size_t last = 0;

      for (feature* i = ae->atomics[*b].begin; i != end; i++)
	{
	  size_t diff = (i->weight_index - last) << 2;
	  last = i->weight_index;
	  if (i->x == 1.) 
	    c = run_len_encode(c, diff);
	  else if (i->x == -1.) 
	    c = run_len_encode(c, diff | neg_1);
	  else {
	    c = run_len_encode(c, diff | general);
	    *(float *)c = i->x;
	    c += sizeof(float);
	  }
	}
      p->source->cache.set(c);
      *(size_t*)storage_size_loc = c - storage_size_loc - sizeof(size_t);
    }
}
