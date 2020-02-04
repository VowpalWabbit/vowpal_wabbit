// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cmath>
#include <cmath>
#include <cctype>
#include "parse_example.h"
#include "hash.h"
#include "unique_sort.h"
#include "global_data.h"
#include "constant.h"
#include "vw_string_view.h"

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

  VW::string_view example(line, num_chars);
  substring_to_example(all, examples[0], example);

  return (int)num_chars_initial;
}

template <bool audit>
class TC_parser
{
 public:
  VW::string_view _line;
  size_t _read_idx;
  float _cur_channel_v;
  bool _new_index;
  size_t _anon;
  uint64_t _channel_hash;
  VW::string_view _base;
  unsigned char _index;
  float _v;
  bool _redefine_some;
  std::array<unsigned char, NUM_NAMESPACES>* _redefine;
  parser* _p;
  example* _ae;
  std::array<uint64_t, NUM_NAMESPACES>* _affix_features;
  std::array<bool, NUM_NAMESPACES>* _spelling_features;
  v_array<char> _spelling;
  uint32_t _hash_seed;
  uint64_t _parse_mask;

  std::array<std::vector<std::shared_ptr<feature_dict>>, NUM_NAMESPACES>* _namespace_dictionaries;

  ~TC_parser() {}

  inline void parserWarning(const char* message, VW::string_view var_msg, const char* message2)
  {
    // VW::string_view will output the entire view into the output stream.
    // That means if there is a null character somewhere in the range, it will terminate
    // the stringstream at that point! Minor hack to give us the behavior we actually want here (i think)..
    // the alternative is to do what the old code was doing.. str(_line).c_str()...
    // TODO: Find a sane way to handle nulls in the middle of a string (either VW::string_view or substring)
    auto tmp_view = _line.substr(0, _line.find('\0'));
    std::stringstream ss;
    ss << message << var_msg << message2 << "in Example #" << this->_p->end_parsed_examples.load() << ": \"" << tmp_view << "\""
       << std::endl;
    if (_p->strict_parse)
    {
      THROW_EX(VW::strict_parse_exception, ss.str());
    }
    else
    {
      std::cerr << ss.str();
    }
  }

  inline float featureValue()
  {
    if (_read_idx >= _line.size() || _line[_read_idx] == ' ' || _line[_read_idx] == '\t' ||
        _line[_read_idx] == '|' || _line[_read_idx] == '\r')
      return 1.;
    else if (_line[_read_idx] == ':')
    {
      // featureValue --> ':' 'Float'
      ++_read_idx;
      size_t end_read = 0;
      VW::string_view sv = _line.substr(_read_idx);
      _v = parseFloat(sv.begin(), end_read, sv.end());
      if (end_read == 0)
      {
        parserWarning("malformed example! Float expected after : \"", _line.substr(0, _read_idx), "\"");
      }
      if (std::isnan(_v))
      {
        _v = 0.f;
        parserWarning(
            "warning: invalid feature value:\"", _line.substr(_read_idx), "\" read as NaN. Replacing with 0.");
      }
      _read_idx += end_read;
      return _v;
    }
    else
    {
      // syntax error
      parserWarning(
          "malformed example! '|', ':', space, or EOL expected after : \"", _line.substr(0, _read_idx), "\"");
      return 0.f;
    }
  }

  inline VW::string_view read_name()
  {
    size_t name_start = _read_idx;
    while (!(_read_idx >= _line.size() || _line[_read_idx] == ' ' || _line[_read_idx] == ':' ||
        _line[_read_idx] == '\t' || _line[_read_idx] == '|' || _line[_read_idx] == '\r'))
      ++_read_idx;

    return _line.substr(name_start, _read_idx - name_start);
  }

  inline void maybeFeature()
  {
    if (_read_idx >= _line.size() || _line[_read_idx] == ' ' || _line[_read_idx] == '\t' ||
        _line[_read_idx] == '|' || _line[_read_idx] == '\r')
    {
      // maybeFeature --> ø
    }
    else
    {
      // maybeFeature --> 'String' FeatureValue
      VW::string_view feature_name = read_name();
      _v = _cur_channel_v * featureValue();
      uint64_t word_hash;
      if (!feature_name.empty())
        word_hash = (_p->hasher(feature_name.begin(), feature_name.length(), _channel_hash) & _parse_mask);
      else
        word_hash = _channel_hash + _anon++;
      if (_v == 0)
        return;  // dont add 0 valued features to list of features
      features& fs = _ae->feature_space[_index];
      fs.push_back(_v, word_hash);
      if (audit)
      {
        fs.space_names.push_back(audit_strings_ptr(new audit_strings(_base.to_string(), feature_name.to_string())));
      }
      if (((*_affix_features)[_index] > 0) && (!feature_name.empty()))
      {
        features& affix_fs = _ae->feature_space[affix_namespace];
        if (affix_fs.size() == 0)
          _ae->indices.push_back(affix_namespace);
        uint64_t affix = (*_affix_features)[_index];

        while (affix > 0)
        {
          bool is_prefix = affix & 0x1;
          uint64_t len = (affix >> 1) & 0x7;
          VW::string_view affix_name(feature_name);
          if (affix_name.size() > len)
          {
            if (is_prefix)
              affix_name.remove_suffix(affix_name.size() - len);
            else
              affix_name.remove_prefix(affix_name.size() - len);
          }

          word_hash =
              _p->hasher(affix_name.begin(), affix_name.length(), (uint64_t)_channel_hash) * (affix_constant + (affix & 0xF) * quadratic_constant);
          affix_fs.push_back(_v, word_hash);
          if (audit)
          {
            v_array<char> affix_v = v_init<char>();
            if (_index != ' ')
              affix_v.push_back(_index);
            affix_v.push_back(is_prefix ? '+' : '-');
            affix_v.push_back('0' + (char)len);
            affix_v.push_back('=');
            push_many(affix_v, affix_name.begin(), affix_name.size());
            affix_v.push_back('\0');
            affix_fs.space_names.push_back(audit_strings_ptr(new audit_strings("affix", affix_v.begin())));
          }
          affix >>= 4;
        }
      }
      if ((*_spelling_features)[_index])
      {
        features& spell_fs = _ae->feature_space[spelling_namespace];
        if (spell_fs.size() == 0)
          _ae->indices.push_back(spelling_namespace);
        // v_array<char> spelling;
        _spelling.clear();
        for (char c : feature_name)
        {
          char d = 0;
          if ((c >= '0') && (c <= '9'))
            d = '0';
          else if ((c >= 'a') && (c <= 'z'))
            d = 'a';
          else if ((c >= 'A') && (c <= 'Z'))
            d = 'A';
          else if (c == '.')
            d = '.';
          else
            d = '#';
          // if ((spelling.size() == 0) || (spelling.last() != d))
          _spelling.push_back(d);
        }

        VW::string_view spelling_strview(_spelling.begin(), _spelling.size());
        uint64_t word_hash = hashstring(spelling_strview.begin(), spelling_strview.length(), (uint64_t)_channel_hash);
        spell_fs.push_back(_v, word_hash);
        if (audit)
        {
          v_array<char> spelling_v = v_init<char>();
          if (_index != ' ')
          {
            spelling_v.push_back(_index);
            spelling_v.push_back('_');
          }
          push_many(spelling_v, spelling_strview.begin(), spelling_strview.size());
          spelling_v.push_back('\0');
          spell_fs.space_names.push_back(audit_strings_ptr(new audit_strings("spelling", spelling_v.begin())));
        }
      }
      if ((*_namespace_dictionaries)[_index].size() > 0)
      {
        // Heterogeneous lookup not yet implemented in std
        // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0919r0.html
        const std::string feature_name_str(feature_name);
        for (const auto& map : (*_namespace_dictionaries)[_index])
        {
          const auto& feats_it = map->find(feature_name_str);
          if ((feats_it != map->end()) && (feats_it->second->values.size() > 0))
          {
            const auto& feats = feats_it->second;
            features& dict_fs = _ae->feature_space[dictionary_namespace];
            if (dict_fs.size() == 0)
              _ae->indices.push_back(dictionary_namespace);
            push_many(dict_fs.values, feats->values.begin(), feats->values.size());
            push_many(dict_fs.indicies, feats->indicies.begin(), feats->indicies.size());
            dict_fs.sum_feat_sq += feats->sum_feat_sq;
            if (audit)
              for (const auto& id : feats->indicies)
              {
		std::stringstream ss;
                ss << _index << '_';
                ss << feature_name;
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
    if (_read_idx >= _line.size() || _line[_read_idx] == ' ' || _line[_read_idx] == '\t' ||
        _line[_read_idx] == '|' || _line[_read_idx] == '\r')
    {
      // nameSpaceInfoValue -->  ø
    }
    else if (_line[_read_idx] == ':')
    {
      // nameSpaceInfoValue --> ':' 'Float'
      ++_read_idx;
      size_t end_read = 0;
      VW::string_view sv = _line.substr(_read_idx);
      _cur_channel_v = parseFloat(sv.begin(), end_read, sv.end());
      if (end_read + _read_idx >= _line.size())
      {
        parserWarning("malformed example! Float expected after : \"", _line.substr(0, _read_idx), "\"");
      }
      if (std::isnan(_cur_channel_v))
      {
        _cur_channel_v = 1.f;
        parserWarning(
            "warning: invalid namespace value:\"", _line.substr(_read_idx), "\" read as NaN. Replacing with 1.");
      }
      _read_idx += end_read;
    }
    else
    {
      // syntax error
      parserWarning(
          "malformed example! '|',':', space, or EOL expected after : \"", _line.substr(0, _read_idx), "\"");
    }
  }

  inline void nameSpaceInfo()
  {
    if (_read_idx >= _line.size() || _line[_read_idx] == '|' || _line[_read_idx] == ' ' ||
        _line[_read_idx] == '\t' || _line[_read_idx] == ':' || _line[_read_idx] == '\r')
    {
      // syntax error
      parserWarning("malformed example! String expected after : \"", _line.substr(0, _read_idx), "\"");
    }
    else
    {
      // NameSpaceInfo --> 'String' NameSpaceInfoValue
      _index = (unsigned char)(_line[_read_idx]);
      if (_redefine_some)
        _index = (*_redefine)[_index];  // redefine _index
      if (_ae->feature_space[_index].size() == 0)
        _new_index = true;
      VW::string_view name = read_name();
      if (audit)
      {
        _base = name;
      }
      _channel_hash = _p->hasher(name.begin(), name.length(), this->_hash_seed);
      nameSpaceInfoValue();
    }
  }

  inline void listFeatures()
  {
    while ((_read_idx < _line.size()) && (_line[_read_idx] == ' ' || _line[_read_idx] == '\t'))
    {
      // listFeatures --> ' ' MaybeFeature ListFeatures
      ++_read_idx;
      maybeFeature();
    }
    if (!(_read_idx >= _line.size() || _line[_read_idx] == '|' || _line[_read_idx] == '\r'))
    {
      // syntax error
      parserWarning("malformed example! '|',space, or EOL expected after : \"", _line.substr(0, _read_idx), "\"");
    }
  }

  inline void nameSpace()
  {
    _cur_channel_v = 1.0;
    _index = 0;
    _new_index = false;
    _anon = 0;
    if (_read_idx >= _line.size() || _line[_read_idx] == ' ' || _line[_read_idx] == '\t' ||
        _line[_read_idx] == '|' || _line[_read_idx] == '\r')
    {
      // NameSpace --> ListFeatures
      _index = (unsigned char)' ';
      if (_ae->feature_space[_index].size() == 0)
        _new_index = true;
      if (audit)
      {
        // TODO: c++17 allows VW::string_view literals, eg: " "sv
        static const char* space = " ";
        _base = space;
      }
      _channel_hash = this->_hash_seed == 0 ? 0 : uniform_hash("", 0, this->_hash_seed);
      listFeatures();
    }
    else if (_line[_read_idx] != ':')
    {
      // NameSpace --> NameSpaceInfo ListFeatures
      nameSpaceInfo();
      listFeatures();
    }
    else
    {
      // syntax error
      parserWarning(
          "malformed example! '|',String,space, or EOL expected after : \"", _line.substr(0, _read_idx), "\"");
    }
    if (_new_index && _ae->feature_space[_index].size() > 0)
      _ae->indices.push_back(_index);
  }

  inline void listNameSpace()
  {
    while (
        (_read_idx < _line.size()) && (_line[_read_idx] == '|'))  // ListNameSpace --> '|' NameSpace ListNameSpace
    {
      ++_read_idx;
      nameSpace();
    }
    if (_read_idx < _line.size() && _line[_read_idx] != '\r')
    {
      // syntax error
      parserWarning("malformed example! '|' or EOL expected after : \"", _line.substr(0, _read_idx), "\"");
    }
  }

  TC_parser(VW::string_view line, vw& all, example* ae) : _line(line)
  {
    _spelling = v_init<char>();
    if (!_line.empty())
    {
      this->_read_idx = 0;
      this->_p = all.p;
      this->_redefine_some = all.redefine_some;
      this->_redefine = &all.redefine;
      this->_ae = ae;
      this->_affix_features = &all.affix_features;
      this->_spelling_features = &all.spelling_features;
      this->_namespace_dictionaries = &all.namespace_dictionaries;
      this->_hash_seed = all.hash_seed;
      this->_parse_mask = all.parse_mask;

      listNameSpace();
    }
  }
};

void substring_to_example(vw* all, example* ae, VW::string_view example)
{
  all->p->lp.default_label(&ae->l);

  size_t bar_idx = example.find('|');

  all->p->words.clear();
  if (bar_idx != 0)
  {
    VW::string_view label_space(example);
    if (bar_idx != VW::string_view::npos)
    {
      // a little bit iffy since bar_idx is based on example and we're working off label_space
      // but safe as long as this is the first manipulation after the copy
      label_space.remove_suffix(label_space.size() - bar_idx);
    }
    size_t tab_idx = label_space.find('\t');
    if (tab_idx != VW::string_view::npos)
    {
      label_space.remove_prefix(tab_idx + 1);
    }

    std::vector<VW::string_view> tokenized;
    tokenize(' ', label_space, all->p->words);
    if (all->p->words.size() > 0 &&
        (all->p->words.last().end() == label_space.end() ||
        all->p->words.last().front() == '\''))  // The last field is a tag, so record and strip it off
    {
      VW::string_view tag = all->p->words.pop();
      if (tag.front() == '\'')
        tag.remove_prefix(1);
      push_many(ae->tag, tag.begin(), tag.size());
    }
  }

  if (!all->p->words.empty())
    all->p->lp.parse_label(all->p, all->p->_shared_data, &ae->l, all->p->words);

  if (bar_idx != VW::string_view::npos)
  {
    if (all->audit || all->hash_inv)
      TC_parser<true> parser_line(example.substr(bar_idx), *all, ae);
    else
      TC_parser<false> parser_line(example.substr(bar_idx), *all, ae);
  }
}

namespace VW
{
void read_line(vw& all, example* ex, VW::string_view line)
{
  while (line.size() > 0 && line.back() == '\n') line.remove_suffix(1);
  substring_to_example(&all, ex, line);
}

void read_line(vw& all, example* ex, char* line) { return read_line(all, ex, VW::string_view(line)); }

void read_lines(vw* all, char* line, size_t /*len*/, v_array<example*>& examples)
{
  std::vector<VW::string_view> lines;
  tokenize('\n', line, lines);
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
