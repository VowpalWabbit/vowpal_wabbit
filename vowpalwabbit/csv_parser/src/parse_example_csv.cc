// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/csv_parser/parse_example_csv.h"

#include "vw/core/best_constant.h"
#include "vw/core/parse_args.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"

#include <string>

namespace VW
{
namespace parsers
{
namespace csv
{
int parse_csv_examples(VW::workspace* all, io_buf& buf, VW::multi_ex& examples)
{
  bool keep_reading = all->custom_parser->next(*all, buf, examples);
  return keep_reading ? 1 : 0;
}

void csv_parser::set_csv_separator(std::string& str, const std::string& name)
{
  if (str.length() > 1)
  {
    char result = str[0];
    if (str[0] == '\\')
    {
      switch (str[1])
      {
        // Allow to specify \t as tabs
        // As pressing tabs usually means auto completion
        case 't':
          result = '\t';
          break;
        default:
          break;
      }
    }

    if ((result != str[0] && str.length() > 2) || result == str[0])
    {
      THROW("Multiple characters passed as " << name << ": " << str);
    }
    str = result;
  }
}

void csv_parser::set_parse_args(VW::config::option_group_definition& in_options, csv_parser_options& parsed_options)
{
  in_options
      .add(VW::config::make_option("csv", parsed_options.enabled)
               .help("Data file will be interpreted as a CSV file")
               .experimental())
      .add(VW::config::make_option("csv_separator", parsed_options.csv_separator)
               .default_value(",")
               .help("CSV Parser: Specify field separator in one character, "
                     "\" | : are not allowed for reservation.")
               .experimental())
      .add(VW::config::make_option("csv_no_file_header", parsed_options.csv_no_file_header)
               .default_value(false)
               .help("CSV Parser: First line is NOT a header. By default, CSV files "
                     "are assumed to have a header with feature and/or namespaces names. "
                     "You MUST specify the header with --csv_header if you use this option.")
               .experimental())
      .add(VW::config::make_option("csv_header", parsed_options.csv_header)
               .default_value("")
               .help("CSV Parser: Override the CSV header by providing (namespace, '|' and) "
                     "feature name separated with ','. By default, CSV files are assumed to "
                     "have a header with feature and/or namespaces names in the CSV first line. "
                     "You can override it by specifying here. Combined with --csv_no_file_header, "
                     "we assume that there is no header in the CSV file.")
               .experimental())
      .add(VW::config::make_option("csv_ns_value", parsed_options.csv_ns_value)
               .default_value("")
               .help("CSV Parser: Scale the namespace values by specifying the float "
                     "ratio. e.g. --csv_ns_value=a:0.5,b:0.3,:8 ")
               .experimental());
}

void csv_parser::handle_parse_args(csv_parser_options& parsed_options)
{
  if (parsed_options.enabled)
  {
    const char* csv_separator_forbid_chars = "\"|:";

    set_csv_separator(parsed_options.csv_separator, "CSV separator");
    for (size_t i = 0; i < strlen(csv_separator_forbid_chars); i++)
    {
      if (parsed_options.csv_separator[0] == csv_separator_forbid_chars[i])
      {
        THROW("Forbidden field separator used: " << parsed_options.csv_separator[0]);
      }
    }

    if (parsed_options.csv_no_file_header && parsed_options.csv_header.empty())
    {
      THROW("No header specified while --csv_no_file_header is set.");
    }
  }
}

class CSV_parser
{
public:
  CSV_parser(VW::workspace* all, VW::example* ae, VW::string_view csv_line, VW::parsers::csv::csv_parser* parser)
      : _parser(parser), _all(all), _ae(ae)
  {
    if (csv_line.empty()) { THROW("Malformed CSV, empty line at " << _parser->line_num << "!"); }
    else
    {
      _csv_line = split(csv_line, parser->options.csv_separator[0], true);
      parse_line();
    }
  }
  ~CSV_parser() {}

private:
  VW::parsers::csv::csv_parser* _parser;
  VW::workspace* _all;
  VW::example* _ae;
  VW::v_array<VW::string_view> _csv_line;
  std::vector<std::string> _token_storage;
  size_t _anon{};
  uint64_t _channel_hash{};

  inline FORCE_INLINE void parse_line()
  {
    bool this_line_is_header = false;

    // Handle the headers and initialize the configuration
    if (_parser->header_fn.empty())
    {
      if (_parser->options.csv_header.empty()) { parse_header(_csv_line); }
      else
      {
        VW::v_array<VW::string_view> header_elements = split(_parser->options.csv_header, ',');
        parse_header(header_elements);
      }

      if (!_parser->options.csv_no_file_header) { this_line_is_header = true; }

      // Store the ns value from CmdLine
      if (_parser->ns_value.empty() && !_parser->options.csv_ns_value.empty()) { parse_ns_value(); }
    }

    if (_csv_line.size() != _parser->header_fn.size())
    {
      THROW("CSV line " << _parser->line_num << " has " << _csv_line.size() << " elements, but the header has "
                        << _parser->header_fn.size() << " elements!");
    }
    else if (!this_line_is_header) { parse_example(); }
  }

  inline FORCE_INLINE void parse_ns_value()
  {
    VW::v_array<VW::string_view> ns_values = split(_parser->options.csv_ns_value, ',', true);
    for (size_t i = 0; i < ns_values.size(); i++)
    {
      VW::v_array<VW::string_view> pair = split(ns_values[i], ':', true);
      std::string ns = " ";
      float value = 1.f;
      if (pair.size() != 2 || pair[1].empty())
      {
        THROW("Malformed namespace value pair at cell " << i + 1 << ": " << ns_values[i]);
      }
      else if (!pair[0].empty()) { ns = std::string{pair[0]}; }

      value = string_to_float(pair[1]);
      if (std::isnan(value)) { THROW("NaN namespace value at cell " << i + 1 << ": " << ns_values[i]); }
      else { _parser->ns_value[std::string{pair[0]}] = value; }
    }
  }

  inline FORCE_INLINE void parse_header(VW::v_array<VW::string_view>& header_elements)
  {
    for (size_t i = 0; i < header_elements.size(); i++)
    {
      if (_parser->options.csv_remove_outer_quotes) { remove_quotation_marks(header_elements[i]); }

      // Handle special column names
      if (header_elements[i] == "_tag" || header_elements[i] == "_label")
      {
        // Handle the tag column
        if (header_elements[i] == "_tag") { _parser->tag_list.emplace_back(i); }
        // Handle the label column
        else if (header_elements[i] == "_label") { _parser->label_list.emplace_back(i); }

        _parser->header_fn.emplace_back();
        _parser->header_ns.emplace_back();
        continue;
      }

      // Handle other column names as feature names
      // Seperate the feature name and namespace from the header.
      VW::v_array<VW::string_view> splitted = split(header_elements[i], '|');
      VW::string_view feature_name;
      VW::string_view ns;
      if (splitted.size() == 1) { feature_name = header_elements[i]; }
      else if (splitted.size() == 2)
      {
        ns = splitted[0];
        feature_name = splitted[1];
      }
      else
      {
        THROW("Malformed header for feature name and namespace separator at cell " << i + 1 << ": "
                                                                                   << header_elements[i]);
      }
      _parser->header_fn.emplace_back(feature_name);
      _parser->header_ns.emplace_back(ns);
      _parser->feature_list[std::string{ns}].emplace_back(i);
    }

    if (_parser->label_list.empty())
    {
      _all->logger.err_warn("No '_label' column found in the header/CSV first line!");
    }
  }

  inline FORCE_INLINE void parse_example()
  {
    _all->example_parser->lbl_parser.default_label(_ae->l);
    if (!_parser->label_list.empty()) { parse_label(); }
    if (!_parser->tag_list.empty()) { parse_tag(); }

    parse_namespaces();
  }

  inline FORCE_INLINE void parse_label()
  {
    VW::string_view label_content = _csv_line[_parser->label_list[0]];
    if (_parser->options.csv_remove_outer_quotes) { remove_quotation_marks(label_content); }

    _all->example_parser->words.clear();
    VW::tokenize(' ', label_content, _all->example_parser->words);

    if (!_all->example_parser->words.empty())
    {
      _all->example_parser->lbl_parser.parse_label(_ae->l, _ae->ex_reduction_features,
          _all->example_parser->parser_memory_to_reuse, _all->sd->ldict.get(), _all->example_parser->words,
          _all->logger);
    }
  }

  inline FORCE_INLINE void parse_tag()
  {
    VW::string_view tag = _csv_line[_parser->tag_list[0]];
    if (_parser->options.csv_remove_outer_quotes) { remove_quotation_marks(tag); }
    if (!tag.empty() && tag.front() == '\'') { tag.remove_prefix(1); }
    _ae->tag.insert(_ae->tag.end(), tag.begin(), tag.end());
  }

  inline FORCE_INLINE void parse_namespaces()
  {
    // Mark to check if all the cells in the line is empty
    bool empty_line = true;
    for (auto& f : _parser->feature_list)
    {
      _anon = 0;
      VW::string_view ns;
      bool new_index = false;
      if (f.first.empty())
      {
        ns = " ";
        _channel_hash = _all->hash_seed == 0 ? 0 : VW::uniform_hash("", 0, _all->hash_seed);
      }
      else
      {
        ns = f.first;
        _channel_hash = _all->example_parser->hasher(ns.data(), ns.length(), _all->hash_seed);
      }

      unsigned char _index = static_cast<unsigned char>(ns[0]);
      if (_ae->feature_space[_index].size() == 0) { new_index = true; }

      float _cur_channel_v = 1.f;
      if (!_parser->ns_value.empty())
      {
        auto it = _parser->ns_value.find(f.first);
        if (it != _parser->ns_value.end()) { _cur_channel_v = it->second; }
      }
      _ae->feature_space[_index].start_ns_extent(_channel_hash);

      for (size_t i = 0; i < f.second.size(); i++)
      {
        size_t column_index = f.second[i];
        empty_line = empty_line && _csv_line[column_index].empty();
        parse_features(_ae->feature_space[_index], column_index, _cur_channel_v, ns);
      }

      _ae->feature_space[_index].end_ns_extent();
      if (new_index && _ae->feature_space[_index].size() > 0) { _ae->indices.emplace_back(_index); }
    }
    _ae->is_newline = empty_line;
  }

  inline FORCE_INLINE void parse_features(features& fs, size_t column_index, float cur_channel_v, VW::string_view ns)
  {
    VW::string_view feature_name = _parser->header_fn[column_index];
    VW::string_view string_feature_value = _csv_line[column_index];

    uint64_t word_hash;
    float _v;
    // don't add empty valued features to list of features
    if (string_feature_value.empty()) { return; }

    bool is_feature_float = false;
    float parsed_feature_value = 0.f;

    if (string_feature_value[0] != '"')
    {
      parsed_feature_value = string_to_float(string_feature_value);
      if (!std::isnan(parsed_feature_value)) { is_feature_float = true; }
    }

    if (!is_feature_float && _parser->options.csv_remove_outer_quotes) { remove_quotation_marks(string_feature_value); }

    if (is_feature_float) { _v = cur_channel_v * parsed_feature_value; }
    else { _v = 1; }

    // Case where feature value is string
    if (!is_feature_float)
    {
      // chain hash is hash(feature_value, hash(feature_name, namespace_hash)) & parse_mask
      word_hash = (_all->example_parser->hasher(string_feature_value.data(), string_feature_value.length(),
                       _all->example_parser->hasher(feature_name.data(), feature_name.length(), _channel_hash)) &
          _all->parse_mask);
    }
    // Case where feature value is float and feature name is not empty
    else if (!feature_name.empty())
    {
      word_hash =
          (_all->example_parser->hasher(feature_name.data(), feature_name.length(), _channel_hash) & _all->parse_mask);
    }
    // Case where feature value is float and feature name is empty
    else { word_hash = _channel_hash + _anon++; }

    // don't add 0 valued features to list of features
    if (_v == 0) { return; }
    fs.push_back(_v, word_hash);

    if (_all->audit || _all->hash_inv)
    {
      if (!is_feature_float)
      {
        fs.space_names.emplace_back(
            VW::audit_strings(std::string{ns}, std::string{feature_name}, std::string{string_feature_value}));
      }
      else { fs.space_names.emplace_back(VW::audit_strings(std::string{ns}, std::string{feature_name})); }
    }
  }

  inline FORCE_INLINE VW::v_array<VW::string_view> split(VW::string_view sv, const char ch, bool use_quotes = false)
  {
    VW::v_array<VW::string_view> collections;
    size_t pointer = 0;
    // Trim extra characters that are useless for us to read
    const char* trim_list = "\r\n\xef\xbb\xbf\f\v";
    sv.remove_prefix(std::min(sv.find_first_not_of(trim_list), sv.size()));
    sv.remove_suffix(std::min(sv.size() - sv.find_last_not_of(trim_list) - 1, sv.size()));

    VW::v_array<size_t> unquoted_quotes_index;
    bool inside_quotes = false;

    if (sv.empty())
    {
      collections.emplace_back();
      return collections;
    }

    for (size_t i = 0; i <= sv.length(); i++)
    {
      if (i == sv.length() && inside_quotes) { THROW("Unclosed quote at end of line " << _parser->line_num << "."); }
      // Skip Quotes at the start and end of the cell
      else if (use_quotes && !inside_quotes && i == pointer && i < sv.length() && sv[i] == '"')
      {
        inside_quotes = true;
      }
      else if (use_quotes && inside_quotes && i < sv.length() - 1 && sv[i] == '"' && sv[i] == sv[i + 1])
      {
        // RFC-4180, paragraph "If double-quotes are used to enclose fields,
        // then a double-quote appearing inside a field must be escaped by
        // preceding it with another double-quote."
        unquoted_quotes_index.emplace_back(i - pointer);
        i++;
      }
      else if (use_quotes && inside_quotes &&
          ((i < sv.length() - 1 && sv[i] == '"' && sv[i + 1] == ch) || (i == sv.length() - 1 && sv[i] == '"')))
      {
        inside_quotes = false;
      }
      else if (use_quotes && inside_quotes && i < sv.length() && sv[i] == '"')
      {
        THROW("Unescaped quote at position "
            << i + 1 << " of line " << _parser->line_num
            << ", double-quote appearing inside a cell must be escaped by preceding it with another double-quote!");
      }
      else if (i == sv.length() || (!inside_quotes && sv[i] == ch))
      {
        VW::string_view element(&sv[pointer], i - pointer);
        if (i == sv.length() && sv[i - 1] == ch) { element = VW::string_view(); }

        if (unquoted_quotes_index.empty()) { collections.emplace_back(element); }
        else
        {
          // Make double escaped quotes into one
          std::string new_string;
          size_t quotes_pointer = 0;
          unquoted_quotes_index.emplace_back(element.size());
          for (size_t j = 0; j < unquoted_quotes_index.size(); j++)
          {
            size_t sv_size = unquoted_quotes_index[j] - quotes_pointer;
            if (sv_size > 0 && quotes_pointer < element.size())
            {
              VW::string_view str_part(&element[quotes_pointer], sv_size);
              new_string += {str_part.begin(), str_part.end()};
            }
            quotes_pointer = unquoted_quotes_index[j] + 1;
          }
          // This is a bit of a hack to expand string lifetime.
          _token_storage.emplace_back(new_string);
          collections.emplace_back(_token_storage.back());
        }
        unquoted_quotes_index.clear();
        if (i < sv.length() - 1) { pointer = i + 1; }
      }
    }
    return collections;
  }

  inline FORCE_INLINE void remove_quotation_marks(VW::string_view& sv)
  {
    // When the outer quotes pair, we just remove them.
    // If they don't, we just keep them without throwing any errors.
    if (sv.size() > 1 && sv[0] == '"' && sv[0] == sv[sv.size() - 1])
    {
      sv.remove_prefix(1);
      sv.remove_suffix(1);
    }
  }

  inline FORCE_INLINE float string_to_float(VW::string_view sv)
  {
    size_t end_read = 0;
    float parsed = VW::details::parse_float(sv.data(), end_read, sv.data() + sv.size());
    // Not a valid float, return NaN
    if (!(end_read == sv.size())) { parsed = std::numeric_limits<float>::quiet_NaN(); }
    return parsed;
  }
};

void csv_parser::reset()
{
  if (options.csv_header.empty())
  {
    header_fn.clear();
    header_ns.clear();
    label_list.clear();
    tag_list.clear();
    feature_list.clear();
  }
  line_num = 0;
}

int csv_parser::parse_csv(VW::workspace* all, VW::example* ae, io_buf& buf)
{
  // This function consumes input until it reaches a '\n' then it walks back the '\n' and '\r' if it exists.
  size_t num_bytes_consumed = read_line(all, ae, buf);
  // Read the data again if what just read is header.
  if (line_num == 1 && !options.csv_no_file_header) { num_bytes_consumed += read_line(all, ae, buf); }
  return static_cast<int>(num_bytes_consumed);
}

size_t csv_parser::read_line(VW::workspace* all, VW::example* ae, io_buf& buf)
{
  char* line = nullptr;
  size_t num_chars_initial = buf.readto(line, '\n');
  // This branch will get hit when we haven't reached EOF of the input device.
  if (num_chars_initial > 0)
  {
    size_t num_chars = num_chars_initial;
    if (line[0] == '\xef' && num_chars >= 3 && line[1] == '\xbb' && line[2] == '\xbf')
    {
      line += 3;
      num_chars -= 3;
    }
    if (num_chars > 0 && line[num_chars - 1] == '\n') { num_chars--; }
    if (num_chars > 0 && line[num_chars - 1] == '\r') { num_chars--; }

    line_num++;
    VW::string_view csv_line(line, num_chars);
    CSV_parser parse_line(all, ae, csv_line, this);
  }
  // EOF is reached, reset for possible next file.
  else { reset(); }
  return num_chars_initial;
}

}  // namespace csv
}  // namespace parsers
}  // namespace VW
