/* 
Copyright (c) 2007 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include <vector.h>
#include <float.h>
#include <fcntl.h>
#include <pthread.h>

#ifdef __APPLE__
#include "platform.h"
#include "getline.h"
#endif

#include "parse_example.h"
#include "hash.h"

pthread_mutex_t input_mutex;

void parse(char delim, substring s, v_array<substring> &ret)
{
  char *last = s.start;
  for (; s.start != s.end; s.start++) {
    if (*s.start == delim) {
      if (s.start != last)
	{
	  substring temp = {last,s.start};
	  push(ret, temp);
	}
      last = s.start+1;
    }
  }
  if (s.start != last)
    {
      substring final = {last, s.start};
      push(ret, final);
    }
}

void unmark(bool* in_already, v_array<feature> &features)
{
  for (feature* j =features.begin; j != features.end; j++)
    in_already[j->weight_index] = false;
}

void mark(bool* in_already, v_array<feature> &features)
{
  for (feature* j =features.begin; j != features.end; j++)
    in_already[j->weight_index] = true;
}

inline size_t hashstring (substring s, unsigned long h)
{
  return hash((unsigned char *)s.start, s.end - s.start, h);
}

inline void add_unique_index(bool *in_already, v_array<feature> &features, 
			     float v, size_t hashvalue)
{
  size_t index = hashvalue;
  if (in_already[index] != true)
    {
      in_already[index] = true;
      feature f = {v,index};
      push(features, f);
    }
}

void new_quadratic(bool *in_already, v_array<feature> &pair_features, 
		   const v_array<feature> &first_part, 
		   const v_array<feature> &second_part, size_t mask)
{
  for (feature* i = first_part.begin; i != first_part.end; i++)
    {
      size_t halfhash = 16381 * i->weight_index;
      float i_value = i->x;
      for (feature* ele = second_part.begin; ele != second_part.end; ele++)
	add_unique_index(in_already, pair_features, i_value * ele->x, 
			 (halfhash + ele->weight_index) & mask);
    }
}

size_t neg_1 = 1;
size_t general = 2;

char* run_len_decode(char *p, size_t& i)
{// read an int 7 bits at a time.
  size_t count = 0;
  while(*p & 128)
    i = i | ((*(p++) & 127) << 7*count++);
  i = i | (*(p++) << 7*count);
  return p;
}

pthread_mutex_t cache_lock;
size_t int_size = 5;
size_t char_size = 2;

int read_cached_features(float& label, float& weight, io_buf &cache, 
			 v_array<char> &tag, v_array<feature>* atomics, 
			 v_array<size_t> &indicies)
{
  char *p;
  pthread_mutex_lock(&cache_lock);
  size_t total = sizeof(label)+sizeof(weight)+int_size*2;
  size_t num_indicies = 0;
  size_t tag_size = 0;
  if (buf_read(cache, p, total) < total) 
    goto badness;

  label = *(float *)p;
  p += sizeof(float);
  weight = *(float *)p;
  p += sizeof(float);
  p = run_len_decode(p, num_indicies);
  p = run_len_decode(p, tag_size);
  cache.set(p);
  if (buf_read(cache, p, tag_size) < tag_size) 
    goto badness;
  push_many(tag, p, tag_size);
  
  for (;num_indicies > 0; num_indicies--)
    {
      size_t temp;
      if((temp = buf_read(cache,p,int_size + sizeof(size_t))) < char_size + sizeof(size_t)) {
	cerr << "truncated example! " << temp << " " << char_size +sizeof(size_t) << endl;
	goto badness;
      }

      size_t index = 0;
      p = run_len_decode(p, index);
      push(indicies, index);
      v_array<feature>* ours = atomics+index;
      size_t storage = *(size_t *)p;
      p += sizeof(size_t);
      cache.set(p);
      total += storage;
      if (buf_read(cache,p,storage) < storage) {
	cerr << "truncated example!" << endl;
	goto badness;
      }

      char *end = p+storage;

      size_t last = 0;
      
      for (;p!= end;)
	{	  
	  feature f = {1., 0};
	  p = run_len_decode(p,f.weight_index);
	  if (f.weight_index & neg_1) 
	    f.x = -1.;
	  else if (f.weight_index & general)
	    {
	      f.x = *(float *)p;
	      p += sizeof(float);
	    }
	  f.weight_index = last + (f.weight_index >> 2);
	  last = f.weight_index;
	  push(*ours, f);
	}
      cache.set(p);
    }
  pthread_mutex_unlock(&cache_lock);

  return total;

 badness:
  pthread_mutex_unlock(&cache_lock);
  return 0;
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

int order_features(const void* first, const void* second)
{
  return ((feature*)first)->weight_index - ((feature*)second)->weight_index;
}

void cache_features(float label, float weight, io_buf &cache, 
		    v_array<char> &tag, v_array<feature>* atomics, 
		    v_array<size_t> &indicies)
{
  char *p;

  pthread_mutex_lock(&cache_lock);
  
  buf_write(cache, p, sizeof(label)+sizeof(weight)+int_size*2+tag.index());
  
  *(float *)p = label;
  p += sizeof(label);
  *(float *)p = weight;
  p += sizeof(weight);
  
  p = run_len_encode(p, indicies.index());
  p = run_len_encode(p, tag.index());
  memcpy(p,tag.begin,tag.index());
  p += tag.index();
  cache.set(p);
  
  for (size_t* b = indicies.begin; b != indicies.end; b++)
    {
      size_t storage = atomics[*b].index() * int_size;
      feature* end = atomics[*b].end;
      for (feature* i = atomics[*b].begin; i != end; i++)
	if (i->x != 1. && i->x != -1.)
	  storage+=sizeof(float);

      buf_write(cache, p, int_size + storage + sizeof(size_t));
      p = run_len_encode(p, *b);
      char *storage_size_loc = p;
      p += sizeof(size_t);

      qsort(atomics[*b].begin, atomics[*b].index(), sizeof(feature), 
	    order_features);
      size_t last = 0;

      for (feature* i = atomics[*b].begin; i != end; i++)
	{
	  size_t diff = (i->weight_index - last) << 2;
	  last = i->weight_index;
	  if (i->x == 1.) 
	    p = run_len_encode(p, diff);
	  else if (i->x == -1.) 
	    p = run_len_encode(p, diff | neg_1);
	  else {
	    p = run_len_encode(p, diff | general);
	    *(float *)p = i->x;
	    p += sizeof(float);
	  }
	}
      cache.set(p);
      *(size_t*)storage_size_loc = p - storage_size_loc - sizeof(size_t);
    }
  pthread_mutex_unlock(&cache_lock);
}

inline float float_of_substring(substring s)
{
  return atof(string(s.start, s.end-s.start).c_str());
}

void feature_value(substring &s, v_array<substring>& name, float &v)
{
  name.erase();
  parse(':', s, name);
  
  switch (name.index()) {
  case 0:
  case 1:
    break;
  case 2:
    v = float_of_substring(name[1]);
    break;
  default:
    cerr << "example with a wierd name.  What is ";
    cerr.write(s.start, s.end - s.start);
    cerr << "\n";
  }
}

int read_features(thread_data &stuff, FILE* data, float &label, 
		  float &weight, v_array<char> &tag, size_t mask)
{
  pthread_mutex_lock(&input_mutex);
  int num_chars = getline (&stuff.line, &stuff.linesize, data);
  if (num_chars == -1) {
    pthread_mutex_unlock(&input_mutex);
    return num_chars;
  }
  
  substring page_offer = {stuff.line, stuff.line + num_chars-1};
  
  stuff.channels.erase();
  parse('|', page_offer, stuff.channels);
  
  stuff.words.erase();
  parse(' ',stuff.channels[0], stuff.words);
  switch(stuff.words.index()) {
  case 0:
    label = FLT_MAX;
    break;
  case 1:
    label = float_of_substring(stuff.words[0]);
    weight = 1.;
    break;
  case 2:
    label = float_of_substring(stuff.words[0]);
    weight = float_of_substring(stuff.words[1]);
    break;
  case 3:
    label = float_of_substring(stuff.words[0]);
    weight = float_of_substring(stuff.words[1]);
    push_many(tag, stuff.words[2].start, 
	      stuff.words[2].end - stuff.words[2].start);
    break;
  default:
    cerr << "malformed example!\n";
  }


  for (substring* i = stuff.channels.begin+1; i != stuff.channels.end; i++) {
    substring channel = *i;
    
    stuff.words.erase();
    parse(' ',channel, stuff.words);
    if (stuff.words.begin == stuff.words.end)
      continue;
    
    float channel_v = 1.;
    feature_value(stuff.words[0], stuff.name, channel_v);
    
    size_t index = 0;
    bool new_index = false;
    if (stuff.name.index() > 0) {
      index = (unsigned char)*(stuff.name[0].start);
      if (stuff.atomics[index].begin == stuff.atomics[index].end)
	new_index = true;
    }
    
    size_t channel_hash = hashstring(stuff.name[0], 0);
    for (substring* i = stuff.words.begin+1; i != stuff.words.end; i++) {
      float v = channel_v;
      feature_value(*i, stuff.name, v);
      
      size_t word_hash = (hashstring(stuff.name[0], channel_hash)) & mask;
      add_unique_index(stuff.in_already, stuff.atomics[index], v, word_hash);
    }
    if (new_index && stuff.atomics[index].begin != stuff.atomics[index].end)
      push(stuff.indicies, index);
  }
  
  pthread_mutex_unlock(&input_mutex);
  return num_chars;
}

bool parse_example(thread_data &stuff, example_file &example_source, 
		   regressor &reg, v_array<feature> &features,
		   float &label, float &weight, v_array<char> &tag)
{
  features.erase();
  tag.erase();

  for (size_t* i = stuff.indicies.begin; i != stuff.indicies.end; i++) 
    stuff.atomics[*i].erase();
  stuff.indicies.erase();

  if (example_source.cache.file == -1 || example_source.write_cache){
    
    if (read_features(stuff, example_source.data, label, weight, tag, 
		      example_source.mask) <= 0)
      return false;
    
    if (example_source.write_cache)
      cache_features(label,weight, example_source.cache, tag, 
		     stuff.atomics, stuff.indicies);
  }
  else // use the read_cache.
    {
      if (read_cached_features(label, weight, example_source.cache, tag, 
			       stuff.atomics, stuff.indicies) <= 0)
	return false;
    }  

  for (size_t* i = stuff.indicies.begin; i != stuff.indicies.end; i++) 
    push_many(features, stuff.atomics[*i].begin, stuff.atomics[*i].index());

  if (example_source.cache.file != -1 && !example_source.write_cache 
      /*&& reg.pairs.size() > 0*/)
    mark(stuff.in_already, features);
  
  for (vector<string>::iterator i = reg.pairs.begin(); i != reg.pairs.end();i++) 
    new_quadratic(stuff.in_already, features, stuff.atomics[(int)(*i)[0]], 
		  stuff.atomics[(int)(*i)[1]], example_source.mask);
  
  add_unique_index(stuff.in_already, features, 1, 0);//a constant features

  unmark(stuff.in_already, features);

  return true;
}

bool inconsistent_cache(size_t numbits, io_buf &cache)
{
  size_t total = sizeof(numbits);
  char *p;
  if (buf_read(cache, p, total) < total) 
    return true;

  size_t cache_numbits = *(size_t *)p;
  if (cache_numbits != numbits)
    return true;

  return false;
}

void reset(size_t numbits, example_file &source)
{
  if (source.write_cache)
    {
      source.cache.flush();
      source.write_cache = false;
      close(source.cache.file);
      rename(source.cache.currentname.c_str(), source.cache.finalname.c_str());
      source.cache.file = open(source.cache.finalname.c_str(), O_RDONLY|O_LARGEFILE);
    }
  if (source.cache.file != -1)
    {
      lseek(source.cache.file, 0, SEEK_SET);
      alloc(source.cache.space, 1 << 16);
      source.cache.fill(0);
      if (inconsistent_cache(numbits, source.cache)) {
	cout << "argh, a bug in caching of some sort!  Exitting\n" ;
	exit(1);
      }
    }
  else
    fseek(source.data, 0, SEEK_SET);
}

void finalize_source(example_file &source)
{
  fclose(source.data);
  if (source.cache.file != -1) {
    close(source.cache.file);  
    free(source.cache.space.begin);
  }
}
