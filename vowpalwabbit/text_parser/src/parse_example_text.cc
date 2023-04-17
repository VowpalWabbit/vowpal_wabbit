// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/text_parser/parse_example_text.h"

#include "vw/common/future_compat.h"
#include "vw/common/hash.h"
#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/core/constant.h"
#include "vw/core/global_data.h"
#include "vw/core/hashstring.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"
#include "vw/core/shared_data.h"
#include "vw/core/unique_sort.h"
#include "vw/io/logger.h"

#include <fmt/format.h>

#include <cctype>
#include <cmath>

namespace
{
template <bool audit>
class tc_parser
{
private:
  // Per call state
  VW::string_view _line;
  size_t _read_idx{};
  float _cur_channel_v{};
  bool _new_index{};
  size_t _anon{};
  uint64_t _channel_hash{};
  VW::string_view _base;
  unsigned char _index{};
  VW::example* _ae = nullptr;

  // config
  VW::hash_func_t _hasher;
  bool _strict_parse;
  uint32_t _hash_seed;
  uint64_t _parse_mask;
  VW::io::logger* _logger;
  // Optional
  bool _redefine_some;
  std::array<unsigned char, VW::NUM_NAMESPACES>* _redefine;
  std::array<uint64_t, VW::NUM_NAMESPACES>* _affix_features;
  std::array<bool, VW::NUM_NAMESPACES>* _spelling_features;
  std::array<std::vector<std::shared_ptr<VW::details::feature_dict>>, VW::NUM_NAMESPACES>* _namespace_dictionaries;

  VW_ATTR(nodiscard) inline FORCE_INLINE bool current_is_whitespace() const
  {
    assert(_read_idx < _line.size());
    return _line[_read_idx] == ' ' || _line[_read_idx] == '\t';
  }

  VW_ATTR(nodiscard) inline FORCE_INLINE bool current_is_colon() const
  {
    assert(_read_idx < _line.size());
    return _line[_read_idx] == ':';
  }

  VW_ATTR(nodiscard) inline FORCE_INLINE bool current_is_pipe() const
  {
    assert(_read_idx < _line.size());
    return _line[_read_idx] == '|';
  }

  VW_ATTR(nodiscard) inline FORCE_INLINE bool current_is_carriage_return() const
  {
    assert(_read_idx < _line.size());
    return _line[_read_idx] == '\r';
  }

  // If reached end of string
  VW_ATTR(nodiscard) inline FORCE_INLINE bool current_is_eol() const { return _read_idx >= _line.size(); }

  std::string format_parser_diagnostic_message(VW::string_view message, size_t example_number)
  {
    return fmt::format("[Example #{}] {}\n\n{}\n{:<{}}", example_number, message, _line, "^", _read_idx);
  }

  void parser_warning(VW::string_view message, size_t example_number)
  {
    auto formatted = format_parser_diagnostic_message(message, example_number);
    if (_strict_parse) { THROW_EX(VW::strict_parse_exception, formatted); }
    else { _logger->warn("{}", formatted); }
  }

  void parser_error(VW::string_view message, size_t example_number)
  {
    auto formatted = format_parser_diagnostic_message(message, example_number);
    if (_strict_parse) { THROW_EX(VW::strict_parse_exception, formatted); }
    else { _logger->error("{}", formatted); }
  }

  void process_spelling(VW::example* ex, VW::string_view feature_name, VW::string_view string_feature_value,
      float float_feature_value) const
  {
    if (_spelling_features == nullptr) { return; }

    if ((*_spelling_features)[_index])
    {
      if (!string_feature_value.empty())
      {
        assert(_logger != nullptr);
        _logger->warn(
            "Spelling features are not supported for string valued features (string after :). Ignoring value portion "
            "of feature.");
      }
      auto& spell_fs = ex->feature_space[VW::details::SPELLING_NAMESPACE];
      if (spell_fs.empty()) { ex->indices.push_back(VW::details::SPELLING_NAMESPACE); }
      // v_array<char> spelling;
      std::vector<char> spelling;
      spelling.reserve(feature_name.size());
      for (char c : feature_name)
      {
        char d = 0;
        if ((c >= '0') && (c <= '9')) { d = '0'; }
        else if ((c >= 'a') && (c <= 'z')) { d = 'a'; }
        else if ((c >= 'A') && (c <= 'Z')) { d = 'A'; }
        else if (c == '.') { d = '.'; }
        else { d = '#'; }
        // if ((spelling.size() == 0) || (spelling.last() != d))
        spelling.push_back(d);
      }

      VW::string_view spelling_strview(spelling.data(), spelling.size());
      auto word_hash =
          VW::details::hashstring(spelling_strview.data(), spelling_strview.length(), (uint64_t)_channel_hash);
      spell_fs.push_back(float_feature_value, word_hash, VW::details::SPELLING_NAMESPACE);
      if (audit)
      {
        VW::v_array<char> spelling_v;
        if (_index != ' ')
        {
          spelling_v.push_back(_index);
          spelling_v.push_back('_');
        }
        spelling_v.insert(spelling_v.end(), spelling_strview.begin(), spelling_strview.end());
        spelling_v.push_back('\0');
        spell_fs.space_names.emplace_back("spelling", spelling_v.begin());
      }
    }
  }

  void process_affix(VW::example* ex, VW::string_view feature_name, VW::string_view string_feature_value,
      float float_feature_value) const
  {
    if (_affix_features == nullptr) { return; }

    if (((*_affix_features)[_index] > 0) && (!feature_name.empty()))
    {
      if (!string_feature_value.empty())
      {
        assert(_logger != nullptr);
        _logger->warn(
            "Affix features are not supported for string valued features (string after :). Ignoring value portion of "
            "feature.");
      }
      auto& affix_fs = ex->feature_space[VW::details::AFFIX_NAMESPACE];
      if (affix_fs.size() == 0) { ex->indices.push_back(VW::details::AFFIX_NAMESPACE); }
      uint64_t affix = (*_affix_features)[_index];

      while (affix > 0)
      {
        bool is_prefix = affix & 0x1;
        uint64_t len = (affix >> 1) & 0x7;
        VW::string_view affix_name(feature_name);
        if (affix_name.size() > len)
        {
          if (is_prefix) { affix_name.remove_suffix(affix_name.size() - len); }
          else { affix_name.remove_prefix(affix_name.size() - len); }
        }

        auto word_hash = _hasher(affix_name.data(), affix_name.length(), (uint64_t)_channel_hash) *
            (VW::details::AFFIX_CONSTANT + (affix & 0xF) * VW::details::QUADRATIC_CONSTANT);
        affix_fs.push_back(float_feature_value, word_hash, VW::details::AFFIX_NAMESPACE);
        if (audit)
        {
          VW::v_array<char> affix_v;
          if (_index != ' ') { affix_v.push_back(_index); }
          affix_v.push_back(is_prefix ? '+' : '-');
          affix_v.push_back('0' + static_cast<char>(len));
          affix_v.push_back('=');
          affix_v.insert(affix_v.end(), affix_name.begin(), affix_name.end());
          affix_v.push_back('\0');
          affix_fs.space_names.emplace_back("affix", affix_v.begin());
        }
        affix >>= 4;
      }
    }
  }

  void process_dictionary(VW::example* ex, VW::string_view feature_name, VW::string_view string_feature_value) const
  {
    if (_namespace_dictionaries == nullptr) { return; }

    if (!(*_namespace_dictionaries)[_index].empty())
    {
      if (!string_feature_value.empty())
      {
        assert(_logger != nullptr);
        _logger->warn(
            "Dictionary features are not supported for string valued features (string after :). Ignoring value portion "
            "of feature.");
      }
      // Heterogeneous lookup not yet implemented in std
      // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0919r0.html
      const std::string feature_name_str(feature_name);
      for (const auto& map : (*_namespace_dictionaries)[_index])
      {
        const auto& feats_it = map->find(feature_name_str);
        if ((feats_it != map->end()) && (!feats_it->second->values.empty()))
        {
          const auto& feats = feats_it->second;
          auto& dict_fs = ex->feature_space[VW::details::DICTIONARY_NAMESPACE];
          if (dict_fs.empty()) { ex->indices.push_back(VW::details::DICTIONARY_NAMESPACE); }
          dict_fs.start_ns_extent(VW::details::DICTIONARY_NAMESPACE);
          dict_fs.values.insert(dict_fs.values.end(), feats->values.begin(), feats->values.end());
          dict_fs.indices.insert(dict_fs.indices.end(), feats->indices.begin(), feats->indices.end());
          dict_fs.sum_feat_sq += feats->sum_feat_sq;
          if (audit)
          {
            for (const auto& id : feats->indices)
            {
              std::stringstream ss;
              ss << _index << '_';
              ss << feature_name;
              ss << '=' << id;
              dict_fs.space_names.emplace_back("dictionary", ss.str());
            }
          }
          dict_fs.end_ns_extent();
        }
      }
    }
  }

  // True if float component consumed else false
  // Consumes ':' and float value if exists
  inline FORCE_INLINE bool is_feature_value_float(float& float_feature_value)
  {
    if (current_is_eol() || current_is_whitespace() || current_is_pipe() || current_is_carriage_return())
    {
      float_feature_value = 1.;
      return true;
    }

    if (current_is_colon())
    {
      // featureValue --> ':' 'Float'
      ++_read_idx;
      size_t end_read = 0;
      VW::string_view sv = _line.substr(_read_idx);
      float_feature_value = VW::details::parse_float(sv.data(), end_read, sv.data() + sv.size());
      if (end_read == 0) { return false; }
      if (std::isnan(float_feature_value))
      {
        float_feature_value = 0.f;
        parser_warning("Invalid feature value read as NaN. Replacing with 0.", _ae->example_counter);
      }
      _read_idx += end_read;
      return true;
    }

    float_feature_value = 0.f;
    // syntax error
    parser_error("malformed example! '|', ':', space, or EOL expected", _ae->example_counter);
    return true;
  }

  inline FORCE_INLINE VW::string_view read_name()
  {
    size_t name_start = _read_idx;
    while (!(current_is_eol() || current_is_whitespace() || current_is_colon() || current_is_pipe() ||
        current_is_carriage_return()))
    {
      ++_read_idx;
    }

    return _line.substr(name_start, _read_idx - name_start);
  }

  inline FORCE_INLINE void maybe_feature()
  {
    // maybeFeature --> ø
    if (current_is_eol() || current_is_whitespace() || current_is_pipe() || current_is_carriage_return()) { return; }

    // maybeFeature --> 'String' FeatureValue
    VW::string_view feature_name = read_name();
    VW::string_view str_feat_value;

    float float_feature_value = 0.f;
    bool is_feature_float = is_feature_value_float(float_feature_value);

    if (!is_feature_float)
    {
      str_feat_value = read_name();
      float_feature_value = 1;
    }
    else { float_feature_value = _cur_channel_v * float_feature_value; }

    uint64_t word_hash;
    // Case where string:string or :string
    if (!str_feat_value.empty())
    {
      // chain hash is hash(feature_value, hash(feature_name, namespace_hash)) & parse_mask
      word_hash = (_hasher(str_feat_value.data(), str_feat_value.length(),
                       _hasher(feature_name.data(), feature_name.length(), _channel_hash)) &
          _parse_mask);
    }
    // Case where string:float
    else if (!feature_name.empty())
    {
      word_hash = (_hasher(feature_name.data(), feature_name.length(), _channel_hash) & _parse_mask);
    }
    // Case where :float
    else { word_hash = _channel_hash + _anon++; }

    if (float_feature_value == 0.f)
    {
      return;  // dont add 0 valued features to list of features
    }

    auto& fs = _ae->feature_space[_index];
    fs.push_back(float_feature_value, word_hash);

    if (audit)
    {
      if (!str_feat_value.empty())
      {
        fs.space_names.push_back(
            VW::audit_strings(std::string{_base}, std::string{feature_name}, std::string{str_feat_value}));
      }
      else { fs.space_names.push_back(VW::audit_strings(std::string{_base}, std::string{feature_name})); }
    }

    process_affix(_ae, feature_name, str_feat_value, float_feature_value);
    process_spelling(_ae, feature_name, str_feat_value, float_feature_value);
    process_dictionary(_ae, feature_name, str_feat_value);
  }

  inline FORCE_INLINE void name_space_info_value()
  {
    if (current_is_eol() || current_is_whitespace() || current_is_pipe() || current_is_carriage_return())
    {
      // nameSpaceInfoValue -->  ø
      return;
    }

    if (current_is_colon())
    {
      // nameSpaceInfoValue --> ':' 'Float'
      ++_read_idx;
      size_t end_read = 0;
      VW::string_view sv = _line.substr(_read_idx);
      _cur_channel_v = VW::details::parse_float(sv.data(), end_read, sv.data() + sv.size());
      if (end_read + _read_idx >= _line.size())
      {
        parser_error("Malformed example! Float expected.", _ae->example_counter);
      }
      if (std::isnan(_cur_channel_v))
      {
        _cur_channel_v = 1.f;
        parser_warning("Invalid namespace value read as NaN. Replacing with 1.", _ae->example_counter);
      }
      _read_idx += end_read;
    }

    // syntax error
    parser_error("Malformed example! '|',':', space, or EOL expected.", _ae->example_counter);
  }

  inline FORCE_INLINE void name_space_info()
  {
    if (current_is_eol() || current_is_pipe() || current_is_whitespace() || current_is_colon() ||
        current_is_carriage_return())
    {
      // syntax error
      parser_error("malformed example! String expected", _ae->example_counter);
      return;
    }

    // NameSpaceInfo --> 'String' NameSpaceInfoValue
    _index = (unsigned char)(_line[_read_idx]);
    if (_redefine_some)
    {
      assert(_redefine != nullptr);
      _index = (*_redefine)[_index];  // redefine _index
    }
    if (_ae->feature_space[_index].empty()) { _new_index = true; }
    VW::string_view name = read_name();
    if (audit) { _base = name; }
    _channel_hash = _hasher(name.data(), name.length(), this->_hash_seed);
    name_space_info_value();
  }

  inline FORCE_INLINE void list_features()
  {
    while (!current_is_eol() && current_is_whitespace())
    {
      // listFeatures --> ' ' MaybeFeature ListFeatures
      ++_read_idx;
      maybe_feature();
    }

    if (!(current_is_eol() || current_is_pipe() || current_is_carriage_return()))
    {
      // syntax error
      parser_error("malformed example! '|',space, or EOL expected", _ae->example_counter);
    }
  }

  inline FORCE_INLINE void name_space()
  {
    _cur_channel_v = 1.0;
    _index = 0;
    _new_index = false;
    _anon = 0;
    bool did_start_extent = false;
    if (current_is_eol() || current_is_whitespace() || current_is_pipe() || current_is_carriage_return())
    {
      // NameSpace --> ListFeatures
      _index = static_cast<unsigned char>(' ');
      if (_ae->feature_space[_index].empty()) { _new_index = true; }
      if (audit)
      {
        // TODO: c++17 allows VW::string_view literals, eg: " "sv
        static const char* space = " ";
        _base = space;
      }
      _channel_hash = this->_hash_seed == 0 ? 0 : VW::uniform_hash("", 0, this->_hash_seed);
      _ae->feature_space[_index].start_ns_extent(_channel_hash);
      did_start_extent = true;
      list_features();
    }
    else if (_line[_read_idx] != ':')
    {
      // NameSpace --> NameSpaceInfo ListFeatures
      name_space_info();
      _ae->feature_space[_index].start_ns_extent(_channel_hash);
      did_start_extent = true;
      list_features();
    }
    else
    {
      // syntax error
      parser_error(
          "Malformed example when reading namespace info. '|', string, space, or EOL expected", _ae->example_counter);
    }

    if (_new_index && !_ae->feature_space[_index].empty()) { _ae->indices.push_back(_index); }

    // If the namespace was empty this will handle it internally.
    if (did_start_extent) { _ae->feature_space[_index].end_ns_extent(); }
  }

  inline FORCE_INLINE void list_name_space()
  {
    while ((!current_is_eol()) && (current_is_pipe()))  // ListNameSpace --> '|' NameSpace ListNameSpace
    {
      ++_read_idx;
      name_space();
    }
    if (!current_is_eol() && !current_is_carriage_return())
    {
      // syntax error
      parser_error(
          "Unexpected character encountered when processing next namespace. '|' or EOL expected", _ae->example_counter);
    }
  }

public:
  tc_parser(VW::hash_func_t hasher, bool strict_parse, uint32_t hash_seed, uint64_t parse_mask, bool redefine_some,
      std::array<unsigned char, VW::NUM_NAMESPACES>* redefine, std::array<uint64_t, VW::NUM_NAMESPACES>* affix_features,
      std::array<bool, VW::NUM_NAMESPACES>* spelling_features,
      std::array<std::vector<std::shared_ptr<VW::details::feature_dict>>, VW::NUM_NAMESPACES>* namespace_dictionaries,
      VW::io::logger* logger)
      : _hasher(hasher)
      , _strict_parse(strict_parse)
      , _hash_seed(hash_seed)
      , _parse_mask(parse_mask)
      , _redefine_some(redefine_some)
      , _redefine(redefine)
      , _affix_features(affix_features)
      , _spelling_features(spelling_features)
      , _namespace_dictionaries(namespace_dictionaries)
      , _logger(logger)
  {
    if (_redefine_some && _redefine == nullptr) { THROW("redefine_some is true but redefine is nullptr."); }
  }

  tc_parser(VW::workspace& all)
      : tc_parser(all.parser_runtime.example_parser->hasher, all.parser_runtime.example_parser->strict_parse,
            all.runtime_config.hash_seed, all.runtime_state.parse_mask, all.feature_tweaks_config.redefine_some,
            &all.feature_tweaks_config.redefine, &all.feature_tweaks_config.affix_features,
            &all.feature_tweaks_config.spelling_features, &all.feature_tweaks_config.namespace_dictionaries,
            &all.logger)
  {
  }

  void parse_features(VW::example& ae, VW::string_view line)
  {
    if (line.empty())
    {
      assert(ae.is_newline && "example should be newline if line is empty");
      return;
    }

    _read_idx = 0;
    _cur_channel_v = 1.0;
    _index = 0;
    _new_index = false;
    _anon = 0;
    _channel_hash = 0;
    _base = "";
    _index = 0;

    _ae = &ae;
    _line = line;

    list_name_space();
  }
};
}  // namespace

void VW::parsers::text::details::substring_to_example(VW::workspace* all, VW::example* ae, VW::string_view example)
{
  if (example.empty()) { ae->is_newline = true; }

  all->parser_runtime.example_parser->lbl_parser.default_label(ae->l);

  size_t bar_idx = example.find('|');

  all->parser_runtime.example_parser->words.clear();
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

    VW::tokenize(' ', label_space, all->parser_runtime.example_parser->words);
    if (!all->parser_runtime.example_parser->words.empty() &&
        ((all->parser_runtime.example_parser->words.back().data() +
             all->parser_runtime.example_parser->words.back().size()) == (label_space.data() + label_space.size()) ||
            all->parser_runtime.example_parser->words.back().front() ==
                '\''))  // The last field is a tag, so record and strip it off
    {
      VW::string_view tag = all->parser_runtime.example_parser->words.back();
      all->parser_runtime.example_parser->words.pop_back();
      if (tag.front() == '\'') { tag.remove_prefix(1); }
      ae->tag.insert(ae->tag.end(), tag.begin(), tag.end());
    }
  }

  if (!all->parser_runtime.example_parser->words.empty())
  {
    all->parser_runtime.example_parser->lbl_parser.parse_label(ae->l, ae->ex_reduction_features,
        all->parser_runtime.example_parser->parser_memory_to_reuse, all->sd->ldict.get(),
        all->parser_runtime.example_parser->words, all->logger);
  }

  if (bar_idx != VW::string_view::npos)
  {
    if (all->output_config.audit || all->output_config.hash_inv)
    {
      tc_parser<true> parser(*all);
      parser.parse_features(*ae, example.substr(bar_idx));
    }
    else
    {
      tc_parser<false> parser(*all);
      parser.parse_features(*ae, example.substr(bar_idx));
    }
  }
}

size_t VW::parsers::text::details::read_features(io_buf& buf, char*& line, size_t& num_chars)
{
  line = nullptr;
  size_t num_chars_initial = buf.readto(line, '\n');
  if (num_chars_initial < 1) { return num_chars_initial; }
  num_chars = num_chars_initial;
  if (line[0] == '\xef' && num_chars >= 3 && line[1] == '\xbb' && line[2] == '\xbf')
  {
    line += 3;
    num_chars -= 3;
  }
  if (num_chars > 0 && line[num_chars - 1] == '\n') { num_chars--; }
  if (num_chars > 0 && line[num_chars - 1] == '\r') { num_chars--; }
  return num_chars_initial;
}

int VW::parsers::text::read_features_string(VW::workspace* all, io_buf& buf, VW::multi_ex& examples)
{
  char* line;
  size_t num_chars;
  // This function consumes input until it reaches a '\n' then it walks back the '\n' and '\r' if it exists.
  size_t num_bytes_consumed = details::read_features(buf, line, num_chars);
  if (num_bytes_consumed < 1)
  {
    // This branch will get hit once we have reached EOF of the input device.
    return static_cast<int>(num_bytes_consumed);
  }

  VW::string_view example(line, num_chars);
  // If this example is empty substring_to_example will mark it as a newline example.
  details::substring_to_example(all, examples[0], example);

  return static_cast<int>(num_bytes_consumed);
}

void VW::parsers::text::read_line(VW::workspace& all, example* ex, VW::string_view line)
{
  while (line.size() > 0 && line.back() == '\n') { line.remove_suffix(1); }
  details::substring_to_example(&all, ex, line);
}

void VW::parsers::text::read_lines(VW::workspace* all, VW::string_view lines_view, VW::multi_ex& examples)
{
  std::vector<VW::string_view> lines;
  VW::tokenize('\n', lines_view, lines);
  for (size_t i = 0; i < lines.size(); i++)
  {
    // Check if a new empty example needs to be added.
    if (examples.size() < i + 1) { examples.push_back(&VW::get_unused_example(all)); }
    read_line(*all, examples[i], lines[i]);
  }
}
