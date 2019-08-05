/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */

#include <cmath>
#include <math.h>
#include <ctype.h>
#include <boost/utility/string_view.hpp>
#include "parse_example.h"
#include "hash.h"
#include "unique_sort.h"
#include "global_data.h"
#include "constant.h"

using namespace std;

size_t read_features(vw* all, char*& line, size_t& num_chars)
{
  line = nullptr;
  size_t num_chars_initial = readto(*(all->p->input), line, '\n');
  if (num_chars_initial < 1)
    return num_chars_initial;
  num_chars = num_chars_initial;
  if (line[0] == '\xef' && num_chars >= 3 && line[1] == '\xbb' && line[2] == '\xbf')
  {
    line += 3;
    num_chars -= 3;
  }
  if (num_chars > 0 && line[num_chars - 1] == '\n')
    num_chars--;
  if (num_chars > 0 && line[num_chars - 1] == '\r')
    num_chars--;
  return num_chars_initial;
}

int read_features_string(vw* all, v_array<example*>& examples)
{
  char* line;
  size_t num_chars;
  size_t num_chars_initial = read_features(all, line, num_chars);
  if (num_chars_initial < 1)
    return (int)num_chars_initial;

  substring example = {line, line + num_chars};
  // boost::string_view example(line, num_chars);
  substring_to_example(all, examples[0], example);

  return (int)num_chars_initial;
}

template <bool audit>
class TC_parser
{
 public:
  char* beginLine;
  char* reading_head;
  char* endLine;
  float cur_channel_v;
  bool new_index;
  size_t anon;
  uint64_t channel_hash;
  char* base;
  unsigned char index;
  float v;
  bool redefine_some;
  unsigned char (*redefine)[256];
  parser* p;
  example* ae;
  uint64_t* affix_features;
  bool* spelling_features;
  v_array<char> spelling;
  uint32_t hash_seed;
  uint64_t parse_mask;

  vector<feature_dict*>* namespace_dictionaries;

  ~TC_parser() {}

  inline void parserWarning(const char* message, char* begin, char* pos, const char* message2)
  {
    std::stringstream ss;
    ss << message << std::string(begin, pos - begin).c_str() << message2 << "in Example #"
       << this->p->end_parsed_examples << ": \"" << std::string(this->beginLine, this->endLine).c_str() << "\"" << endl;
    if (p->strict_parse)
    {
      THROW_EX(VW::strict_parse_exception, ss.str());
    }
    else
    {
      cerr << ss.str();
    }
  }

  inline float featureValue()
  {
    if (*reading_head == ' ' || *reading_head == '\t' || *reading_head == '|' || reading_head == endLine ||
        *reading_head == '\r')
      return 1.;
    else if (*reading_head == ':')
    {
      // featureValue --> ':' 'Float'
      ++reading_head;
      char* end_read = nullptr;
      v = parseFloat(reading_head, &end_read, endLine);
      if (end_read == reading_head)
      {
        parserWarning("malformed example! Float expected after : \"", beginLine, reading_head, "\"");
      }
      if (std::isnan(v))
      {
        v = 0.f;
        parserWarning("warning: invalid feature value:\"", reading_head, end_read, "\" read as NaN. Replacing with 0.");
      }
      reading_head = end_read;
      return v;
    }
    else
    {
      // syntax error
      parserWarning("malformed example! '|', ':', space, or EOL expected after : \"", beginLine, reading_head, "\"");
      return 0.f;
    }
  }

  inline substring read_name()
  {
    substring ret;
    ret.begin = reading_head;
    while (!(*reading_head == ' ' || *reading_head == ':' || *reading_head == '\t' || *reading_head == '|' ||
        reading_head == endLine || *reading_head == '\r'))
      ++reading_head;
    ret.end = reading_head;

    return ret;
  }

  inline void maybeFeature()
  {
    if (*reading_head == ' ' || *reading_head == '\t' || *reading_head == '|' || reading_head == endLine ||
        *reading_head == '\r')
    {
      // maybeFeature --> ø
    }
    else
    {
      // maybeFeature --> 'String' FeatureValue
      substring feature_name = read_name();
      v = cur_channel_v * featureValue();
      uint64_t word_hash;
      if (feature_name.end != feature_name.begin)
        word_hash = (p->hasher(feature_name, channel_hash) & parse_mask);
      else
        word_hash = channel_hash + anon++;
      if (v == 0)
        return;  // dont add 0 valued features to list of features
      features& fs = ae->feature_space[index];
      fs.push_back(v, word_hash);
      if (audit)
      {
        v_array<char> feature_v = v_init<char>();
        push_many(feature_v, feature_name.begin, feature_name.end - feature_name.begin);
        feature_v.push_back('\0');
        fs.space_names.push_back(audit_strings_ptr(new audit_strings(base, feature_v.begin())));
        feature_v.delete_v();
      }
      if ((affix_features[index] > 0) && (feature_name.end != feature_name.begin))
      {
        features& affix_fs = ae->feature_space[affix_namespace];
        if (affix_fs.size() == 0)
          ae->indices.push_back(affix_namespace);
        uint64_t affix = affix_features[index];
        while (affix > 0)
        {
          bool is_prefix = affix & 0x1;
          uint64_t len = (affix >> 1) & 0x7;
          substring affix_name = {feature_name.begin, feature_name.end};
          if (affix_name.end > affix_name.begin + len)
          {
            if (is_prefix)
              affix_name.end = affix_name.begin + len;
            else
              affix_name.begin = affix_name.end - len;
          }
          word_hash =
              p->hasher(affix_name, (uint64_t)channel_hash) * (affix_constant + (affix & 0xF) * quadratic_constant);
          affix_fs.push_back(v, word_hash);
          if (audit)
          {
            v_array<char> affix_v = v_init<char>();
            if (index != ' ')
              affix_v.push_back(index);
            affix_v.push_back(is_prefix ? '+' : '-');
            affix_v.push_back('0' + (char)len);
            affix_v.push_back('=');
            push_many(affix_v, affix_name.begin, affix_name.end - affix_name.begin);
            affix_v.push_back('\0');
            affix_fs.space_names.push_back(audit_strings_ptr(new audit_strings("affix", affix_v.begin())));
          }
          affix >>= 4;
        }
      }
      if (spelling_features[index])
      {
        features& spell_fs = ae->feature_space[spelling_namespace];
        if (spell_fs.size() == 0)
          ae->indices.push_back(spelling_namespace);
        // v_array<char> spelling;
        spelling.clear();
        for (char* c = feature_name.begin; c != feature_name.end; ++c)
        {
          char d = 0;
          if ((*c >= '0') && (*c <= '9'))
            d = '0';
          else if ((*c >= 'a') && (*c <= 'z'))
            d = 'a';
          else if ((*c >= 'A') && (*c <= 'Z'))
            d = 'A';
          else if (*c == '.')
            d = '.';
          else
            d = '#';
          // if ((spelling.size() == 0) || (spelling.last() != d))
          spelling.push_back(d);
        }
        substring spelling_ss = {spelling.begin(), spelling.end()};
        uint64_t word_hash = hashstring(spelling_ss, (uint64_t)channel_hash);
        spell_fs.push_back(v, word_hash);
        if (audit)
        {
          v_array<char> spelling_v = v_init<char>();
          if (index != ' ')
          {
            spelling_v.push_back(index);
            spelling_v.push_back('_');
          }
          push_many(spelling_v, spelling_ss.begin, spelling_ss.end - spelling_ss.begin);
          spelling_v.push_back('\0');
          spell_fs.space_names.push_back(audit_strings_ptr(new audit_strings("spelling", spelling_v.begin())));
        }
      }
      if (namespace_dictionaries[index].size() > 0)
      {
        for (size_t dict = 0; dict < namespace_dictionaries[index].size(); dict++)
        {
          feature_dict* map = namespace_dictionaries[index][dict];
          uint64_t hash = uniform_hash(feature_name.begin, feature_name.end - feature_name.begin, quadratic_constant);
          features* feats = map->get(feature_name, hash);
          if ((feats != nullptr) && (feats->values.size() > 0))
          {
            features& dict_fs = ae->feature_space[dictionary_namespace];
            if (dict_fs.size() == 0)
              ae->indices.push_back(dictionary_namespace);
            push_many(dict_fs.values, feats->values.begin(), feats->values.size());
            push_many(dict_fs.indicies, feats->indicies.begin(), feats->indicies.size());
            dict_fs.sum_feat_sq += feats->sum_feat_sq;
            if (audit)
              for (size_t i = 0; i < feats->indicies.size(); ++i)
              {
                uint64_t id = feats->indicies[i];
                stringstream ss;
                ss << index << '_';
                for (char* fc = feature_name.begin; fc != feature_name.end; ++fc) ss << *fc;
                ss << '=' << id;
                dict_fs.space_names.push_back(audit_strings_ptr(new audit_strings("dictionary", ss.str())));
              }
          }
        }
      }
    }
  }

  inline void nameSpaceInfoValue()
  {
    if (*reading_head == ' ' || *reading_head == '\t' || reading_head == endLine || *reading_head == '|' ||
        *reading_head == '\r')
    {
      // nameSpaceInfoValue -->  ø
    }
    else if (*reading_head == ':')
    {
      // nameSpaceInfoValue --> ':' 'Float'
      ++reading_head;
      char* end_read = nullptr;
      cur_channel_v = parseFloat(reading_head, &end_read);
      if (end_read == reading_head)
      {
        parserWarning("malformed example! Float expected after : \"", beginLine, reading_head, "\"");
      }
      if (std::isnan(cur_channel_v))
      {
        cur_channel_v = 1.f;
        parserWarning(
            "warning: invalid namespace value:\"", reading_head, end_read, "\" read as NaN. Replacing with 1.");
      }
      reading_head = end_read;
    }
    else
    {
      // syntax error
      parserWarning("malformed example! '|',':', space, or EOL expected after : \"", beginLine, reading_head, "\"");
    }
  }

  inline void nameSpaceInfo()
  {
    if (reading_head == endLine || *reading_head == '|' || *reading_head == ' ' || *reading_head == '\t' ||
        *reading_head == ':' || *reading_head == '\r')
    {
      // syntax error
      parserWarning("malformed example! String expected after : \"", beginLine, reading_head, "\"");
    }
    else
    {
      // NameSpaceInfo --> 'String' NameSpaceInfoValue
      index = (unsigned char)(*reading_head);
      if (redefine_some)
        index = (*redefine)[index];  // redefine index
      if (ae->feature_space[index].size() == 0)
        new_index = true;
      substring name = read_name();
      if (audit)
      {
        v_array<char> base_v_array = v_init<char>();
        push_many(base_v_array, name.begin, name.end - name.begin);
        base_v_array.push_back('\0');
        if (base != nullptr)
          free(base);
        base = base_v_array.begin();
      }
      channel_hash = p->hasher(name, this->hash_seed);
      nameSpaceInfoValue();
    }
  }

  inline void listFeatures()
  {
    while ((*reading_head == ' ' || *reading_head == '\t') && (reading_head < endLine))
    {
      // listFeatures --> ' ' MaybeFeature ListFeatures
      ++reading_head;
      maybeFeature();
    }
    if (!(*reading_head == '|' || reading_head == endLine || *reading_head == '\r'))
    {
      // syntax error
      parserWarning("malformed example! '|',space, or EOL expected after : \"", beginLine, reading_head, "\"");
    }
  }

  inline void nameSpace()
  {
    cur_channel_v = 1.0;
    index = 0;
    new_index = false;
    anon = 0;
    if (*reading_head == ' ' || *reading_head == '\t' || reading_head == endLine || *reading_head == '|' ||
        *reading_head == '\r')
    {
      // NameSpace --> ListFeatures
      index = (unsigned char)' ';
      if (ae->feature_space[index].size() == 0)
        new_index = true;
      if (audit)
      {
        if (base != nullptr)
          free(base);
        base = calloc_or_throw<char>(2);
        base[0] = ' ';
        base[1] = '\0';
      }
      channel_hash = this->hash_seed == 0 ? 0 : uniform_hash("", 0, this->hash_seed);
      listFeatures();
    }
    else if (*reading_head != ':')
    {
      // NameSpace --> NameSpaceInfo ListFeatures
      nameSpaceInfo();
      listFeatures();
    }
    else
    {
      // syntax error
      parserWarning("malformed example! '|',String,space, or EOL expected after : \"", beginLine, reading_head, "\"");
    }
    if (new_index && ae->feature_space[index].size() > 0)
      ae->indices.push_back(index);
  }

  inline void listNameSpace()
  {
    while ((*reading_head == '|') && (reading_head < endLine))  // ListNameSpace --> '|' NameSpace ListNameSpace
    {
      ++reading_head;
      nameSpace();
    }
    if (reading_head != endLine && *reading_head != '\r')
    {
      // syntax error
      parserWarning("malformed example! '|' or EOL expected after : \"", beginLine, reading_head, "\"");
    }
  }

  TC_parser(char* reading_head, char* endLine, vw& all, example* ae)
  {
    spelling = v_init<char>();
    if (endLine != reading_head)
    {
      this->beginLine = reading_head;
      this->reading_head = reading_head;
      this->endLine = endLine;
      this->p = all.p;
      this->redefine_some = all.redefine_some;
      this->redefine = &all.redefine;
      this->ae = ae;
      this->affix_features = all.affix_features;
      this->spelling_features = all.spelling_features;
      this->namespace_dictionaries = all.namespace_dictionaries;
      this->base = nullptr;
      this->hash_seed = all.hash_seed;
      this->parse_mask = all.parse_mask;
      listNameSpace();
      if (base != nullptr)
        free(base);
    }
  }
};

std::vector<boost::string_view> split(boost::string_view phrase, char delimiter)
{
  std::vector<boost::string_view> list;
  size_t start_pos = 0;
  size_t end_pos = 0;

  while ((end_pos = phrase.find(delimiter, start_pos)) != boost::string_view::npos)
  {
    // don't insert empty elements
    if (start_pos != end_pos)
      list.emplace_back(phrase.begin() + start_pos, end_pos - start_pos);
    start_pos = end_pos + 1;
  }
  // don't insert empty string
  if (start_pos < phrase.size() - 1)
    list.emplace_back(phrase.begin() + start_pos, phrase.size() - start_pos);
  return list;
}

void substring_to_example(vw* all, example* ae, boost::string_view example)
{
  all->p->lp.default_label(&ae->l);

  size_t bar_idx = example.find('|');
  char* bar_location = (char*)example.end();
  if (bar_idx != boost::string_view::npos)
    bar_location = (char*)example.begin() + bar_idx;

  all->p->words.clear();
  if (bar_idx != 0)
  {
    boost::string_view label_space(example);
    if (bar_idx != boost::string_view::npos)
    {
      // a little bit iffy since bar_idx is based on example and we're working off label_space
      // but safe as long as this is the first manipulation after the copy
      label_space.remove_suffix(label_space.size() - bar_idx);
    }
    size_t tab_idx = label_space.find('\t');
    if (tab_idx != boost::string_view::npos)
    {
      label_space.remove_prefix(tab_idx + 1);
    }
    // TODO: don't copy, its dumb and temporary. If somebody sees this comment, I screwed up
    const auto& tokenized = split(label_space, ' ');
    for (const auto& tmp_str : tokenized)
    {
      substring temp = {(char*)tmp_str.begin(), (char*)tmp_str.end()};
      all->p->words.push_back(temp);
    }
    if (all->p->words.size() > 0 &&
        (all->p->words.last().end == label_space.end() ||
            *(all->p->words.last().begin) == '\''))  // The last field is a tag, so record and strip it off
    {
      substring tag = all->p->words.pop();
      if (*tag.begin == '\'')
        tag.begin++;
      push_many(ae->tag, tag.begin, tag.end - tag.begin);
    }
  }

  if (all->p->words.size() > 0)
    all->p->lp.parse_label(all->p, all->sd, &ae->l, all->p->words);

  if (all->audit || all->hash_inv)
    TC_parser<true> parser_line(bar_location, (char*)example.end(), *all, ae);
  else
    TC_parser<false> parser_line(bar_location, (char*)example.end(), *all, ae);
}

void substring_to_example(vw* all, example* ae, substring example)
{
  boost::string_view strview(example.begin, example.end - example.begin);
  substring_to_example(all, ae, strview);
}

namespace VW
{
void read_line(vw& all, example* ex, boost::string_view line)
{
  while (line.size() > 0 && line.back() == '\n') line.remove_suffix(1);
  substring_to_example(&all, ex, line);
}

void read_line(vw& all, example* ex, char* line) { return read_line(all, ex, boost::string_view(line)); }

void read_lines(vw* all, char* line, size_t /*len*/, v_array<example*>& examples)
{
  auto lines = split(line, '\n');
  for (size_t i = 0; i < lines.size(); i++)
  {
    // Check if a new empty example needs to be added.
    if (examples.size() < i + 1)
    {
      examples.push_back(&VW::get_unused_example(all));
    }
    read_line(*all, examples[i], lines[i]);
  }
}

}  // namespace VW
