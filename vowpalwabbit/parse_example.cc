// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cmath>
#include <cctype>
#include "parse_example.h"
#include "parse_primitives.h"
#include "hash.h"
#include "unique_sort.h"
#include "global_data.h"
#include "constant.h"
#include "vw_string_view.h"
#include "future_compat.h"

#include "io/logger.h"

namespace logger = VW::io::logger;

size_t read_features(vw* all, char*& line, size_t& num_chars)
{
  line = nullptr;
  size_t num_chars_initial = all->example_parser->input->readto(line, '\n');
  if (num_chars_initial < 1) { return num_chars_initial; }
  num_chars = num_chars_initial;
  if (line[0] == '\xef' && num_chars >= 3 && line[1] == '\xbb' && line[2] == '\xbf')
  {
    line += 3;
    num_chars -= 3;
  }
  if (num_chars > 0 && line[num_chars - 1] == '\n') num_chars--;
  if (num_chars > 0 && line[num_chars - 1] == '\r') num_chars--;
  return num_chars_initial;
}

int read_features_string(vw* all, v_array<example*>& examples)
{
  char* line;
  size_t num_chars;
  size_t num_chars_initial = read_features(all, line, num_chars);
  if (num_chars_initial < 1)
  {
    examples[0]->is_newline = true;
    return static_cast<int>(num_chars_initial);
  }

  VW::string_view example(line, num_chars);
  substring_to_example(all, examples[0], example);

  return static_cast<int>(num_chars_initial);
}

class feature_group_helper
{
public:
  feature_group_helper(
      example* current_example,
      bool redefine_char_flag,
      std::array<unsigned char,NUM_NAMESPACES>* redefine_char_map,
      hash_func_t hash_fn
  ):
    _feature_group_char(0),
    _hash(0),
    _p_current_feature_group(nullptr),
    _p_affix_feature_group(nullptr),
    _p_spelling_feature_group(nullptr),
    _p_dictionary_feature_group(nullptr),
    _current_example(current_example),
    _redefine_char_flag(redefine_char_flag),
    _redefine_char_map(redefine_char_map),
    _hash_function(hash_fn)
  {}
  inline unsigned char feature_group_char() const
  {
    return _feature_group_char;
  }
  inline uint64_t feature_group_hash() const
  {
    return _hash;
  }
  inline features& get_current_fg()
  {
    return get_feature_group(_p_current_feature_group,feature_group_char());
  }
  inline features& get_affix_fg()
  {
    return get_feature_group(_p_affix_feature_group,affix_namespace);
  }
  inline features& get_spelling_fg()
  {
    return get_feature_group(_p_spelling_feature_group,spelling_namespace);
  }
  inline features& get_dictionary_fg()
  {
    return get_feature_group(_p_dictionary_feature_group,dictionary_namespace);
  }
  inline void set_current_fg(const VW::string_view& fg_name)
  {
    _feature_group_name = fg_name;
    auto namespace_char = static_cast<unsigned char>(_feature_group_name[0]);
    if (_redefine_char_flag) namespace_char = (*_redefine_char_map)[namespace_char];  // redefine feature_group_char
    _feature_group_char = namespace_char;
    _hash = _hash_function(_feature_group_name.begin(), _feature_group_name.length(), _hash_seed);
    _p_current_feature_group = nullptr;  // reset cached feature group
  }
  inline void set_current_fg()
  {
    _feature_group_name = "";
    _feature_group_char = ' ';
    _hash = this->_hash_seed == 0 ? 0 : uniform_hash("", 0, this->_hash_seed);
    _p_current_feature_group = nullptr;  // reset cached feature group
  }

private:
  inline features& get_feature_group(features*& p_features_ref, unsigned char namespace_char)
  {
    if(p_features_ref == nullptr)
    {
      p_features_ref = & _current_example->feature_space[namespace_char];

      if(p_features_ref->empty())
        _current_example->indices.emplace_back(namespace_char);
    }
    return {*p_features_ref};    
  }

  unsigned char _feature_group_char;
  int64_t _hash;
  features* _p_current_feature_group;
  features* _p_affix_feature_group;
  features* _p_spelling_feature_group;
  features* _p_dictionary_feature_group;
  example* _current_example;
  bool _redefine_char_flag;
  std::array<unsigned char, NUM_NAMESPACES>* _redefine_char_map;
  hash_func_t _hash_function;
  uint32_t _hash_seed;
  VW::string_view _feature_group_name;
};

template <bool audit>
class TC_parser
{
public:
  VW::string_view _line;
  size_t _read_idx;
  float _cur_channel_v;
  size_t _anon;
  float _v;
  parser* _p;
  std::array<uint64_t, NUM_NAMESPACES>* _affix_features;
  std::array<bool, NUM_NAMESPACES>* _spelling_features;
  v_array<char> _spelling;
  uint64_t _parse_mask;
  std::array<std::vector<std::shared_ptr<feature_dict>>, NUM_NAMESPACES>* _namespace_dictionaries;
  feature_group_helper _feature_group_helper;

  ~TC_parser() {}

  // TODO: Currently this function is called by both warning and error conditions. We only log
  //      to warning here though.
  inline FORCE_INLINE void parserWarning(const char* message, VW::string_view var_msg, const char* message2)
  {
    // VW::string_view will output the entire view into the output stream.
    // That means if there is a null character somewhere in the range, it will terminate
    // the stringstream at that point! Minor hack to give us the behavior we actually want here (i think)..
    // the alternative is to do what the old code was doing.. str(_line).c_str()...
    // TODO: Find a sane way to handle nulls in the middle of a string (either VW::string_view or substring)
    auto tmp_view = _line.substr(0, _line.find('\0'));
    std::stringstream ss;
    ss << message << var_msg << message2 << "in Example #" << this->_p->end_parsed_examples.load() << ": \"" << tmp_view
       << "\"";

    if (_p->strict_parse)
    {
      // maintain newline behavior
      ss << std::endl;
      THROW_EX(VW::strict_parse_exception, ss.str());
    }
    else
    {
      logger::errlog_warn(ss.str());
    }
  }

  inline FORCE_INLINE VW::string_view stringFeatureValue(VW::string_view sv)
  {
    size_t start_idx = sv.find_first_not_of(" \t\r\n");
    if (start_idx > 0 && start_idx != std::string::npos)
    {
      _read_idx += start_idx;
      sv.remove_prefix(start_idx);
    }

    size_t end_idx = sv.find_first_of(" \t\r\n");
    if (end_idx == std::string::npos) { end_idx = sv.size(); }
    _read_idx += end_idx;
    return sv.substr(0, end_idx);
  }

  inline FORCE_INLINE bool isFeatureValueFloat(float& float_feature_value)
  {
    if (_read_idx >= _line.size() || _line[_read_idx] == ' ' || _line[_read_idx] == '\t' || _line[_read_idx] == '|' ||
        _line[_read_idx] == '\r')
    {
      float_feature_value = 1.;
      return true;
    }

    else if (_line[_read_idx] == ':')
    {
      // featureValue --> ':' 'Float'
      ++_read_idx;
      size_t end_read = 0;
      VW::string_view sv = _line.substr(_read_idx);
      _v = float_feature_value = parseFloat(sv.begin(), end_read, sv.end());
      if (end_read == 0) { return false; }
      if (std::isnan(_v))
      {
        _v = float_feature_value = 0.f;
        parserWarning(
            "warning: invalid feature value:\"", _line.substr(_read_idx), "\" read as NaN. Replacing with 0.");
      }
      _read_idx += end_read;
      return true;
    }
    else
    {
      _v = float_feature_value = 0.f;
      // syntax error
      parserWarning("malformed example! '|', ':', space, or EOL expected after : \"", _line.substr(0, _read_idx), "\"");
      return true;
    }
  }

  inline FORCE_INLINE VW::string_view read_name()
  {
    size_t name_start = _read_idx;
    while (!(_read_idx >= _line.size() || _line[_read_idx] == ' ' || _line[_read_idx] == ':' ||
        _line[_read_idx] == '\t' || _line[_read_idx] == '|' || _line[_read_idx] == '\r'))
      ++_read_idx;

    return _line.substr(name_start, _read_idx - name_start);
  }

  inline FORCE_INLINE void maybeFeature()
  {
    if (_read_idx >= _line.size() || _line[_read_idx] == ' ' || _line[_read_idx] == '\t' || _line[_read_idx] == '|' ||
        _line[_read_idx] == '\r')
    {
      // maybeFeature --> ø
    }
    else
    {
      // maybeFeature --> 'String' FeatureValue
      VW::string_view feature_name = read_name();
      VW::string_view string_feature_value;

      float float_feature_value = 0.f;
      bool is_feature_float = isFeatureValueFloat(float_feature_value);

      if (!is_feature_float)
      {
        string_feature_value = stringFeatureValue(_line.substr(_read_idx));
        _v = 1;
      }
      else
      {
        _v = _cur_channel_v * float_feature_value;
      }

      uint64_t word_hash;
      // Case where string:string or :string
      if (!string_feature_value.empty())
      {
        // chain hash is hash(feature_value, hash(feature_name, namespace_hash)) & parse_mask
        word_hash = (_p->hasher(string_feature_value.begin(), string_feature_value.length(),
                         _p->hasher(feature_name.begin(), feature_name.length(), _feature_group_helper.feature_group_hash())) &
            _parse_mask);
      }
      // Case where string:float
      else if (!feature_name.empty())
      {
        word_hash = (_p->hasher(feature_name.begin(), feature_name.length(), _feature_group_helper.feature_group_hash()) & _parse_mask);
      }
      // Case where :float
      else
      {
        word_hash = _feature_group_helper.feature_group_hash() + _anon++;
      }

      if (_v == 0) return;  // dont add 0 valued features to list of features
      features& fs = _feature_group_helper.get_current_fg();
      fs.push_back(_v, word_hash);

      if (audit)
      {
        if (!string_feature_value.empty())
        {
          std::stringstream ss;
          ss << feature_name << "^" << string_feature_value;
          fs.space_names.push_back(audit_strings(_base.to_string(), ss.str()));
        }
        else
        {
          fs.space_names.push_back(audit_strings(_base.to_string(), feature_name.to_string()));
        }
      }

      if (((*_affix_features)[_feature_group_helper.feature_group_char()] > 0) && (!feature_name.empty()))
      {
        features& affix_fs = _feature_group_helper.get_affix_fg();
        uint64_t affix = (*_affix_features)[_feature_group_helper.feature_group_char()];

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

          word_hash = _p->hasher(affix_name.begin(), affix_name.length(), _feature_group_helper.feature_group_hash()) *
              (affix_constant + (affix & 0xF) * quadratic_constant);
          affix_fs.push_back(_v, word_hash);
          if (audit)
          {
            v_array<char> affix_v;
            if (_feature_group_helper.feature_group_char() != ' ') affix_v.push_back(_feature_group_helper.feature_group_char());
            affix_v.push_back(is_prefix ? '+' : '-');
            affix_v.push_back('0' + static_cast<char>(len));
            affix_v.push_back('=');
            affix_v.insert(affix_v.end(), affix_name.begin(), affix_name.end());
            affix_v.push_back('\0');
            affix_fs.space_names.push_back(audit_strings("affix", affix_v.begin()));
          }
          affix >>= 4;
        }
      }
      if ((*_spelling_features)[_feature_group_helper.feature_group_char()])
      {
        features& spell_fs = _feature_group_helper.get_spelling_fg();
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
        word_hash = hashstring(spelling_strview.begin(), spelling_strview.length(), _feature_group_helper.feature_group_hash());
        spell_fs.push_back(_v, word_hash);
        if (audit)
        {
          v_array<char> spelling_v;
          if (_feature_group_helper.feature_group_char() != ' ')
          {
            spelling_v.push_back(_feature_group_helper.feature_group_char());
            spelling_v.push_back('_');
          }
          spelling_v.insert(spelling_v.end(), spelling_strview.begin(), spelling_strview.end());
          spelling_v.push_back('\0');
          spell_fs.space_names.push_back(audit_strings("spelling", spelling_v.begin()));
        }
      }
      if ((*_namespace_dictionaries)[_feature_group_helper.feature_group_char()].size() > 0)
      {
        // Heterogeneous lookup not yet implemented in std
        // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0919r0.html
        const std::string feature_name_str(feature_name);
        for (const auto& map : (*_namespace_dictionaries)[_feature_group_helper.feature_group_char()])
        {
          const auto& feats_it = map->find(feature_name_str);
          if ((feats_it != map->end()) && (feats_it->second->values.size() > 0))
          {
            const auto& feats = feats_it->second;
            features& dict_fs = _feature_group_helper.get_dictionary_fg();
            dict_fs.values.insert(dict_fs.values.end(), feats->values.begin(), feats->values.end());
            dict_fs.indicies.insert(dict_fs.indicies.end(), feats->indicies.begin(), feats->indicies.end());
            dict_fs.sum_feat_sq += feats->sum_feat_sq;
            if (audit)
              for (const auto& id : feats->indicies)
              {
                std::stringstream ss;
                ss << _feature_group_helper.feature_group_char() << '_';
                ss << feature_name;
                ss << '=' << id;
                dict_fs.space_names.push_back(audit_strings("dictionary", ss.str()));
              }
          }
        }
      }
    }
  }

  inline FORCE_INLINE void nameSpaceInfoValue()
  {
    if (_read_idx >= _line.size() || _line[_read_idx] == ' ' || _line[_read_idx] == '\t' || _line[_read_idx] == '|' ||
        _line[_read_idx] == '\r')
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
      { parserWarning("malformed example! Float expected after : \"", _line.substr(0, _read_idx), "\""); }
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
      parserWarning("malformed example! '|',':', space, or EOL expected after : \"", _line.substr(0, _read_idx), "\"");
    }
  }

  inline FORCE_INLINE void nameSpaceInfo()
  {
    if (_read_idx >= _line.size() || _line[_read_idx] == '|' || _line[_read_idx] == ' ' || _line[_read_idx] == '\t' ||
        _line[_read_idx] == ':' || _line[_read_idx] == '\r')
    {
      // syntax error
      parserWarning("malformed example! String expected after : \"", _line.substr(0, _read_idx), "\"");
    }
    else
    {
      // NameSpaceInfo --> 'String' NameSpaceInfoValue
      const VW::string_view name = read_name();
      _feature_group_helper.set_current_fg(name);
      if (audit) { _base = name; }
      nameSpaceInfoValue();
    }
  }

  inline FORCE_INLINE void listFeatures()
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

  inline FORCE_INLINE void nameSpace()
  {
    _cur_channel_v = 1.0;
    _anon = 0;
    if (_read_idx >= _line.size() || _line[_read_idx] == ' ' || _line[_read_idx] == '\t' || _line[_read_idx] == '|' ||
        _line[_read_idx] == '\r')
    {
      // NameSpace --> ListFeatures
      _feature_group_helper.set_current_fg();
      if (audit)
      {
        // TODO: c++17 allows VW::string_view literals, eg: " "sv
        static const char* space = " ";
        _base = space;
      }
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
  }

  inline FORCE_INLINE void listNameSpace()
  {
    while ((_read_idx < _line.size()) && (_line[_read_idx] == '|'))  // ListNameSpace --> '|' NameSpace ListNameSpace
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

  TC_parser(VW::string_view line, vw& all, example* ae)
    : _line(line),
    _feature_group_helper(ae, all.redefine_some, &all.redefine, all.example_parser->hasher)
  {
    if (!_line.empty())
    {
      this->_read_idx = 0;
      this->_p = all.example_parser;
      this->_affix_features = &all.affix_features;
      this->_spelling_features = &all.spelling_features;
      this->_namespace_dictionaries = &all.namespace_dictionaries;
      this->_parse_mask = all.parse_mask;
      listNameSpace();
    }
    else
    {
      ae->is_newline = true;
    }
  }
};

void substring_to_example(vw* all, example* ae, VW::string_view example)
{
  if (example.empty()) { ae->is_newline = true; }

  all->example_parser->lbl_parser.default_label(&ae->l);

  size_t bar_idx = example.find('|');

  all->example_parser->words.clear();
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
    if (tab_idx != VW::string_view::npos) { label_space.remove_prefix(tab_idx + 1); }

    tokenize(' ', label_space, all->example_parser->words);
    if (all->example_parser->words.size() > 0 &&
        (all->example_parser->words.back().end() == label_space.end() ||
            all->example_parser->words.back().front() == '\''))  // The last field is a tag, so record and strip it off
    {
      VW::string_view tag = all->example_parser->words.back();
      all->example_parser->words.pop_back();
      if (tag.front() == '\'') { tag.remove_prefix(1); }
      ae->tag.insert(ae->tag.end(), tag.begin(), tag.end());
    }
  }

  if (!all->example_parser->words.empty())
    all->example_parser->lbl_parser.parse_label(all->example_parser, all->example_parser->_shared_data, &ae->l,
        all->example_parser->words, ae->_reduction_features);

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

void read_line(vw& all, example* ex, const char* line) { return read_line(all, ex, VW::string_view(line)); }

void read_lines(vw* all, const char* line, size_t /*len*/, v_array<example*>& examples)
{
  std::vector<VW::string_view> lines;
  tokenize('\n', line, lines);
  for (size_t i = 0; i < lines.size(); i++)
  {
    // Check if a new empty example needs to be added.
    if (examples.size() < i + 1) { examples.push_back(&VW::get_unused_example(all)); }
    read_line(*all, examples[i], lines[i]);
  }
}

}  // namespace VW
