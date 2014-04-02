/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */

#include <math.h>
#include <ctype.h>
#include "parse_example.h"
#include "hash.h"
#include "cache.h"
#include "unique_sort.h"
#include "global_data.h"
#include "constant.h"
#include "memory.h"

using namespace std;

size_t hashstring (substring s, uint32_t h)
{
  size_t ret = 0;
  //trim leading whitespace but not UTF-8
  for(; s.begin < s.end && *(s.begin) <= 0x20 && (int)*(s.begin) >= 0; s.begin++);
  //trim trailing white space but not UTF-8
  for(; s.end > s.begin && *(s.end-1) <= 0x20 && (int)*(s.end-1) >=0; s.end--);

  char *p = s.begin;
  while (p != s.end)
    if (*p >= '0' && *p <= '9')
      ret = 10*ret + *(p++) - '0';
    else
      return uniform_hash((unsigned char *)s.begin, s.end - s.begin, h);

  return ret + h;
}

size_t hashall (substring s, uint32_t h)
{
  return uniform_hash((unsigned char *)s.begin, s.end - s.begin, h);
}

hash_func_t getHasher(const string& s){
  if (s=="strings")
    return hashstring;
  else if(s=="all")
    return hashall;
  else{
    cerr << "Unknown hash function: " << s << ". Exiting " << endl;
    throw exception();
  }
}

char* copy(char* base)
{
  size_t len = 0;
  while (base[len++] != '\0');
  char* ret = (char *)calloc_or_die(len,sizeof(char));
  memcpy(ret,base,len);
  return ret;
}

class TC_parser {
  
public:
  char* beginLine;
  char* reading_head;
  char* endLine;
  float cur_channel_v;
  bool  new_index;
  size_t anon; 
  bool audit;
  size_t channel_hash;
  char* base;
  unsigned char index;
  float v;
  parser* p;
  example* ae;
  uint32_t weights_per_problem;
  uint32_t* affix_features;
  bool* spelling_features;
  v_array<char> spelling;
  
  ~TC_parser(){ }
  
  inline float featureValue(){
    if(reading_head == endLine || *reading_head == '|' || *reading_head == ' ' || *reading_head == '\t' || *reading_head == '\r')
      return 1.;
    else if(*reading_head == ':'){
      // featureValue --> ':' 'Float'
      ++reading_head;
      char *end_read = NULL;
      v = parseFloat(reading_head,&end_read);
      if(end_read == reading_head){
	cout << "malformed example !\nFloat expected after : \"" << std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
      }
      if(nanpattern(v)) {
        v = 0.f;
        cout << "warning: invalid feature value:\"" << std::string(reading_head, end_read - reading_head).c_str() << "\" read as NaN. Replacing with 0." << endl;
      }
      reading_head = end_read;
      return v;
    }else{
      // syntax error
      cout << "malformed example !\n'|' , ':' , space or EOL expected after : \"" << std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
      return 0.f;
    }
  }

  inline substring read_name(){
    substring ret;
    ret.begin = reading_head;
    while( !(*reading_head == ' ' || *reading_head == '\t' || *reading_head == ':' ||*reading_head == '|' || reading_head == endLine || *reading_head == '\r' ))
      ++reading_head;
    ret.end = reading_head;

    return ret;
  }
  
  inline void maybeFeature(){
    if(*reading_head == ' ' || *reading_head == '\t' || *reading_head == '|'|| reading_head == endLine || *reading_head == '\r' ){
      // maybeFeature --> ø
    }else {
      // maybeFeature --> 'String' FeatureValue
      substring feature_name=read_name();
      v = cur_channel_v * featureValue();
      size_t word_hash;
      if (feature_name.end != feature_name.begin)
	word_hash = (p->hasher(feature_name,(uint32_t)channel_hash));
      else
	word_hash = channel_hash + anon++;
      if(v == 0) return; //dont add 0 valued features to list of features
      feature f = {v,(uint32_t)word_hash * weights_per_problem};
      ae->sum_feat_sq[index] += v*v;
      ae->atomics[index].push_back(f);
      if(audit){
	v_array<char> feature_v;
	push_many(feature_v, feature_name.begin, feature_name.end - feature_name.begin);
	feature_v.push_back('\0');
	audit_data ad = {copy(base),feature_v.begin,word_hash,v,true};
	ae->audit_features[index].push_back(ad);
      }
      if ((affix_features[index] > 0) && (feature_name.end != feature_name.begin)) {
        if (ae->atomics[affix_namespace].size() == 0)
          ae->indices.push_back(affix_namespace);
        uint32_t affix = affix_features[index];
        while (affix > 0) {
          bool is_prefix = affix & 0x1;
          uint32_t len   = (affix >> 1) & 0x7;
          substring affix_name = { feature_name.begin, feature_name.end };
          if (affix_name.end > affix_name.begin + len) {
            if (is_prefix)
              affix_name.end = affix_name.begin + len;
            else
              affix_name.begin = affix_name.end - len;
          }
          word_hash = p->hasher(affix_name,(uint32_t)channel_hash) * (affix_constant + (affix & 0xF) * quadratic_constant);
          feature f2 = { v, (uint32_t) word_hash * weights_per_problem };
          ae->sum_feat_sq[affix_namespace] += v*v;
          ae->atomics[affix_namespace].push_back(f2);
          if (audit) {
            v_array<char> affix_v;
            if (index != ' ') affix_v.push_back(index);
            affix_v.push_back(is_prefix ? '+' : '-');
            affix_v.push_back('0' + len);
            affix_v.push_back('=');
            push_many(affix_v, affix_name.begin, affix_name.end - affix_name.begin);
            affix_v.push_back('\0');
            audit_data ad = {copy((char*)"affix"),affix_v.begin,word_hash,v,true};
            ae->audit_features[affix_namespace].push_back(ad);
          }
          
          affix >>= 4;
        }
      }
      if (spelling_features[index]) {
        if (ae->atomics[spelling_namespace].size() == 0)
          ae->indices.push_back(spelling_namespace);
        //v_array<char> spelling;
        spelling.erase();
        for (char*c = feature_name.begin; c!=feature_name.end; ++c) {
          char d = 0;
          if      ((*c >= '0') && (*c <= '9')) d = '0';
          else if ((*c >= 'a') && (*c <= 'z')) d = 'a';
          else if ((*c >= 'A') && (*c <= 'Z')) d = 'A';
          else if  (*c == '.')                 d = '.';
          else                                 d = '#';
          //if ((spelling.size() == 0) || (spelling.last() != d))
            spelling.push_back(d);
        }
        substring spelling_ss = { spelling.begin, spelling.end };
        size_t word_hash = hashstring(spelling_ss, (uint32_t)channel_hash);
        feature f2 = { v, (uint32_t) word_hash * weights_per_problem };
        ae->sum_feat_sq[spelling_namespace] += v*v;
        ae->atomics[spelling_namespace].push_back(f2);
        if (audit) {
          v_array<char> spelling_v;
          if (index != ' ') { spelling_v.push_back(index); spelling_v.push_back('_'); }
          push_many(spelling_v, spelling_ss.begin, spelling_ss.end - spelling_ss.begin);
          spelling_v.push_back('\0');
          audit_data ad = {copy((char*)"spelling"),spelling_v.begin,word_hash,v,true};
          ae->audit_features[spelling_namespace].push_back(ad);
        }
      }
    }
  }
  
  inline void nameSpaceInfoValue(){
    if(*reading_head == ' ' || *reading_head == '\t' || reading_head == endLine || *reading_head == '|' || *reading_head == '\r' ){
      // nameSpaceInfoValue -->  ø
    }else if(*reading_head == ':'){
      // nameSpaceInfoValue --> ':' 'Float'
      ++reading_head;
      char *end_read = NULL;
      cur_channel_v = parseFloat(reading_head,&end_read);
      if(end_read == reading_head){
	cout << "malformed example !\nFloat expected after : \"" << std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
      }
      if(nanpattern(cur_channel_v)) {
        cur_channel_v = 1.f;
        cout << "warning: invalid namespace value:\"" << std::string(reading_head, end_read - reading_head).c_str() << "\" read as NaN. Replacing with 1." << endl;
      }
      reading_head = end_read;
    }else{
      // syntax error
      cout << "malformed example !\n'|' , ':' , space or EOL expected after : \"" << std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
    }
  }
  
  inline void nameSpaceInfo(){
    if(reading_head == endLine ||*reading_head == '|' || *reading_head == ' ' || *reading_head == '\t' || *reading_head == ':' || *reading_head == '\r'){
      // syntax error
      cout << "malformed example !\nString expected after : " << std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
    }else{
      // NameSpaceInfo --> 'String' NameSpaceInfoValue
      index = (unsigned char)(*reading_head);
      if(ae->atomics[index].begin == ae->atomics[index].end)
	new_index = true;
      substring name = read_name();
      if(audit){
	v_array<char> base_v_array;
	push_many(base_v_array, name.begin, name.end - name.begin);
	base_v_array.push_back('\0');
	if (base != NULL)
	  free(base);
	base = base_v_array.begin;
      }
      channel_hash = p->hasher(name, hash_base);
      nameSpaceInfoValue();
    }
  }
  
  inline void listFeatures(){
    while(*reading_head == ' ' || *reading_head == '\t'){
      //listFeatures --> ' ' MaybeFeature ListFeatures
      ++reading_head;
      maybeFeature();
    }
    if(!(*reading_head == '|' || reading_head == endLine || *reading_head == '\r')){
      //syntax error
      cout << "malformed example !\n'|' , space or EOL expected after : \"" << std::string(beginLine, reading_head - beginLine).c_str() << "\"" << endl;
    }
  }
    
  inline void nameSpace(){
    cur_channel_v = 1.0;
    index = 0;
    new_index = false;
    anon = 0;
    if(*reading_head == ' ' || *reading_head == '\t' || reading_head == endLine || *reading_head == '|' || *reading_head == '\r' ){
      // NameSpace --> ListFeatures
      index = (unsigned char)' ';
      if(ae->atomics[index].begin == ae->atomics[index].end)
	new_index = true;
      if(audit)
	{
	  if (base != NULL)
	    free(base);
	  base = (char *) calloc_or_die(2,sizeof(char));
	  base[0] = ' ';
	  base[1] = '\0';
	}
      channel_hash = 0;
      listFeatures();
    }else if(*reading_head != ':'){
      // NameSpace --> NameSpaceInfo ListFeatures
      nameSpaceInfo();
      listFeatures();
    }else{
      // syntax error
      cout << "malformed example !\n'|' , String, space or EOL expected after : \"" << std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
    }
    if(new_index && ae->atomics[index].begin != ae->atomics[index].end)
      ae->indices.push_back(index);
  }
  
  inline void listNameSpace(){
    while(*reading_head == '|'){ // ListNameSpace --> '|' NameSpace ListNameSpace
      ++reading_head;
      nameSpace();
    }
    if(reading_head != endLine && *reading_head != '\r')
      {
	// syntax error
	cout << "malformed example !\n'|' or EOL expected after : \"" << std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
      }
  }

  TC_parser(char* reading_head, char* endLine, vw& all, example* ae){
    if (endLine != reading_head)
      {
	this->beginLine = reading_head;
	this->reading_head = reading_head;
	this->endLine = endLine;
	this->p = all.p;
	this->ae = ae;
	this->weights_per_problem = all.wpp;
	this->affix_features = all.affix_features;
	this->spelling_features = all.spelling_features;
	this->base = NULL;
	audit = all.audit || all.hash_inv;
	listNameSpace();
	if (base != NULL)
	  free(base);
      }
  }
};

void substring_to_example(vw* all, example* ae, substring example)
{
  all->p->lp.default_label(ae->ld);
  char* bar_location = safe_index(example.begin, '|', example.end);
  char* tab_location = safe_index(example.begin, '\t', bar_location);
  substring label_space;
  if (tab_location != bar_location){
    label_space.begin = tab_location + 1;
  }else{
    label_space.begin = example.begin;
  }
  label_space.end = bar_location;
  
  if (*example.begin == '|')	{
    all->p->words.erase();
  } else 	{
    tokenize(' ', label_space, all->p->words);
    if (all->p->words.size() > 0 && (all->p->words.last().end == label_space.end	|| *(all->p->words.last().begin) == '\'')) //The last field is a tag, so record and strip it off
      {
	substring tag = all->p->words.pop();
	if (*tag.begin == '\'')
	  tag.begin++;
	push_many(ae->tag, tag.begin, tag.end - tag.begin);
      }
  }

  if (all->p->words.size() > 0)
    all->p->lp.parse_label(all->p, all->sd, ae->ld, all->p->words);
  
  TC_parser parser_line(bar_location,example.end,*all,ae);
}

int read_features(void* in, example* ex)
{
  vw* all = (vw*)in;
  example* ae = (example*)ex;
  char *line=NULL;
  size_t num_chars_initial = readto(*(all->p->input), line, '\n');
  if (num_chars_initial < 1)
    return (int)num_chars_initial;
  size_t num_chars = num_chars_initial;
  if (line[0] =='\xef' && num_chars >= 3 && line[1] == '\xbb' && line[2] == '\xbf') {
    line += 3;
    num_chars -= 3;
  }
  if (line[num_chars-1] == '\n')
    num_chars--;
  if (line[num_chars-1] == '\r')
    num_chars--;
  substring example = {line, line + num_chars};
  substring_to_example(all, ae, example);

  return (int)num_chars_initial;
}

void read_line(vw& all, example* ex, char* line)
{
  substring ss = {line, line+strlen(line)};
  substring_to_example(&all, ex, ss);  
}
