/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include <vector>
#include <math.h>
#include <string>
#include <string.h>
#include <float.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/file.h>
#include "parse_example.h"
#include "hash.h"
#include "source.h"
#include "cache.h"

inline size_t hashstring (substring s, unsigned long h)
{
  size_t ret = h;
  while (s.start != s.end)
    if (*s.start > 47 && *s.start < 58)
      ret = 10*ret + *(s.start++) - 48;
    else
      return uniform_hash((unsigned char *)s.start, s.end - s.start, h);

  return ret;
}

parser* new_parser(example_source* s, const label_parser* lp)
{
  parser* ret = (parser*) calloc(1,sizeof(parser));
  ret->source = s;
  ret->lp = lp;
  return ret;
}


int order_features(const void* first, const void* second)
{
  return ((feature*)first)->weight_index - ((feature*)second)->weight_index;
}

int order_audit_features(const void* first, const void* second)
{
  return ((audit_data*)first)->weight_index - ((audit_data*)second)->weight_index;
}

void feature_value(substring &s, v_array<substring>& name, float &v)
{
  tokenize(':', s, name);
  
  switch (name.index()) {
  case 0:
  case 1:
    break;
  case 2:
    v = float_of_substring(name[1]);
    if (isnan(v))
      {
	cerr << "error NaN value for feature: ";
	cerr.write(name[0].start, name[0].end - name[0].start);
	cerr << " terminating." << endl;
	exit(1);
      }
    break;
  default:
    cerr << "example with a wierd name.  What is ";
    cerr.write(s.start, s.end - s.start);
    cerr << "\n";
  }
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

// new should be empty. --Alex
template<class T> void copy_v_array(v_array<T>& old, v_array<T>& new_va)
{
  // allocate new memory for new.
  new_va.begin = (T *) malloc(sizeof(T)*old.index());

  // copy the old to the new.
  memcpy(new_va.begin,old.begin,sizeof(T)*old.index());
  
  // update pointers
  new_va.end = new_va.begin + old.index();
  new_va.end_array = new_va.end;
}

char* c_string_of_substring(substring s)
{
  size_t len = s.end - s.start+1;
  char* ret = (char *)calloc(len,sizeof(char));
  memcpy(ret,s.start,len-1);
  return ret;
}

char* copy(char* base)
{
  size_t len = 0;
  while (base[len++] != '\0');
  char* ret = (char *)calloc(len,sizeof(char));
  memcpy(ret,base,len);
  return ret;
}

void unique_sort_features(parser* p, example* ae)
{
  bool audit = p->source->global->audit;
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

int read_features(parser* p, example* ae)
{
  char *line=NULL;
  int num_chars = readto(p->source->input, line, '\n');
  if (num_chars == 0)
    return num_chars;
  
  substring example = {line, line + num_chars};

  tokenize('|', example, p->channels);
  p->lp->default_label(ae->ld);
  substring* feature_start = &(p->channels[1]);
  if (*line == '|')
    feature_start = &(p->channels[0]);
  else 
    p->lp->parse_label(ae->ld,p->channels[0],p->words);
  
  size_t mask = p->source->global->mask;
  bool audit = p->source->global->audit;
  for (substring* i = feature_start; i != p->channels.end; i++) {
    substring channel = *i;
    
    tokenize(' ',channel, p->words);
    if (p->words.begin == p->words.end)
      continue;
    
    float channel_v = 1.;
    size_t channel_hash;
    char* base=NULL;
    size_t index = 0;
    bool new_index = false;
    size_t feature_offset = 0;
    if (channel.start[0] != ' ')//we have a nonanonymous namespace
      {
	feature_offset++;
	feature_value(p->words[0], p->name, channel_v);

	if (p->name.index() > 0) {
	  index = (unsigned char)(*p->name[0].start);
	  if (ae->atomics[index].begin == ae->atomics[index].end)
	    new_index = true;
	}
	if (audit)
	  base = c_string_of_substring(p->name[0]);
	channel_hash = hashstring(p->name[0], hash_base);
      }
    else
      {
	index = (unsigned char)' ';
	if (ae->atomics[index].begin == ae->atomics[index].end)
	  new_index = true;
	if (audit)
	  {
	    base = (char *)calloc(2,sizeof(char));
	    base[0]=' ';
	    base[1]='\0';
	  }
	channel_hash = 0;
      }
 
    for (substring* i = p->words.begin+feature_offset; i != p->words.end; i++) {
      float v = channel_v;
      feature_value(*i, p->name, v);
      
      size_t word_hash = (hashstring(p->name[0], channel_hash)) & mask;
      feature f = {v,word_hash};
      push(ae->atomics[index], f);
    }

    if (new_index && ae->atomics[index].begin != ae->atomics[index].end)
      push(ae->indices, index);
    
    if (audit)
      {
	for (substring* i = p->words.begin+feature_offset; i != p->words.end; i++) {
	  float v = channel_v;
	  feature_value(*i, p->name, v);
	  
	  size_t word_hash = (hashstring(p->name[0], channel_hash)) & mask;
      
	  char* feature = c_string_of_substring(p->name[0]);
	  audit_data ad = {copy(base), feature, word_hash, v, true};
	  push(ae->audit_features[index], ad);
	}
	free(base);
      }
  }

  unique_sort_features(p,ae);

  return num_chars;
}

bool parse_atomic_example(parser* p, example *ae)
{
  if (p->source->global->audit)
    for (size_t* i = ae->indices.begin; i != ae->indices.end; i++) 
      {
	for (audit_data* temp 
	       = ae->audit_features[*i].begin; 
	     temp != ae->audit_features[*i].end; temp++)
	  {
	    if (temp->alloced)
	      {
		free(temp->space);
		free(temp->feature);
		temp->alloced=false;
	      }
	  }
	ae->audit_features[*i].erase();
      }

  for (size_t* i = ae->indices.begin; i != ae->indices.end; i++) 
    ae->atomics[*i].erase();

  ae->indices.erase();
  if (p->source->cache.file == -1 || p->source->write_cache){
    if (read_features(p, ae) <= 0)
      return false;
    if (p->source->write_cache) 
      cache_features(p, ae);
  }
  else
    {
      if (read_cached_features(p, ae) == 0) 
	return false;
      unique_sort_features(p,ae);
    }
  return true;
}

const size_t ring_size = 20;
example* examples;//A Ring of examples.
pthread_mutex_t examples_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t example_available = PTHREAD_COND_INITIALIZER;
pthread_cond_t example_unused = PTHREAD_COND_INITIALIZER;
size_t parsed_index; // The index of the parsed example.
size_t* used_index; // The index of the example currently used by thread i.
bool done;

example* get_unused_example()
{
  while (true)
    {
      pthread_mutex_lock(&examples_lock);
      if (examples[parsed_index % ring_size].in_use == false)
	{
	  examples[parsed_index % ring_size].in_use = true;
	  pthread_mutex_unlock(&examples_lock);
	  return examples + (parsed_index % ring_size);
	}
      else 
	pthread_cond_wait(&example_unused, &examples_lock);
      pthread_mutex_unlock(&examples_lock);
    }
}

void setup_example(example* ae, static_data* global)
{
  size_t num_threads = global->num_threads();

  ae->partial_prediction = 0.;
  ae->num_features = 1;
  ae->threads_to_finish = num_threads;	

  //Should loop through the features to determine the boundaries
  size_t length = global->mask + 1;
  size_t expert_size = length >> global->thread_bits; //#features/expert
  for (size_t* i = ae->indices.begin; i != ae->indices.end; i++) 
    {
      ae->subsets[*i].erase();
      feature* f = ae->atomics[*i].begin;
      push(ae->subsets[*i],f);
      size_t last_index = 0;
      for (; f != ae->atomics[*i].end && f->weight_index < length; f++)
	while (f->weight_index / expert_size != last_index) {
	  push(ae->subsets[*i],f);
	  last_index++;
	}
      while(ae->subsets[*i].index() < num_threads+1)
	push(ae->subsets[*i],f);
      
      ae->num_features += ae->atomics[*i].end - ae->atomics[*i].begin;
    }
  
  for (vector<string>::iterator i = global->pairs.begin(); i != global->pairs.end();i++) 
    {
    ae->num_features 
      += (ae->atomics[(int)(*i)[0]].end - ae->atomics[(int)(*i)[0]].begin)
      *(ae->atomics[(int)(*i)[1]].end - ae->atomics[(int)(*i)[1]].begin);
    }
}

void *main_parse_loop(void *in)
{
  parser* p = (parser*) in;
  
  while(!done)
    {
      example* ae=get_unused_example();

      if (parse_atomic_example(p, ae)) {	
	setup_example(ae,p->source->global);

	pthread_mutex_lock(&examples_lock);
	parsed_index++;
	pthread_cond_broadcast(&example_available);
	pthread_mutex_unlock(&examples_lock);

      }
      else
	{
	  pthread_mutex_lock(&examples_lock);
	  done = true;
	  ae->in_use = false;
	  pthread_cond_broadcast(&example_available);
	  pthread_mutex_unlock(&examples_lock);
	}
    }  

  free(p->channels.begin);
  p->channels.begin = p->channels.end = p->channels.end_array = NULL;
  free(p->words.begin);
  p->words.begin = p->words.end = p->words.end_array = NULL;
  free(p->name.begin);
  p->name.begin = p->name.end = p->name.end_array = NULL;

  return NULL;
}

pthread_t parse_thread;

void setup_parser(size_t num_threads, parser* pf)
{
  //This must be called first.
  used_index = (size_t*) calloc(num_threads, sizeof(size_t));
  parsed_index = 0;
  done = false;
  
  examples = (example*)calloc(ring_size, sizeof(example));
  
  for (size_t i = 0; i < ring_size; i++)
    {
      examples[i].lock = examples_lock;
      examples[i].ld = calloc(1,pf->lp->label_size);
    }
  pthread_create(&parse_thread, NULL, main_parse_loop, pf);
}

void destroy_parser(parser* pf)
{
  pthread_join(parse_thread, NULL);
  free(used_index);
  
  for (size_t i = 0; i < ring_size; i++) 
    {
      pf->lp->delete_label(examples[i].ld);
      free(examples[i].ld);
      for (size_t j = 0; j < 256; j++)
	{
	  if (examples[i].atomics[j].begin != examples[i].atomics[j].end_array)
	    free(examples[i].atomics[j].begin);
	  if (examples[i].audit_features[j].begin != examples[i].audit_features[j].end)
	    {
	      for (audit_data* temp = examples[i].audit_features[j].begin; 
		   temp != examples[i].audit_features[j].end; temp++)
		if (temp->alloced) {
		  free(temp->space);
		  free(temp->feature);
		  temp->alloced = false;
		}
	      free(examples[i].audit_features[j].begin);
	    }
	  if (examples[i].subsets[j].begin != examples[i].subsets[j].end_array)
	    free(examples[i].subsets[j].begin);
	}
      free(examples[i].indices.begin);
    }
  free(examples);
}

bool examples_to_finish()
{
  for(size_t i = 0; i < ring_size; i++)
    if (examples[i].in_use)
      return true;
  return false;
}

inline void finish_example(example* ec)
{
  pthread_mutex_lock(&examples_lock);
  if (-- ec->threads_to_finish == 0)
    {
      ec->in_use = false;
      pthread_cond_signal(&example_unused);
      if (done)
	pthread_cond_broadcast(&example_available);
    }
  pthread_mutex_unlock(&examples_lock);
}

example* get_example(example* ec, size_t thread_num)
{
  if (ec != NULL)
    finish_example(ec);
  
  while (true) // busy wait until an atomic example is acquired.
    {
      pthread_mutex_lock(&examples_lock);
      
      if (parsed_index != used_index[thread_num]) {
	size_t ring_index = used_index[thread_num]++ % ring_size;
	pthread_mutex_unlock(&examples_lock);
	return examples + ring_index;
      }
      else {
	if (!done || examples_to_finish()) {
	  pthread_cond_wait(&example_available, &examples_lock);
	  pthread_mutex_unlock(&examples_lock);
	}
	else 
	  { 
	    pthread_mutex_unlock(&examples_lock);
	    return NULL;
	  }
      }
    }
}

