// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cstdio>
#include <cfloat>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <utility>

#include "constant.h"
#include "parse_regressor.h"
#include "parser.h"
#include "parse_primitives.h"
#include "vw.h"
#include "interactions.h"

#include "parse_args.h"
#include "reduction_stack.h"

#include "rand48.h"
#include "learner.h"
#include "prediction_type.h"
#include "label_type.h"
#include "parse_example.h"
#include "best_constant.h"
#include "vw_exception.h"
#include "accumulate.h"
#include "vw_validate.h"
#include "vw_allreduce.h"
#include "metrics.h"
#include "text_utils.h"
#include "interactions.h"

#include "options.h"
#include "options_boost_po.h"
#include "options_serializer_boost_po.h"
#include "named_labels.h"

#include "io/io_adapter.h"
#include "io/custom_streambuf.h"
#include "io/owning_stream.h"
#include "io/logger.h"

#ifdef BUILD_EXTERNAL_PARSER
#  include "parse_example_binary.h"
#endif

using std::endl;
using namespace VW::config;

uint64_t hash_file_contents(VW::io::reader* f)
{
  uint64_t v = 5289374183516789128;
  char buf[1024];
  while (true)
  {
    ssize_t n = f->read(buf, 1024);
    if (n <= 0) break;
    for (ssize_t i = 0; i < n; i++)
    {
      v *= 341789041;
      v += static_cast<uint64_t>(buf[i]);
    }
  }
  return v;
}

bool directory_exists(const std::string& path)
{
  struct stat info;
  if (stat(path.c_str(), &info) != 0)
    return false;
  else
    return (info.st_mode & S_IFDIR) > 0;
  //  boost::filesystem::path p(path);
  //  return boost::filesystem::exists(p) && boost::filesystem::is_directory(p);
}

std::string find_in_path(const std::vector<std::string>& paths, const std::string& fname)
{
#ifdef _WIN32
  const std::string delimiter = "\\";
#else
  const std::string delimiter = "/";
#endif
  for (const auto& path : paths)
  {
    std::string full = path;
    if (!VW::ends_with(path, delimiter)) { full += delimiter; }
    full += fname;
    std::ifstream f(full.c_str());
    if (f.good()) return full;
  }
  return "";
}

void parse_dictionary_argument(VW::workspace& all, const std::string& str)
{
  if (str.length() == 0) return;
  // expecting 'namespace:file', for instance 'w:foo.txt'
  // in the case of just 'foo.txt' it's applied to the default namespace

  char ns = ' ';
  VW::string_view s(str);
  if ((str.length() > 2) && (str[1] == ':'))
  {
    ns = str[0];
    s.remove_prefix(2);
  }

  std::string file_name = find_in_path(all.dictionary_path, std::string(s));
  if (file_name.empty()) THROW("error: cannot find dictionary '" << s << "' in path; try adding --dictionary_path")

  bool is_gzip = VW::ends_with(file_name, ".gz");
  std::unique_ptr<VW::io::reader> file_adapter;
  try
  {
    file_adapter = is_gzip ? VW::io::open_compressed_file_reader(file_name) : VW::io::open_file_reader(file_name);
  }
  catch (...)
  {
    THROW("error: cannot read dictionary from file '" << file_name << "'"
                                                      << ", opening failed")
  }

  uint64_t fd_hash = hash_file_contents(file_adapter.get());

  if (!all.quiet)
    *(all.trace_message) << "scanned dictionary '" << s << "' from '" << file_name << "', hash=" << std::hex << fd_hash
                         << std::dec << endl;

  // see if we've already read this dictionary
  for (size_t id = 0; id < all.loaded_dictionaries.size(); id++)
  {
    if (all.loaded_dictionaries[id].file_hash == fd_hash)
    {
      all.namespace_dictionaries[static_cast<size_t>(ns)].push_back(all.loaded_dictionaries[id].dict);
      return;
    }
  }

  std::unique_ptr<VW::io::reader> fd;
  try
  {
    fd = VW::io::open_file_reader(file_name);
  }
  catch (...)
  {
    THROW("error: cannot re-read dictionary from file '" << file_name << "', opening failed")
  }
  auto map = std::make_shared<feature_dict>();
  // mimicking old v_hashmap behavior for load factor.
  // A smaller factor will generally use more memory but have faster access
  map->max_load_factor(0.25);
  example* ec = VW::alloc_examples(1);

  auto def = static_cast<size_t>(' ');

  ssize_t size = 2048, pos, num_read;
  char rc;
  char* buffer = calloc_or_throw<char>(size);
  do
  {
    pos = 0;
    do
    {
      num_read = fd->read(&rc, 1);
      if ((rc != EOF) && (num_read > 0)) buffer[pos++] = rc;
      if (pos >= size - 1)
      {
        size *= 2;
        const auto new_buffer = static_cast<char*>(realloc(buffer, size));
        if (new_buffer == nullptr)
        {
          free(buffer);
          VW::dealloc_examples(ec, 1);
          THROW("error: memory allocation failed in reading dictionary")
        }
        else
          buffer = new_buffer;
      }
    } while ((rc != EOF) && (rc != '\n') && (num_read > 0));
    buffer[pos] = 0;

    // we now have a line in buffer
    char* c = buffer;
    while (*c == ' ' || *c == '\t') ++c;  // skip initial whitespace
    char* d = c;
    while (*d != ' ' && *d != '\t' && *d != '\n' && *d != '\0') ++d;  // gobble up initial word
    if (d == c) continue;                                             // no word
    if (*d != ' ' && *d != '\t') continue;                            // reached end of line
    std::string word(c, d - c);
    if (map->find(word) != map->end())  // don't overwrite old values!
    { continue; }
    d--;
    *d = '|';  // set up for parser::read_line
    VW::read_line(all, ec, d);
    // now we just need to grab stuff from the default namespace of ec!
    if (ec->feature_space[def].empty()) { continue; }
    map->emplace(word, VW::make_unique<features>(ec->feature_space[def]));

    // clear up ec
    ec->tag.clear();
    ec->indices.clear();
    for (size_t i = 0; i < 256; i++) { ec->feature_space[i].clear(); }
  } while ((rc != EOF) && (num_read > 0));
  free(buffer);
  VW::dealloc_examples(ec, 1);

  if (!all.quiet)
    *(all.trace_message) << "dictionary " << s << " contains " << map->size() << " item"
                         << (map->size() == 1 ? "" : "s") << endl;

  all.namespace_dictionaries[static_cast<size_t>(ns)].push_back(map);
  dictionary_info info = {s.to_string(), fd_hash, map};
  all.loaded_dictionaries.push_back(info);
}

void parse_affix_argument(VW::workspace& all, const std::string& str)
{
  if (str.length() == 0) return;
  char* cstr = calloc_or_throw<char>(str.length() + 1);
  VW::string_cpy(cstr, (str.length() + 1), str.c_str());

  char* next_token;
  char* p = strtok_s(cstr, ",", &next_token);

  try
  {
    while (p)
    {
      char* q = p;
      uint16_t prefix = 1;
      if (q[0] == '+') { q++; }
      else if (q[0] == '-')
      {
        prefix = 0;
        q++;
      }
      if ((q[0] < '1') || (q[0] > '7')) THROW("malformed affix argument (length must be 1..7): " << p)

      auto len = static_cast<uint16_t>(q[0] - '0');
      auto ns = static_cast<uint16_t>(' ');  // default namespace
      if (q[1] != 0)
      {
        if (valid_ns(q[1]))
          ns = static_cast<uint16_t>(q[1]);
        else
          THROW("malformed affix argument (invalid namespace): " << p)

        if (q[2] != 0) THROW("malformed affix argument (too long): " << p)
      }

      uint16_t afx = (len << 1) | (prefix & 0x1);
      all.affix_features[ns] <<= 4;
      all.affix_features[ns] |= afx;

      p = strtok_s(nullptr, ",", &next_token);
    }
  }
  catch (...)
  {
    free(cstr);
    throw;
  }

  free(cstr);
}

void parse_diagnostics(options_i& options, VW::workspace& all)
{
  bool version_arg = false;
  bool help = false;
  bool skip_driver = false;
  std::string progress_arg;
  option_group_definition diagnostic_group("Diagnostic");
  diagnostic_group.add(make_option("version", version_arg).help("Version information"))
      .add(make_option("audit", all.audit).short_name("a").help("Print weights of features"))
      .add(make_option("progress", progress_arg)
               .short_name("P")
               .help("Progress update frequency. int: additive, float: multiplicative"))
      .add(make_option("dry_run", skip_driver)
               .help("Parse arguments and print corresponding metadata. Will not execute driver"))
      .add(make_option("help", help)
               .short_name("h")
               .help("More information on vowpal wabbit can be found here https://vowpalwabbit.org"));

  options.add_and_parse(diagnostic_group);

  if (help)
  {
    all.quiet = true;
    all.logger.set_level(VW::io::log_level::off);
    // This is valid:
    // https://stackoverflow.com/questions/25690636/is-it-valid-to-construct-an-stdostream-from-a-null-buffer This
    // results in the ostream not outputting anything.
    all.trace_message = VW::make_unique<std::ostream>(nullptr);
  }

  // pass all.quiet around
  if (all.all_reduce) all.all_reduce->quiet = all.quiet;

  // Upon direct query for version -- spit it out directly to stdout
  if (version_arg)
  {
    std::cout << VW::version.to_string() << " (git commit: " << VW::git_commit << ")\n";
    exit(0);
  }

  if (options.was_supplied("progress") && !all.quiet)
  {
    all.progress_arg = static_cast<float>(::atof(progress_arg.c_str()));
    // --progress interval is dual: either integer or floating-point
    if (progress_arg.find_first_of('.') == std::string::npos)
    {
      // No "." in arg: assume integer -> additive
      all.progress_add = true;
      if (all.progress_arg < 1)
      {
        all.logger.err_warn("Additive --progress <int> can't be < 1: forcing to 1");
        all.progress_arg = 1;
      }
      all.sd->dump_interval = all.progress_arg;
    }
    else
    {
      // A "." in arg: assume floating-point -> multiplicative
      all.progress_add = false;

      if (all.progress_arg <= 1.f)
      {
        all.logger.err_warn("Multiplicative --progress <float> '{}' is <= 1.0: adding 1.0", progress_arg);
        all.progress_arg += 1.f;
      }
      else if (all.progress_arg > 9.f)
      {
        all.logger.err_warn(
            "Multiplicative --progress <float> '' is > 9.0: Did you mean mean to use an integer?", progress_arg);
      }
      all.sd->dump_interval = 1.f;
    }
  }
}

input_options parse_source(VW::workspace& all, options_i& options)
{
  input_options parsed_options;

  option_group_definition input_options("Input");
  input_options.add(make_option("data", all.data_filename).short_name("d").help("Example set"))
      .add(make_option("daemon", parsed_options.daemon).help("Persistent daemon mode on port 26542"))
      .add(make_option("foreground", parsed_options.foreground)
               .help("In persistent daemon mode, do not run in the background"))
      .add(make_option("port", parsed_options.port).help("Port to listen on; use 0 to pick unused port"))
      .add(make_option("num_children", all.num_children).help("Number of children for persistent daemon mode"))
      .add(make_option("pid_file", parsed_options.pid_file).help("Write pid file in persistent daemon mode"))
      .add(make_option("port_file", parsed_options.port_file).help("Write port used in persistent daemon mode"))
      .add(make_option("cache", parsed_options.cache).short_name("c").help("Use a cache.  The default is <data>.cache"))
      .add(make_option("cache_file", parsed_options.cache_files).help("The location(s) of cache_file"))
      .add(make_option("json", parsed_options.json).help("Enable JSON parsing"))
      .add(make_option("dsjson", parsed_options.dsjson).help("Enable Decision Service JSON parsing"))
      .add(make_option("kill_cache", parsed_options.kill_cache)
               .short_name("k")
               .help("Do not reuse existing cache: create a new one always"))
      .add(
          make_option("compressed", parsed_options.compressed)
              .help(
                  "use gzip format whenever possible. If a cache file is being created, this option creates a "
                  "compressed cache file. A mixture of raw-text & compressed inputs are supported with autodetection."))
      .add(make_option("no_stdin", all.stdin_off).help("Do not default to reading from stdin"))
      .add(make_option("no_daemon", all.no_daemon)
               .help("Force a loaded daemon or active learning model to accept local input instead of starting in "
                     "daemon mode"))
      .add(make_option("chain_hash", parsed_options.chain_hash_json)
               .keep()
               .help("Enable chain hash in JSON for feature name and string feature value. e.g. {'A': {'B': 'C'}} is "
                     "hashed as A^B^C."))
      .add(make_option("flatbuffer", parsed_options.flatbuffer)
               .help("Data file will be interpreted as a flatbuffer file"));
#ifdef BUILD_EXTERNAL_PARSER
  VW::external::parser::set_parse_args(input_options, parsed_options);
#endif

  options.add_and_parse(input_options);

  // Check if the options provider has any positional args. Only really makes sense for command line, others just return
  // an empty list.
  const auto positional_tokens = options.get_positional_tokens();
  if (positional_tokens.size() == 1) { all.data_filename = positional_tokens[0]; }
  else if (positional_tokens.size() > 1)
  {
    all.logger.err_warn(
        "Multiple data files passed as positional parameters, only the first one will be "
        "read and the rest will be ignored.");
  }

  if (parsed_options.daemon || options.was_supplied("pid_file") || (options.was_supplied("port") && !all.active))
  {
    all.daemon = true;
    // allow each child to process up to 1e5 connections
    all.numpasses = static_cast<size_t>(1e5);
  }

  // Add an implicit cache file based on the data filename.
  if (parsed_options.cache) { parsed_options.cache_files.push_back(all.data_filename + ".cache"); }

  if ((parsed_options.cache || options.was_supplied("cache_file")) && options.was_supplied("invert_hash"))
    THROW("invert_hash is incompatible with a cache file.  Use it in single pass mode only.")

  if (!all.holdout_set_off &&
      (options.was_supplied("output_feature_regularizer_binary") ||
          options.was_supplied("output_feature_regularizer_text")))
  {
    all.holdout_set_off = true;
    *(all.trace_message) << "Making holdout_set_off=true since output regularizer specified" << endl;
  }

  return parsed_options;
}

namespace VW
{
const char* are_features_compatible(VW::workspace& vw1, VW::workspace& vw2)
{
  if (vw1.example_parser->hasher != vw2.example_parser->hasher) return "hasher";

  if (!std::equal(vw1.spelling_features.begin(), vw1.spelling_features.end(), vw2.spelling_features.begin()))
    return "spelling_features";

  if (!std::equal(vw1.affix_features.begin(), vw1.affix_features.end(), vw2.affix_features.begin()))
    return "affix_features";

  if (vw1.skip_gram_transformer != nullptr && vw2.skip_gram_transformer != nullptr)
  {
    const auto& vw1_ngram_strings = vw1.skip_gram_transformer->get_initial_ngram_definitions();
    const auto& vw2_ngram_strings = vw2.skip_gram_transformer->get_initial_ngram_definitions();
    const auto& vw1_skips_strings = vw1.skip_gram_transformer->get_initial_skip_definitions();
    const auto& vw2_skips_strings = vw2.skip_gram_transformer->get_initial_skip_definitions();

    if (!std::equal(vw1_ngram_strings.begin(), vw1_ngram_strings.end(), vw2_ngram_strings.begin())) return "ngram";

    if (!std::equal(vw1_skips_strings.begin(), vw1_skips_strings.end(), vw2_skips_strings.begin())) return "skips";
  }
  else if (vw1.skip_gram_transformer != nullptr || vw2.skip_gram_transformer != nullptr)
  {
    // If one of them didn't define the ngram transformer then they differ by ngram (skips depends on ngram)
    return "ngram";
  }

  if (!std::equal(vw1.limit.begin(), vw1.limit.end(), vw2.limit.begin())) return "limit";

  if (vw1.num_bits != vw2.num_bits) return "num_bits";

  if (vw1.permutations != vw2.permutations) return "permutations";

  if (vw1.interactions.size() != vw2.interactions.size()) return "interactions size";

  if (vw1.ignore_some != vw2.ignore_some) return "ignore_some";

  if (vw1.ignore_some && !std::equal(vw1.ignore.begin(), vw1.ignore.end(), vw2.ignore.begin())) return "ignore";

  if (vw1.ignore_some_linear != vw2.ignore_some_linear) return "ignore_some_linear";

  if (vw1.ignore_some_linear &&
      !std::equal(vw1.ignore_linear.begin(), vw1.ignore_linear.end(), vw2.ignore_linear.begin()))
    return "ignore_linear";

  if (vw1.redefine_some != vw2.redefine_some) return "redefine_some";

  if (vw1.redefine_some && !std::equal(vw1.redefine.begin(), vw1.redefine.end(), vw2.redefine.begin()))
    return "redefine";

  if (vw1.add_constant != vw2.add_constant) return "add_constant";

  if (vw1.dictionary_path.size() != vw2.dictionary_path.size()) return "dictionary_path size";

  if (!std::equal(vw1.dictionary_path.begin(), vw1.dictionary_path.end(), vw2.dictionary_path.begin()))
    return "dictionary_path";

  for (auto i = std::begin(vw1.interactions), j = std::begin(vw2.interactions); i != std::end(vw1.interactions);
       ++i, ++j)
    if (*i != *j) return "interaction mismatch";

  return nullptr;
}

}  // namespace VW

std::vector<namespace_index> parse_char_interactions(VW::string_view input, VW::io::logger& logger)
{
  std::vector<namespace_index> result;

  auto decoded = VW::decode_inline_hex(input, logger);
  result.insert(result.begin(), decoded.begin(), decoded.end());
  return result;
}

std::vector<extent_term> parse_full_name_interactions(VW::workspace& all, VW::string_view str)
{
  std::vector<extent_term> result;
  auto encoded = VW::decode_inline_hex(str, all.logger);

  std::vector<VW::string_view> tokens;
  tokenize('|', str, tokens, true);
  for (const auto& token : tokens)
  {
    if (token.empty()) { THROW("A term in --experimental_full_name_interactions cannot be empty. Given: " << str) }
    if (std::find(token.begin(), token.end(), ':') != token.end())
    {
      if (token.size() != 1)
      {
        THROW(
            "A wildcard term in --experimental_full_name_interactions cannot contain characters other than ':'. Found: "
            << token)
      }
      result.emplace_back(wildcard_namespace, wildcard_namespace);
    }
    else
    {
      const auto ns_hash = VW::hash_space(all, std::string{token});
      result.emplace_back(static_cast<namespace_index>(token[0]), ns_hash);
    }
  }
  return result;
}

void parse_feature_tweaks(options_i& options, VW::workspace& all, bool interactions_settings_duplicated,
    std::vector<std::string>& dictionary_nses)
{
  std::string hash_function;
  uint32_t new_bits;
  std::vector<std::string> spelling_ns;
  std::vector<std::string> quadratics;
  std::vector<std::string> cubics;
  std::vector<std::string> interactions;
  std::vector<std::string> full_name_interactions;
  std::vector<std::string> ignores;
  std::vector<std::string> ignore_linears;
  std::vector<std::string> keeps;
  std::vector<std::string> redefines;

  std::vector<std::string> ngram_strings;
  std::vector<std::string> skip_strings;

  std::vector<std::string> dictionary_path;

  bool noconstant;
  bool leave_duplicate_interactions;
  std::string affix;

  option_group_definition feature_options("Feature");
  feature_options
#ifdef PRIVACY_ACTIVATION
      .add(make_option("privacy_activation", all.privacy_activation)
               .help("turns on aggregated weight exporting when the unique feature tags cross "
                     "`privacy_activation_threshold`"))
      .add(make_option("privacy_activation_threshold", all.privacy_activation_threshold)
               .help("takes effect when `privacy_activation` is turned on and is the number of unique tag hashes a "
                     "weight needs to see before it is exported"))
#endif
      .add(make_option("hash", hash_function)
               .default_value("strings")
               .keep()
               .one_of({"strings", "all"})
               .help("How to hash the features"))
      .add(make_option("hash_seed", all.hash_seed).keep().default_value(0).help("Seed for hash function"))
      .add(make_option("ignore", ignores).keep().help("Ignore namespaces beginning with character <arg>"))
      .add(make_option("ignore_linear", ignore_linears)
               .keep()
               .help("Ignore namespaces beginning with character <arg> for linear terms only"))
      .add(make_option("keep", keeps).keep().help("Keep namespaces beginning with character <arg>"))
      .add(make_option("redefine", redefines)
               .keep()
               .help("Redefine namespaces beginning with characters of std::string S as namespace N. <arg> shall be in "
                     "form "
                     "'N:=S' where := is operator. Empty N or S are treated as default namespace. Use ':' as a "
                     "wildcard in S.")
               .keep())
      .add(make_option("bit_precision", new_bits).short_name("b").help("Number of bits in the feature table"))
      .add(make_option("noconstant", noconstant).help("Don't add a constant feature"))
      .add(make_option("constant", all.initial_constant)
               .default_value(0.f)
               .short_name("C")
               .help("Set initial value of constant"))
      .add(make_option("ngram", ngram_strings)
               .help("Generate N grams. To generate N grams for a single namespace 'foo', arg should be fN"))
      .add(make_option("skips", skip_strings)
               .help("Generate skips in N grams. This in conjunction with the ngram tag can be used to generate "
                     "generalized n-skip-k-gram. To generate n-skips for a single namespace 'foo', arg should be fN."))
      .add(
          make_option("feature_limit", all.limit_strings)
              .help("Limit to N unique features per namespace. To apply to a single namespace 'foo', arg should be fN"))
      .add(make_option("affix", affix)
               .keep()
               .help("Generate prefixes/suffixes of features; argument '+2a,-3b,+1' means generate 2-char prefixes for "
                     "namespace a, 3-char suffixes for b and 1 char prefixes for default namespace"))
      .add(make_option("spelling", spelling_ns)
               .keep()
               .help("Compute spelling features for a give namespace (use '_' for default namespace)"))
      .add(make_option("dictionary", dictionary_nses)
               .keep()
               .help("Read a dictionary for additional features (arg either 'x:file' or just 'file')"))
      .add(make_option("dictionary_path", dictionary_path)
               .help("Look in this directory for dictionaries; defaults to current directory or env{PATH}"))
      .add(make_option("interactions", interactions)
               .keep()
               .help("Create feature interactions of any level between namespaces"))
      .add(make_option("experimental_full_name_interactions", full_name_interactions)
               .keep()
               .help("EXPERIMENTAL: Create feature interactions of any level between namespaces by specifying the full "
                     "name of each namespace."))
      .add(make_option("permutations", all.permutations)
               .help("Use permutations instead of combinations for feature interactions of same namespace"))
      .add(make_option("leave_duplicate_interactions", leave_duplicate_interactions)
               .help("Don't remove interactions with duplicate combinations of namespaces. For ex. this is a "
                     "duplicate: '-q ab -q ba' and a lot more in '-q ::'."))
      .add(make_option("quadratic", quadratics).short_name("q").keep().help("Create and use quadratic features"))
      .add(make_option("cubic", cubics).keep().help("Create and use cubic features"));
  options.add_and_parse(feature_options);

  // feature manipulation
  all.example_parser->hasher = getHasher(hash_function);

  if (options.was_supplied("spelling"))
  {
    for (auto& spelling_n : spelling_ns)
    {
      spelling_n = VW::decode_inline_hex(spelling_n, all.logger);
      if (spelling_n[0] == '_')
        all.spelling_features[static_cast<unsigned char>(' ')] = true;
      else
        all.spelling_features[static_cast<size_t>(spelling_n[0])] = true;
    }
  }

  if (options.was_supplied("affix")) parse_affix_argument(all, VW::decode_inline_hex(affix, all.logger));

  // Process ngram and skips arguments
  if (options.was_supplied("skips"))
  {
    if (!options.was_supplied("ngram")) { THROW("skip cannot be used unless ngram is > 1") }
  }

  if (options.was_supplied("ngram"))
  {
    if (options.was_supplied("sort_features")) { THROW("ngram is incompatible with sort_features.") }

    std::vector<std::string> hex_decoded_ngram_strings;
    hex_decoded_ngram_strings.reserve(ngram_strings.size());
    std::transform(ngram_strings.begin(), ngram_strings.end(), std::back_inserter(hex_decoded_ngram_strings),
        [&](const std::string& arg) { return VW::decode_inline_hex(arg, all.logger); });

    std::vector<std::string> hex_decoded_skip_strings;
    hex_decoded_skip_strings.reserve(skip_strings.size());
    std::transform(skip_strings.begin(), skip_strings.end(), std::back_inserter(hex_decoded_skip_strings),
        [&](const std::string& arg) { return VW::decode_inline_hex(arg, all.logger); });

    all.skip_gram_transformer = VW::make_unique<VW::kskip_ngram_transformer>(
        VW::kskip_ngram_transformer::build(hex_decoded_ngram_strings, hex_decoded_skip_strings, all.quiet, all.logger));
  }

  if (options.was_supplied("feature_limit")) compile_limits(all.limit_strings, all.limit, all.quiet, all.logger);

  if (options.was_supplied("bit_precision"))
  {
    if (all.default_bits == false && new_bits != all.num_bits)
      THROW("Number of bits is set to " << new_bits << " and " << all.num_bits
                                        << " by argument and model.  That does not work.")

    all.default_bits = false;
    all.num_bits = new_bits;

    VW::validate_num_bits(all);
  }

  // prepare namespace interactions
  std::vector<std::vector<namespace_index>> decoded_interactions;

  if ( ( (!all.interactions.empty() && /*data was restored from old model file directly to v_array and will be overriden automatically*/
          (options.was_supplied("quadratic") || options.was_supplied("cubic") || options.was_supplied("interactions")) ) )
       ||
       interactions_settings_duplicated /*settings were restored from model file to file_options and overriden by params from command line*/)
  {
    all.logger.err_warn(
        "model file has set of {-q, --cubic, --interactions} settings stored, but they'll be "
        "OVERRIDDEN by set of {-q, --cubic, --interactions} settings from command line.");
    // in case arrays were already filled in with values from old model file - reset them
    if (!all.interactions.empty()) { all.interactions.clear(); }
  }

  if (options.was_supplied("quadratic"))
  {
    for (auto& i : quadratics)
    {
      auto parsed = parse_char_interactions(i, all.logger);
      if (parsed.size() != 2) { THROW("error, quadratic features must involve two sets.)") }
      decoded_interactions.emplace_back(parsed.begin(), parsed.end());
    }

    if (!all.quiet)
    {
      *(all.trace_message) << fmt::format("creating quadratic features for pairs: {}\n", fmt::join(quadratics, " "));
    }
  }

  if (options.was_supplied("cubic"))
  {
    for (const auto& i : cubics)
    {
      auto parsed = parse_char_interactions(i, all.logger);
      if (parsed.size() != 3) { THROW("Cubic features must involve three sets.") }
      decoded_interactions.emplace_back(parsed.begin(), parsed.end());
    }

    if (!all.quiet)
    { *(all.trace_message) << fmt::format("creating cubic features for triples: {}\n", fmt::join(cubics, " ")); }
  }

  if (options.was_supplied("interactions"))
  {
    for (const auto& i : interactions)
    {
      auto parsed = parse_char_interactions(i, all.logger);
      if (parsed.size() < 2) { THROW("Feature interactions must involve at least two namespaces.") }
      decoded_interactions.emplace_back(parsed.begin(), parsed.end());
    }
    if (!all.quiet)
    {
      *(all.trace_message) << fmt::format(
          "creating features for following interactions: {}\n", fmt::join(interactions, " "));
    }
  }

  if (!decoded_interactions.empty())
  {
    if (!all.quiet && !options.was_supplied("leave_duplicate_interactions"))
    {
      auto any_contain_wildcards = std::any_of(decoded_interactions.begin(), decoded_interactions.end(),
          [](const std::vector<namespace_index>& interaction) { return INTERACTIONS::contains_wildcard(interaction); });
      if (any_contain_wildcards)
      {
        all.logger.err_warn(
            "Any duplicate namespace interactions will be removed\n"
            "You can use --leave_duplicate_interactions to disable this behaviour.");
      }
    }

    // Sorts the overall list
    std::sort(decoded_interactions.begin(), decoded_interactions.end(), INTERACTIONS::sort_interactions_comparator);

    size_t removed_cnt = 0;
    size_t sorted_cnt = 0;
    // Sorts individual interactions
    INTERACTIONS::sort_and_filter_duplicate_interactions(
        decoded_interactions, !leave_duplicate_interactions, removed_cnt, sorted_cnt);

    if (removed_cnt > 0 && !all.quiet)
    {
      all.logger.err_warn(
          "Duplicate namespace interactions were found. Removed: {}.\nYou can use --leave_duplicate_interactions to "
          "disable this behaviour.",
          removed_cnt);
    }

    if (sorted_cnt > 0 && !all.quiet)
    {
      all.logger.err_warn(
          "Some interactions contain duplicate characters and their characters order has been changed. Interactions "
          "affected: {}.",
          sorted_cnt);
    }

    all.interactions = std::move(decoded_interactions);
  }

  if (options.was_supplied("experimental_full_name_interactions"))
  {
    for (const auto& i : full_name_interactions)
    {
      auto parsed = parse_full_name_interactions(all, i);
      if (parsed.size() < 2) { THROW("Feature interactions must involve at least two namespaces") }
      std::sort(parsed.begin(), parsed.end());
      all.extent_interactions.push_back(parsed);
    }
    std::sort(all.extent_interactions.begin(), all.extent_interactions.end());
    if (!leave_duplicate_interactions)
    {
      all.extent_interactions.erase(
          std::unique(all.extent_interactions.begin(), all.extent_interactions.end()), all.extent_interactions.end());
    }
  }

  for (size_t i = 0; i < NUM_NAMESPACES; i++)
  {
    all.ignore[i] = false;
    all.ignore_linear[i] = false;
  }
  all.ignore_some = false;
  all.ignore_some_linear = false;

  if (options.was_supplied("ignore"))
  {
    all.ignore_some = true;

    for (auto& i : ignores)
    {
      i = VW::decode_inline_hex(i, all.logger);
      for (auto j : i) all.ignore[static_cast<size_t>(static_cast<unsigned char>(j))] = true;
    }

    if (!all.quiet)
    {
      *(all.trace_message) << "ignoring namespaces beginning with:";
      for (size_t i = 0; i < NUM_NAMESPACES; ++i)
      {
        if (all.ignore[i]) *(all.trace_message) << " " << static_cast<unsigned char>(i);
      }
      *(all.trace_message) << endl;
    }
  }

  if (options.was_supplied("ignore_linear"))
  {
    all.ignore_some_linear = true;

    for (auto& i : ignore_linears)
    {
      i = VW::decode_inline_hex(i, all.logger);
      for (auto j : i) all.ignore_linear[static_cast<size_t>(static_cast<unsigned char>(j))] = true;
    }

    if (!all.quiet)
    {
      *(all.trace_message) << "ignoring linear terms for namespaces beginning with:";
      for (size_t i = 0; i < NUM_NAMESPACES; ++i)
      {
        if (all.ignore_linear[i]) *(all.trace_message) << " " << static_cast<unsigned char>(i);
      }
      *(all.trace_message) << endl;
    }
  }

  if (options.was_supplied("keep"))
  {
    for (size_t i = 0; i < NUM_NAMESPACES; i++) all.ignore[i] = true;

    all.ignore_some = true;

    for (auto& i : keeps)
    {
      i = VW::decode_inline_hex(i, all.logger);
      for (const auto& j : i) all.ignore[static_cast<size_t>(static_cast<unsigned char>(j))] = false;
    }

    if (!all.quiet)
    {
      *(all.trace_message) << "using namespaces beginning with:";
      for (size_t i = 0; i < NUM_NAMESPACES; ++i)
      {
        if (!all.ignore[i]) *(all.trace_message) << " " << static_cast<unsigned char>(i);
      }
      *(all.trace_message) << endl;
    }
  }

  // --redefine param code
  all.redefine_some = false;  // false by default

  if (options.was_supplied("redefine"))
  {
    // initial values: i-th namespace is redefined to i itself
    for (size_t i = 0; i < NUM_NAMESPACES; i++) all.redefine[i] = static_cast<unsigned char>(i);

    // note: --redefine declaration order is matter
    // so --redefine :=L --redefine ab:=M  --ignore L  will ignore all except a and b under new M namspace

    for (const auto& arg : redefines)
    {
      const std::string& argument = VW::decode_inline_hex(arg, all.logger);
      size_t arg_len = argument.length();

      size_t operator_pos = 0;  // keeps operator pos + 1 to stay unsigned type
      bool operator_found = false;
      unsigned char new_namespace = ' ';

      // let's find operator ':=' position in N:=S
      for (size_t i = 0; i < arg_len; i++)
      {
        if (operator_found)
        {
          if (i > 2) { new_namespace = argument[0]; }  // N is not empty
          break;
        }
        else if (argument[i] == ':')
          operator_pos = i + 1;
        else if ((argument[i] == '=') && (operator_pos == i))
          operator_found = true;
      }

      if (!operator_found) THROW("argument of --redefine is malformed. Valid format is N:=S, :=S or N:=")

      if (++operator_pos > 3)  // seek operator end
      {
        all.logger.err_warn(
            "Multiple namespaces are used in target part of --redefine argument. Only first one ('{}') will be used as "
            "target namespace.",
            new_namespace);
      }
      all.redefine_some = true;

      // case ':=S' doesn't require any additional code as new_namespace = ' ' by default

      if (operator_pos == arg_len)  // S is empty, default namespace shall be used
        all.redefine[static_cast<int>(' ')] = new_namespace;
      else
        for (size_t i = operator_pos; i < arg_len; i++)
        {
          // all namespaces from S are redefined to N
          unsigned char c = argument[i];
          if (c != ':')
            all.redefine[c] = new_namespace;
          else
          {
            // wildcard found: redefine all except default and break
            for (size_t j = 0; j < NUM_NAMESPACES; j++) all.redefine[j] = new_namespace;
            break;  // break processing S
          }
        }
    }
  }

  if (options.was_supplied("dictionary"))
  {
    if (options.was_supplied("dictionary_path"))
      for (const std::string& path : dictionary_path)
        if (directory_exists(path)) all.dictionary_path.push_back(path);
    if (directory_exists(".")) all.dictionary_path.emplace_back(".");

#if _WIN32
    std::string PATH;
    char* buf;
    size_t buf_size;
    auto err = _dupenv_s(&buf, &buf_size, "PATH");
    if (!err && buf_size != 0)
    {
      PATH = std::string(buf, buf_size);
      free(buf);
    }
    const char delimiter = ';';
#else
    const std::string PATH = getenv("PATH");
    const char delimiter = ':';
#endif
    if (!PATH.empty())
    {
      size_t previous = 0;
      size_t index = PATH.find(delimiter);
      while (index != std::string::npos)
      {
        all.dictionary_path.push_back(PATH.substr(previous, index - previous));
        previous = index + 1;
        index = PATH.find(delimiter, previous);
      }
      all.dictionary_path.push_back(PATH.substr(previous));
    }
  }

  if (noconstant) all.add_constant = false;
}

void parse_example_tweaks(options_i& options, VW::workspace& all)
{
  std::string named_labels;
  std::string loss_function;
  float loss_parameter = 0.0;
  size_t early_terminate_passes;
  bool test_only = false;

  option_group_definition example_options("Example");
  example_options.add(make_option("testonly", test_only).short_name("t").help("Ignore label information and just test"))
      .add(make_option("holdout_off", all.holdout_set_off).help("No holdout data in multiple passes"))
      .add(make_option("holdout_period", all.holdout_period).default_value(10).help("Holdout period for test only"))
      .add(make_option("holdout_after", all.holdout_after)
               .help("Holdout after n training examples, default off (disables holdout_period)"))
      .add(
          make_option("early_terminate", early_terminate_passes)
              .default_value(3)
              .help(
                  "Specify the number of passes tolerated when holdout loss doesn't decrease before early termination"))
      .add(make_option("passes", all.numpasses).default_value(1).help("Number of Training Passes"))
      .add(make_option("initial_pass_length", all.pass_length)
               .default_value(std::numeric_limits<size_t>::max())
               .help("Initial number of examples per pass"))
      .add(make_option("examples", all.max_examples)
               .default_value(std::numeric_limits<size_t>::max())
               .help("Number of examples to parse"))
      .add(make_option("min_prediction", all.sd->min_label).help("Smallest prediction to output"))
      .add(make_option("max_prediction", all.sd->max_label).help("Largest prediction to output"))
      .add(make_option("sort_features", all.example_parser->sort_features)
               .help("Turn this on to disregard order in which features have been defined. This will lead to smaller "
                     "cache sizes"))
      .add(make_option("loss_function", loss_function)
               .default_value("squared")
               .one_of({"squared", "classic", "hinge", "logistic", "quantile", "poisson"})
               .help("Specify the loss function to be used, uses squared by default"))
      .add(make_option("quantile_tau", loss_parameter)
               .default_value(0.5f)
               .help("Parameter \\tau associated with Quantile loss. Defaults to 0.5"))
      .add(make_option("l1", all.l1_lambda).default_value(0.0f).help("L_1 lambda"))
      .add(make_option("l2", all.l2_lambda).default_value(0.0f).help("L_2 lambda"))
      .add(make_option("no_bias_regularization", all.no_bias).help("No bias in regularization"))
      .add(make_option("named_labels", named_labels)
               .keep()
               .help("Use names for labels (multiclass, etc.) rather than integers, argument specified all possible "
                     "labels, comma-sep, eg \"--named_labels Noun,Verb,Adj,Punc\""));
  options.add_and_parse(example_options);

  if (test_only || all.eta == 0.)
  {
    if (!all.quiet) *(all.trace_message) << "only testing" << endl;
    all.training = false;
    if (all.lda > 0) all.eta = 0;
  }
  else
    all.training = true;

  if ((all.numpasses > 1 || all.holdout_after > 0) && !all.holdout_set_off)
    all.holdout_set_off = false;  // holdout is on unless explicitly off
  else
    all.holdout_set_off = true;

  if (options.was_supplied("min_prediction") || options.was_supplied("max_prediction") || test_only)
    all.set_minmax = noop_mm;

  if (options.was_supplied("named_labels"))
  {
    all.sd->ldict = VW::make_unique<VW::named_labels>(named_labels);
    if (!all.quiet) *(all.trace_message) << "parsed " << all.sd->ldict->getK() << " named labels" << endl;
  }

  all.loss = getLossFunction(all, loss_function, loss_parameter);
  if (options.was_supplied("quantile_tau") && all.loss->getType() != "quantile")
  {
    all.logger.err_warn(
        "Option 'quantile_tau' was passed but the quantile loss function is not being used. 'quantile_tau' value will "
        "be ignored.");
  }

  if (all.l1_lambda < 0.f)
  {
    *(all.trace_message) << "l1_lambda should be nonnegative: resetting from " << all.l1_lambda << " to 0" << endl;
    all.l1_lambda = 0.f;
  }
  if (all.l2_lambda < 0.f)
  {
    *(all.trace_message) << "l2_lambda should be nonnegative: resetting from " << all.l2_lambda << " to 0" << endl;
    all.l2_lambda = 0.f;
  }
  all.reg_mode += (all.l1_lambda > 0.) ? 1 : 0;
  all.reg_mode += (all.l2_lambda > 0.) ? 2 : 0;
  if (!all.quiet)
  {
    if (all.reg_mode % 2 && !options.was_supplied("bfgs"))
      *(all.trace_message) << "using l1 regularization = " << all.l1_lambda << endl;
    if (all.reg_mode > 1) *(all.trace_message) << "using l2 regularization = " << all.l2_lambda << endl;
  }
}

void parse_update_options(options_i& options, VW::workspace& all)
{
  option_group_definition update_args("Update");
  update_args
      .add(make_option("learning_rate", all.eta)
               .default_value(0.5f)
               .keep(all.save_resume)
               .allow_override(all.save_resume)
               .help("Set learning rate")
               .short_name("l"))
      .add(make_option("power_t", all.power_t)
               .default_value(0.5f)
               .keep(all.save_resume)
               .allow_override(all.save_resume)
               .help("T power value"))
      .add(make_option("decay_learning_rate", all.eta_decay_rate)
               .default_value(1.f)
               .help("Set Decay factor for learning_rate between passes"))
      .add(make_option("initial_t", all.sd->t).help("Initial t value"))
      .add(make_option("feature_mask", all.feature_mask)
               .help("Use existing regressor to determine which parameters may be updated.  If no initial_regressor "
                     "given, also used for initial weights."));

  options.add_and_parse(update_args);
  all.initial_t = static_cast<float>(all.sd->t);
}

void parse_output_preds(options_i& options, VW::workspace& all)
{
  std::string predictions;
  std::string raw_predictions;

  option_group_definition output_options("Prediction Output");
  output_options.add(make_option("predictions", predictions).short_name("p").help("File to output predictions to"))
      .add(make_option("raw_predictions", raw_predictions)
               .short_name("r")
               .help("File to output unnormalized predictions to"));
  options.add_and_parse(output_options);

  if (options.was_supplied("predictions"))
  {
    if (!all.quiet) *(all.trace_message) << "predictions = " << predictions << endl;

    if (predictions == "stdout")
    {
      all.final_prediction_sink.push_back(VW::io::open_stdout());  // stdout
    }
    else
    {
      try
      {
        all.final_prediction_sink.push_back(VW::io::open_file_writer(predictions));
      }
      catch (...)
      {
        all.logger.err_error("Error opening the predictions file: {}", predictions);
      }
    }
  }

  if (options.was_supplied("raw_predictions"))
  {
    if (!all.quiet)
    {
      *(all.trace_message) << "raw predictions = " << raw_predictions << endl;
      if (options.was_supplied("binary"))
      { all.logger.err_warn("--raw_predictions has no defined value when --binary specified, expect no output"); }
    }
    if (raw_predictions == "stdout") { all.raw_prediction = VW::io::open_stdout(); }
    else
    {
      all.raw_prediction = VW::io::open_file_writer(raw_predictions);
    }
  }
}

void parse_output_model(options_i& options, VW::workspace& all)
{
  bool predict_only_model = false;
  bool save_resume = false;
  option_group_definition output_model_options("Output Model");
  output_model_options
      .add(make_option("final_regressor", all.final_regressor_name).short_name("f").help("Final regressor"))
      .add(make_option("readable_model", all.text_regressor_name)
               .help("Output human-readable final regressor with numeric features"))
      .add(make_option("invert_hash", all.inv_hash_regressor_name)
               .help("Output human-readable final regressor with feature names.  Computationally expensive"))
      .add(
          make_option("predict_only_model", predict_only_model)
              .help("Do not save extra state for learning to be resumed. Stored model can only be used for prediction"))
      .add(make_option("save_resume", save_resume)
               .help("This flag is now deprecated and models can continue learning by default"))
      .add(make_option("preserve_performance_counters", all.preserve_performance_counters)
               .help("Reset performance counters when warmstarting"))
      .add(make_option("save_per_pass", all.save_per_pass).help("Save the model after every pass over data"))
      .add(make_option("output_feature_regularizer_binary", all.per_feature_regularizer_output)
               .help("Per feature regularization output file"))
      .add(make_option("output_feature_regularizer_text", all.per_feature_regularizer_text)
               .help("Per feature regularization output file, in text"))
      .add(make_option("id", all.id).help("User supplied ID embedded into the final regressor"));
  options.add_and_parse(output_model_options);

  if (!all.final_regressor_name.empty() && !all.quiet)
    *(all.trace_message) << "final_regressor = " << all.final_regressor_name << endl;

  if (options.was_supplied("invert_hash")) { all.hash_inv = true; }
  if (save_resume)
  {
    all.logger.err_warn("--save_resume flag is deprecated -- learning can now continue on saved models by default.");
  }
  if (predict_only_model) { all.save_resume = false; }

  // Question: This doesn't seem necessary
  // if (options.was_supplied("id") && find(arg.args.begin(), arg.args.end(), "--id") == arg.args.end())
  // {
  //   arg.args.push_back("--id");
  //   arg.args.push_back(arg.vm["id"].as<std::string>());
  // }
}

void load_input_model(VW::workspace& all, io_buf& io_temp)
{
  // Need to see if we have to load feature mask first or second.
  // -i and -mask are from same file, load -i file first so mask can use it
  if (!all.feature_mask.empty() && !all.initial_regressors.empty() && all.feature_mask == all.initial_regressors[0])
  {
    // load rest of regressor
    all.l->save_load(io_temp, true, false);
    io_temp.close_file();

    parse_mask_regressor_args(all, all.feature_mask, all.initial_regressors);
  }
  else
  {  // load mask first
    parse_mask_regressor_args(all, all.feature_mask, all.initial_regressors);

    // load rest of regressor
    all.l->save_load(io_temp, true, false);
    io_temp.close_file();
  }
}

ssize_t trace_message_wrapper_adapter(void* context, const char* buffer, size_t num_bytes)
{
  auto* wrapper_context = reinterpret_cast<trace_message_wrapper*>(context);
  const std::string str(buffer, num_bytes);
  wrapper_context->_trace_message(wrapper_context->_inner_context, str);
  return static_cast<ssize_t>(num_bytes);
}

VW::workspace& parse_args(
    std::unique_ptr<options_i, options_deleter_type> options, trace_message_t trace_listener, void* trace_context)
{
  auto logger = trace_listener != nullptr ? VW::io::create_custom_sink_logger(trace_context, trace_listener)
                                          : VW::io::create_default_logger();

  bool quiet = false;
  bool driver_output_off = false;
  std::string driver_output_stream;
  std::string log_level;
  std::string log_output_stream;
  size_t upper_limit = 0;
  option_group_definition logging_options("Logging");
  logging_options
      .add(make_option("quiet", quiet)
               .help("Don't output diagnostics and progress updates. Supplying this implies --log_level off and "
                     "--driver_output_off. Supplying this overrides an explicit log_level argument."))
      .add(make_option("driver_output_off", driver_output_off).help("Disable output for the driver"))
      .add(make_option("driver_output", driver_output_stream)
               .default_value("stderr")
               .one_of({"stdout", "stderr"})
               .help("Specify the stream to output driver output to"))
      .add(make_option("log_level", log_level)
               .default_value("info")
               .one_of({"info", "warn", "error", "critical", "off"})
               .help("Log level for logging messages. Specifying this wil override --quiet for log output"))
      .add(make_option("log_output", log_output_stream)
               .default_value("stdout")
               .one_of({"stdout", "stderr", "compat"})
               .help("Specify the stream to output log messages to. In the past VW's choice of stream for logging "
                     "messages wasn't consistent. Supplying compat will maintain that old behavior. Compat is now "
                     "deprecated so it is recommended that stdout or stderr is chosen."))
      .add(make_option("limit_output", upper_limit)
               .default_value(0)
               .help("Avoid chatty output. Limit total printed lines. 0 means unbounded"));

  options->add_and_parse(logging_options);

  if (quiet)
  {
    log_level = "off";
    driver_output_off = true;
  }

  auto level = VW::io::get_log_level(log_level);
  logger.set_level(level);
  auto location = VW::io::get_output_location(log_output_stream);
  logger.set_location(location);

  // Don't print warning if a custom log output trace_listener is supplied.
  if (trace_listener == nullptr && location == VW::io::output_location::compat)
  { logger.err_warn("'compat' mode for --log_output is deprecated and will be removed in a future release."); }

  if (options->was_supplied("limit_output") && (upper_limit != 0)) { logger.set_max_output(upper_limit); }

  VW::workspace& all = *(new VW::workspace(logger));
  all.options = std::move(options);
  all.quiet = quiet;

  if (driver_output_off)
  {
    // This is valid:
    // https://stackoverflow.com/questions/25690636/is-it-valid-to-construct-an-stdostream-from-a-null-buffer This
    // results in the ostream not outputting anything.
    all.trace_message = VW::make_unique<std::ostream>(nullptr);
  }
  else
  {
    if (trace_listener != nullptr)
    {
      if (all.options->was_supplied("log_output"))
      {
        all.logger.err_warn(
            "--log_output option is unused. This is because when a custom trace_listener is being used.");
      }

      // Since the trace_message_t interface uses a string and the writer interface uses a buffer we unfortunately
      // need to adapt between them here.
      all.trace_message_wrapper_context = std::make_shared<trace_message_wrapper>(trace_context, trace_listener);
      all.trace_message = VW::make_unique<VW::io::owning_ostream>(VW::make_unique<VW::io::writer_stream_buf>(
          VW::io::create_custom_writer(all.trace_message_wrapper_context.get(), trace_message_wrapper_adapter)));
    }
    else if (driver_output_stream == "stdout")
    {
      all.trace_message = VW::make_unique<std::ostream>(std::cout.rdbuf());
    }
    else
    {
      all.trace_message = VW::make_unique<std::ostream>(std::cerr.rdbuf());
    }
  }

  try
  {
    time(&all.init_time);

    bool strict_parse = false;
    int ring_size_tmp;
    int64_t example_queue_limit_tmp;
    option_group_definition vw_args("Parser");
    vw_args.add(make_option("ring_size", ring_size_tmp).default_value(256).help("Size of example ring"))
        .add(make_option("example_queue_limit", example_queue_limit_tmp)
                 .default_value(256)
                 .help("Max number of examples to store after parsing but before the learner has processed. Rarely "
                       "needs to be changed."))
        .add(make_option("strict_parse", strict_parse).help("Throw on malformed examples"));
    all.options->add_and_parse(vw_args);

    if (ring_size_tmp <= 0) { THROW("ring_size should be positive") }
    if (example_queue_limit_tmp <= 0) { THROW("ring_size should be positive") }
    auto ring_size = static_cast<size_t>(ring_size_tmp);
    auto example_queue_limit = static_cast<size_t>(example_queue_limit_tmp);
    auto final_example_queue_limit = example_queue_limit;
    if (all.options->was_supplied("ring_size"))
    {
      final_example_queue_limit = ring_size;
      all.logger.err_warn("--ring_size is deprecated and has been replaced with --example_queue_limit");
      if (all.options->was_supplied("example_queue_limit"))
      {
        final_example_queue_limit = example_queue_limit;
        all.logger.err_info("--example_queue_limit overrides --ring_size");
      }
    }

    all.example_parser = new parser{final_example_queue_limit, strict_parse};
    all.example_parser->_shared_data = all.sd;

    option_group_definition weight_args("Weight");
    weight_args
        .add(make_option("initial_regressor", all.initial_regressors).help("Initial regressor(s)").short_name("i"))
        .add(make_option("initial_weight", all.initial_weight)
                 .default_value(0.f)
                 .help("Set all weights to an initial value of arg"))
        .add(make_option("random_weights", all.random_weights).help("Make initial weights random"))
        .add(make_option("normal_weights", all.normal_weights).help("Make initial weights normal"))
        .add(make_option("truncated_normal_weights", all.tnormal_weights).help("Make initial weights truncated normal"))
        .add(make_option("sparse_weights", all.weights.sparse).help("Use a sparse datastructure for weights"))
        .add(make_option("input_feature_regularizer", all.per_feature_regularizer_input)
                 .help("Per feature regularization input file"));
    all.options->add_and_parse(weight_args);

    std::string span_server_arg;
    int span_server_port_arg;
    // bool threads_arg;
    size_t unique_id_arg;
    size_t total_arg;
    size_t node_arg;
    option_group_definition parallelization_args("Parallelization");
    parallelization_args
        .add(make_option("span_server", span_server_arg).help("Location of server for setting up spanning tree"))
        //(make_option("threads", threads_arg).help("Enable multi-threading")) Unused option?
        .add(make_option("unique_id", unique_id_arg).default_value(0).help("Unique id used for cluster parallel jobs"))
        .add(
            make_option("total", total_arg).default_value(1).help("Total number of nodes used in cluster parallel job"))
        .add(make_option("node", node_arg).default_value(0).help("Node number in cluster parallel job"))
        .add(make_option("span_server_port", span_server_port_arg)
                 .default_value(26543)
                 .help("Port of the server for setting up spanning tree"));
    all.options->add_and_parse(parallelization_args);

    // total, unique_id and node must be specified together.
    if ((all.options->was_supplied("total") || all.options->was_supplied("node") ||
            all.options->was_supplied("unique_id")) &&
        !(all.options->was_supplied("total") && all.options->was_supplied("node") &&
            all.options->was_supplied("unique_id")))
    { THROW("unique_id, total, and node must be all be specified if any are specified.") }

    if (all.options->was_supplied("span_server"))
    {
      all.all_reduce_type = AllReduceType::Socket;
      all.all_reduce =
          new AllReduceSockets(span_server_arg, span_server_port_arg, unique_id_arg, total_arg, node_arg, all.quiet);
    }

    parse_diagnostics(*all.options, all);

    return all;
  }
  catch (...)
  {
    VW::finish(all);
    throw;
  }
}

bool check_interaction_settings_collision(options_i& options, const std::string& file_options)
{
  const bool command_line_has_interaction = options.was_supplied("q") || options.was_supplied("quadratic") ||
      options.was_supplied("cubic") || options.was_supplied("interactions");

  if (!command_line_has_interaction) return false;

  // we don't use -q to save pairs in all.file_options, so only 3 options checked
  bool file_options_has_interaction = file_options.find("--quadratic") != std::string::npos;
  file_options_has_interaction = file_options_has_interaction || (file_options.find("--cubic") != std::string::npos);
  file_options_has_interaction =
      file_options_has_interaction || (file_options.find("--interactions") != std::string::npos);

  return file_options_has_interaction;
}

void merge_options_from_header_strings(const std::vector<std::string>& strings, bool skip_interactions,
    VW::config::options_i& options, bool& is_ccb_input_model)
{
  po::options_description desc("");

  // Get list of options in file options std::string
  po::parsed_options pos = po::command_line_parser(strings).options(desc).allow_unregistered().run();

  bool skipping = false;
  std::string saved_key = "";
  unsigned int count = 0;
  bool first_seen = false;

  for (auto opt : pos.options)
  {
    // If we previously encountered an option we want to skip, ignore tokens without --.
    if (skipping)
    {
      for (const auto& token : opt.original_tokens)
      {
        auto found = token.find("--");
        if (found != std::string::npos) { skipping = false; }
      }

      if (skipping)
      {
        saved_key = "";
        continue;
      }
    }

    bool treat_as_value = false;
    // If the key starts with a digit, this is a mis-interpretation of a value as a key. Pull it into the previous
    // option. This was found in the case of --lambda -1, misinterpreting -1 as an option key. The easy way to fix this
    // requires introducing "identifier-like" semantics for options keys, e.g. "does not begin with a digit". That does
    // not seem like an unreasonable restriction. The logical check here is: is "string_key" of the form {'-', <digit>,
    // <etc.>}.
    if (opt.string_key.length() > 1 && opt.string_key[0] == '-' && opt.string_key[1] >= '0' && opt.string_key[1] <= '9')
    { treat_as_value = true; }

    // File options should always use long form.

    // If the key is empty this must be a value, otherwise set the key.
    if (!treat_as_value && !opt.string_key.empty())
    {
      // If the new token is a new option and there were no values previously it was a bool option. Add it as a switch.
      if (count == 0 && first_seen) { options.insert(saved_key, ""); }

      count = 0;
      first_seen = true;

      // If the interaction settings are doubled, the copy in the model file is ignored.
      if (skip_interactions &&
          (opt.string_key == "quadratic" || opt.string_key == "cubic" || opt.string_key == "interactions"))
      {
        // skip this option.
        skipping = true;
        first_seen = false;
        continue;
      }

      saved_key = opt.string_key;
      is_ccb_input_model = is_ccb_input_model || (saved_key == "ccb_explore_adf");

      if (!opt.value.empty())
      {
        for (const auto& value : opt.value)
        {
          options.insert(saved_key, value);
          count++;
        }
      }
    }
    else
    {
      // If treat_as_value is set, boost incorrectly interpreted the token as containing an option key
      // In this case, what should have happened is all original_tokens items should be in value.
      auto source = treat_as_value ? opt.original_tokens : opt.value;
      for (const auto& value : source)
      {
        options.insert(saved_key, value);
        count++;
      }
    }
  }

  if (count == 0 && !saved_key.empty()) { options.insert(saved_key, ""); }
}

options_i& load_header_merge_options(
    options_i& options, VW::workspace& all, io_buf& model, bool& interactions_settings_duplicated)
{
  std::string file_options;
  save_load_header(all, model, true, false, file_options, options);

  interactions_settings_duplicated = check_interaction_settings_collision(options, file_options);

  // Convert file_options into vector.
  std::istringstream ss{file_options};
  const std::vector<std::string> container{
      std::istream_iterator<std::string>{ss}, std::istream_iterator<std::string>{}};

  merge_options_from_header_strings(container, interactions_settings_duplicated, options, all.is_ccb_input_model);

  return options;
}

void parse_modules(options_i& options, VW::workspace& all, bool interactions_settings_duplicated,
    std::vector<std::string>& dictionary_namespaces)
{
  option_group_definition rand_options("Randomization");
  rand_options.add(make_option("random_seed", all.random_seed).default_value(0).help("Seed random number generator"));
  options.add_and_parse(rand_options);
  all.get_random_state()->set_random_state(all.random_seed);

  parse_feature_tweaks(options, all, interactions_settings_duplicated, dictionary_namespaces);  // feature tweaks

  parse_example_tweaks(options, all);  // example manipulation

  parse_output_model(options, all);

  parse_update_options(options, all);

  parse_output_preds(options, all);
}

// note: Although we have the option to override setup_base_i,
// the most common scenario is to use the default_reduction_stack_setup.
// Expect learner_builder to be nullptr most/all of the cases.
void instantiate_learner(VW::workspace& all, std::unique_ptr<VW::setup_base_i> learner_builder)
{
  if (!learner_builder)
  { learner_builder = VW::make_unique<VW::default_reduction_stack_setup>(all, *all.options.get()); }
  else
  {
    learner_builder->delayed_state_attach(all, *all.options.get());
  }

  // kick-off reduction setup functions
  all.l = learner_builder->setup_base_learner();

  // explicit destroy of learner_builder state
  // avoids misuse of this interface:
  learner_builder.reset();
  assert(learner_builder == nullptr);
}

void parse_sources(options_i& options, VW::workspace& all, io_buf& model, bool skip_model_load)
{
  if (!skip_model_load)
    load_input_model(all, model);
  else
    model.close_file();

  auto parsed_source_options = parse_source(all, options);
  enable_sources(all, all.quiet, all.numpasses, parsed_source_options);

  // force wpp to be a power of 2 to avoid 32-bit overflow
  uint32_t i = 0;
  const size_t params_per_problem = all.l->increment;
  while (params_per_problem > (static_cast<uint64_t>(1) << i)) i++;
  all.wpp = (1 << i) >> all.weights.stride_shift();
}

namespace VW
{
void cmd_string_replace_value(std::stringstream*& ss, std::string flag_to_replace, const std::string& new_value)
{
  flag_to_replace.append(
      " ");  // add a space to make sure we obtain the right flag in case 2 flags start with the same set of characters
  std::string cmd = ss->str();
  size_t pos = cmd.find(flag_to_replace);
  if (pos == std::string::npos)
    // flag currently not present in command string, so just append it to command string
    *ss << " " << flag_to_replace << new_value;
  else
  {
    // flag is present, need to replace old value with new value

    // compute position after flag_to_replace
    pos += flag_to_replace.size();

    // now pos is position where value starts
    // find position of next space
    const size_t pos_after_value = cmd.find(' ', pos);
    if (pos_after_value == std::string::npos)
    {
      // we reach the end of the std::string, so replace the all characters after pos by new_value
      cmd.replace(pos, cmd.size() - pos, new_value);
    }
    else
    {
      // replace characters between pos and pos_after_value by new_value
      cmd.replace(pos, pos_after_value - pos, new_value);
    }

    ss->str(cmd);
  }
}

char** to_argv_escaped(std::string const& s, int& argc)
{
  std::vector<std::string> tokens = escaped_tokenize(' ', s);
  char** argv = calloc_or_throw<char*>(tokens.size() + 1);
  argv[0] = calloc_or_throw<char>(2);
  argv[0][0] = 'b';
  argv[0][1] = '\0';

  for (size_t i = 0; i < tokens.size(); i++)
  {
    argv[i + 1] = calloc_or_throw<char>(tokens[i].length() + 1);
    sprintf_s(argv[i + 1], (tokens[i].length() + 1), "%s", tokens[i].data());
  }

  argc = static_cast<int>(tokens.size() + 1);
  return argv;
}

char** to_argv(std::string const& s, int& argc)
{
  const VW::string_view strview(s);
  std::vector<VW::string_view> foo;
  tokenize(' ', strview, foo);

  char** argv = calloc_or_throw<char*>(foo.size() + 1);
  // small optimization to avoid a string copy before tokenizing
  argv[0] = calloc_or_throw<char>(2);
  argv[0][0] = 'b';
  argv[0][1] = '\0';
  for (size_t i = 0; i < foo.size(); i++)
  {
    const size_t len = foo[i].length();
    argv[i + 1] = calloc_or_throw<char>(len + 1);
    memcpy(argv[i + 1], foo[i].data(), len);
    // copy() is supported with boost::string_view, not with string_ref
    // foo[i].copy(argv[i], len);
    // unnecessary because of the calloc, but needed if we change stuff in the future
    // argv[i][len] = '\0';
  }

  argc = static_cast<int>(foo.size()) + 1;
  return argv;
}

void free_args(int argc, char* argv[])
{
  for (int i = 0; i < argc; i++) free(argv[i]);
  free(argv);
}

void print_enabled_reductions(VW::workspace& all, std::vector<std::string>& enabled_reductions)
{
  // output list of enabled reductions
  if (!all.quiet && !all.options->was_supplied("audit_regressor") && !enabled_reductions.empty())
  {
    const char* const delim = ", ";
    std::ostringstream imploded;
    std::copy(
        enabled_reductions.begin(), enabled_reductions.end() - 1, std::ostream_iterator<std::string>(imploded, delim));

    *(all.trace_message) << "Enabled reductions: " << imploded.str() << enabled_reductions.back() << std::endl;
  }
}

VW::workspace* initialize(config::options_i& options, io_buf* model, bool skip_model_load,
    trace_message_t trace_listener, void* trace_context)
{
  std::unique_ptr<options_i, options_deleter_type> opts(&options, [](VW::config::options_i*) {});

  return initialize(std::move(opts), model, skip_model_load, trace_listener, trace_context);
}

VW::workspace* initialize_with_builder(std::unique_ptr<options_i, options_deleter_type> options, io_buf* model,
    bool skip_model_load, trace_message_t trace_listener, void* trace_context,
    std::unique_ptr<VW::setup_base_i> learner_builder = nullptr)
{
  // Set up logger as early as possible
  VW::workspace& all = parse_args(std::move(options), trace_listener, trace_context);

  try
  {
    // if user doesn't pass in a model, read from options
    io_buf local_model;
    if (!model)
    {
      std::vector<std::string> all_initial_regressor_files(all.initial_regressors);
      if (all.options->was_supplied("input_feature_regularizer"))
      { all_initial_regressor_files.push_back(all.per_feature_regularizer_input); }
      read_regressor_file(all, all_initial_regressor_files, local_model);
      model = &local_model;
    }

    // Loads header of model files and loads the command line options into the options object.
    bool interactions_settings_duplicated;
    load_header_merge_options(*all.options, all, *model, interactions_settings_duplicated);

    std::vector<std::string> dictionary_namespaces;
    parse_modules(*all.options, all, interactions_settings_duplicated, dictionary_namespaces);
    instantiate_learner(all, std::move(learner_builder));
    parse_sources(*all.options, all, *model, skip_model_load);
    if (!all.quiet)
    {
      *(all.trace_message) << "Num weight bits = " << all.num_bits << endl;
      *(all.trace_message) << "learning rate = " << all.eta << endl;
      *(all.trace_message) << "initial_t = " << all.sd->t << endl;
      *(all.trace_message) << "power_t = " << all.power_t << endl;
      if (all.numpasses > 1) *(all.trace_message) << "decay_learning_rate = " << all.eta_decay_rate << endl;
    }

    // we must delay so parse_mask is fully defined.
    for (const auto& name_space : dictionary_namespaces) parse_dictionary_argument(all, name_space);

    all.options->check_unregistered(all.logger);

    std::vector<std::string> enabled_reductions;
    if (all.l != nullptr) all.l->get_enabled_reductions(enabled_reductions);

    // upon direct query for help -- spit it out to stdout;
    if (all.options->get_typed_option<bool>("help").value())
    {
      std::cout << all.options->help(enabled_reductions);
      exit(0);
    }

    print_enabled_reductions(all, enabled_reductions);

    if (!all.quiet)
    {
      *(all.trace_message) << "Input label = " << VW::to_string(all.l->get_input_label_type()).substr(14) << std::endl;
      *(all.trace_message) << "Output pred = " << VW::to_string(all.l->get_output_prediction_type()).substr(19)
                           << std::endl;
    }

    if (!all.options->get_typed_option<bool>("dry_run").value())
    {
      if (!all.quiet && !all.bfgs && !all.searchstr && !all.options->was_supplied("audit_regressor"))
      { all.sd->print_update_header(*all.trace_message); }
      all.l->init_driver();
    }

    return &all;
  }
  catch (VW::save_load_model_exception& e)
  {
    auto msg = fmt::format("{}, model files = {}", e.what(), fmt::join(all.initial_regressors, ", "));
    delete &all;
    throw save_load_model_exception(e.Filename(), e.LineNumber(), msg);
  }
  catch (...)
  {
    delete &all;
    throw;
  }
}

VW::workspace* initialize(std::unique_ptr<options_i, options_deleter_type> options, io_buf* model, bool skip_model_load,
    trace_message_t trace_listener, void* trace_context)
{
  return initialize_with_builder(std::move(options), model, skip_model_load, trace_listener, trace_context, nullptr);
}

VW::workspace* initialize_escaped(
    std::string const& s, io_buf* model, bool skip_model_load, trace_message_t trace_listener, void* trace_context)
{
  int argc = 0;
  char** argv = to_argv_escaped(s, argc);
  VW::workspace* ret = nullptr;

  try
  {
    ret = initialize(argc, argv, model, skip_model_load, trace_listener, trace_context);
  }
  catch (...)
  {
    free_args(argc, argv);
    throw;
  }

  free_args(argc, argv);
  return ret;
}

VW::workspace* initialize_with_builder(int argc, char* argv[], io_buf* model, bool skip_model_load,
    trace_message_t trace_listener, void* trace_context, std::unique_ptr<VW::setup_base_i> learner_builder)
{
  std::unique_ptr<options_i, options_deleter_type> options(
      new config::options_boost_po(argc, argv), [](VW::config::options_i* ptr) { delete ptr; });
  return initialize_with_builder(
      std::move(options), model, skip_model_load, trace_listener, trace_context, std::move(learner_builder));
}

VW::workspace* initialize(
    int argc, char* argv[], io_buf* model, bool skip_model_load, trace_message_t trace_listener, void* trace_context)
{
  return initialize_with_builder(argc, argv, model, skip_model_load, trace_listener, trace_context, nullptr);
}

VW::workspace* initialize_with_builder(const std::string& s, io_buf* model, bool skip_model_load,
    trace_message_t trace_listener, void* trace_context, std::unique_ptr<VW::setup_base_i> learner_builder)
{
  int argc = 0;
  char** argv = to_argv(s, argc);
  VW::workspace* ret = nullptr;

  try
  {
    ret = initialize_with_builder(
        argc, argv, model, skip_model_load, trace_listener, trace_context, std::move(learner_builder));
  }
  catch (...)
  {
    free_args(argc, argv);
    throw;
  }

  free_args(argc, argv);
  return ret;
}

VW::workspace* initialize(
    const std::string& s, io_buf* model, bool skip_model_load, trace_message_t trace_listener, void* trace_context)
{
  return initialize_with_builder(s, model, skip_model_load, trace_listener, trace_context, nullptr);
}

// Create a new VW instance while sharing the model with another instance
// The extra arguments will be appended to those of the other VW instance
VW::workspace* seed_vw_model(
    VW::workspace* vw_model, const std::string& extra_args, trace_message_t trace_listener, void* trace_context)
{
  options_serializer_boost_po serializer;
  for (auto const& option : vw_model->options->get_all_options())
  {
    if (vw_model->options->was_supplied(option->m_name))
    {
      // ignore no_stdin since it will be added by vw::initialize, and ignore -i since we don't want to reload the
      // model.
      if (option->m_name == "no_stdin" || option->m_name == "initial_regressor") { continue; }

      serializer.add(*option);
    }
  }

  auto serialized_options = serializer.str();
  serialized_options = serialized_options + " " + extra_args;

  VW::workspace* new_model =
      VW::initialize(serialized_options, nullptr, true /* skip_model_load */, trace_listener, trace_context);
  free_it(new_model->sd);

  // reference model states stored in the specified VW instance
  new_model->weights.shallow_copy(vw_model->weights);  // regressor
  new_model->sd = vw_model->sd;                        // shared data
  new_model->example_parser->_shared_data = new_model->sd;

  return new_model;
}

void sync_stats(VW::workspace& all)
{
  if (all.all_reduce != nullptr)
  {
    const auto loss = static_cast<float>(all.sd->sum_loss);
    all.sd->sum_loss = static_cast<double>(accumulate_scalar(all, loss));
    const auto weighted_labeled_examples = static_cast<float>(all.sd->weighted_labeled_examples);
    all.sd->weighted_labeled_examples = static_cast<double>(accumulate_scalar(all, weighted_labeled_examples));
    const auto weighted_labels = static_cast<float>(all.sd->weighted_labels);
    all.sd->weighted_labels = static_cast<double>(accumulate_scalar(all, weighted_labels));
    const auto weighted_unlabeled_examples = static_cast<float>(all.sd->weighted_unlabeled_examples);
    all.sd->weighted_unlabeled_examples = static_cast<double>(accumulate_scalar(all, weighted_unlabeled_examples));
    const auto example_number = static_cast<float>(all.sd->example_number);
    all.sd->example_number = static_cast<uint64_t>(accumulate_scalar(all, example_number));
    const auto total_features = static_cast<float>(all.sd->total_features);
    all.sd->total_features = static_cast<uint64_t>(accumulate_scalar(all, total_features));
  }
}

void finish(VW::workspace& all, bool delete_all)
{
  // also update VowpalWabbit::PerformanceStatistics::get() (vowpalwabbit.cpp)
  if (!all.quiet && !all.options->was_supplied("audit_regressor"))
  {
    all.trace_message->precision(6);
    *(all.trace_message) << std::fixed;
    *(all.trace_message) << endl << "finished run";
    if (all.current_pass == 0 || all.current_pass == 1)
      *(all.trace_message) << endl << "number of examples = " << all.sd->example_number;
    else
    {
      *(all.trace_message) << endl << "number of examples per pass = " << all.sd->example_number / all.current_pass;
      *(all.trace_message) << endl << "passes used = " << all.current_pass;
    }
    *(all.trace_message) << endl << "weighted example sum = " << all.sd->weighted_examples();
    *(all.trace_message) << endl << "weighted label sum = " << all.sd->weighted_labels;
    *(all.trace_message) << endl << "average loss = ";
    if (all.holdout_set_off)
      if (all.sd->weighted_labeled_examples > 0)
        *(all.trace_message) << all.sd->sum_loss / all.sd->weighted_labeled_examples;
      else
        *(all.trace_message) << "n.a.";
    else if ((all.sd->holdout_best_loss == FLT_MAX) || (all.sd->holdout_best_loss == FLT_MAX * 0.5))
      *(all.trace_message) << "undefined (no holdout)";
    else
      *(all.trace_message) << all.sd->holdout_best_loss << " h";
    if (all.sd->report_multiclass_log_loss)
    {
      if (all.holdout_set_off)
        *(all.trace_message) << endl
                             << "average multiclass log loss = "
                             << all.sd->multiclass_log_loss / all.sd->weighted_labeled_examples;
      else
        *(all.trace_message) << endl
                             << "average multiclass log loss = "
                             << all.sd->holdout_multiclass_log_loss / all.sd->weighted_labeled_examples << " h";
    }

    float best_constant;
    float best_constant_loss;
    if (get_best_constant(all.loss.get(), all.sd, best_constant, best_constant_loss))
    {
      *(all.trace_message) << endl << "best constant = " << best_constant;
      if (best_constant_loss != FLT_MIN)
        *(all.trace_message) << endl << "best constant's loss = " << best_constant_loss;
    }

    *(all.trace_message) << endl << "total feature number = " << all.sd->total_features;
    if (all.sd->queries > 0) *(all.trace_message) << endl << "total queries = " << all.sd->queries;
    *(all.trace_message) << endl;
  }

  // implement finally.
  // finalize_regressor can throw if it can't write the file.
  // we still want to free up all the memory.
  std::exception_ptr finalize_regressor_exception;
  try
  {
    finalize_regressor(all, all.final_regressor_name);
  }
  catch (vw_exception& /* e */)
  {
    finalize_regressor_exception = std::current_exception();
  }

  metrics::output_metrics(all);
  all.logger.log_summary();

  if (delete_all) delete &all;

  if (finalize_regressor_exception) { std::rethrow_exception(finalize_regressor_exception); }
}
}  // namespace VW
