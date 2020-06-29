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

#include "parse_regressor.h"
#include "parser.h"
#include "parse_primitives.h"
#include "vw.h"
#include "interactions.h"

#include "sender.h"
#include "nn.h"
#include "gd.h"
#include "cbify.h"
#include "oaa.h"
#include "boosting.h"
#include "multilabel_oaa.h"
#include "rand48.h"
#include "bs.h"
#include "topk.h"
#include "ect.h"
#include "csoaa.h"
#include "cb_algs.h"
#include "cb_adf.h"
#include "cb_dro.h"
#include "cb_explore.h"
#include "cb_explore_adf_bag.h"
#include "cb_explore_adf_cover.h"
#include "cb_explore_adf_first.h"
#include "cb_explore_adf_greedy.h"
#include "cb_explore_adf_regcb.h"
#include "cb_explore_adf_rnd.h"
#include "cb_explore_adf_softmax.h"
#include "slates.h"
#include "mwt.h"
#include "confidence.h"
#include "scorer.h"
#include "expreplay.h"
#include "search.h"
#include "bfgs.h"
#include "lda_core.h"
#include "noop.h"
#include "print.h"
#include "gd_mf.h"
#include "learner.h"
#include "mf.h"
#include "ftrl.h"
#include "svrg.h"
#include "rand48.h"
#include "binary.h"
#include "lrq.h"
#include "lrqfa.h"
#include "autolink.h"
#include "log_multi.h"
#include "recall_tree.h"
#include "memory_tree.h"
#include "stagewise_poly.h"
#include "active.h"
#include "active_cover.h"
#include "cs_active.h"
#include "kernel_svm.h"
#include "parse_example.h"
#include "best_constant.h"
#include "interact.h"
#include "vw_exception.h"
#include "accumulate.h"
#include "vw_validate.h"
#include "vw_allreduce.h"
#include "OjaNewton.h"
#include "audit_regressor.h"
#include "marginal.h"
#include "explore_eval.h"
#include "baseline.h"
#include "classweight.h"
#include "cb_sample.h"
#include "warm_cb.h"
#include "shared_feature_merger.h"
// #include "cntk.h"

#include "options.h"
#include "options_boost_po.h"
#include "options_serializer_boost_po.h"

using std::cerr;
using std::cout;
using std::endl;
using namespace VW::config;

//
// Does std::string end with a certain substring?
//
bool ends_with(std::string const& fullString, std::string const& ending)
{
  if (fullString.length() > ending.length())
  {
    return (fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0);
  }
  else
  {
    return false;
  }
}

uint64_t hash_file_contents(VW::io::reader* f)
{
  uint64_t v = 5289374183516789128;
  char buf[1024];
  while (true)
  {
    ssize_t n = f->read(buf, 1024);
    if (n <= 0)
      break;
    for (ssize_t i = 0; i < n; i++)
    {
      v *= 341789041;
      v += static_cast<uint64_t>(buf[i]);
    }
  }
  return v;
}

bool directory_exists(std::string path)
{
  struct stat info;
  if (stat(path.c_str(), &info) != 0)
    return false;
  else
    return (info.st_mode & S_IFDIR) > 0;
  //  boost::filesystem::path p(path);
  //  return boost::filesystem::exists(p) && boost::filesystem::is_directory(p);
}

std::string find_in_path(std::vector<std::string> paths, std::string fname)
{
#ifdef _WIN32
  std::string delimiter = "\\";
#else
  std::string delimiter = "/";
#endif
  for (std::string path : paths)
  {
    std::string full = ends_with(path, delimiter) ? (path + fname) : (path + delimiter + fname);
    std::ifstream f(full.c_str());
    if (f.good())
      return full;
  }
  return "";
}

void parse_dictionary_argument(vw& all, const std::string& str)
{
  if (str.length() == 0)
    return;
  // expecting 'namespace:file', for instance 'w:foo.txt'
  // in the case of just 'foo.txt' it's applied to the default namespace

  char ns = ' ';
  VW::string_view s(str);
  if ((str.length() > 2) && (str[1] == ':'))
  {
    ns = str[0];
    s.remove_prefix(2);
  }

  std::string fname = find_in_path(all.dictionary_path, std::string(s));
  if (fname == "")
    THROW("error: cannot find dictionary '" << s << "' in path; try adding --dictionary_path");

  bool is_gzip = ends_with(fname, ".gz");
  std::unique_ptr<VW::io::reader> file_adapter;
  try
  {
    file_adapter = is_gzip ? VW::io::open_compressed_file_reader(fname) : VW::io::open_file_reader(fname);
  }
  catch (...)
  {
    THROW("error: cannot read dictionary from file '" << fname << "'"
                                                      << ", opening failed");
  }

  uint64_t fd_hash = hash_file_contents(file_adapter.get());

  if (!all.logger.quiet)
    all.oc.trace_message << "scanned dictionary '" << s << "' from '" << fname << "', hash=" << std::hex << fd_hash
                      << std::dec << endl;

  // see if we've already read this dictionary
  for (size_t id = 0; id < all.loaded_dictionaries.size(); id++)
  {
    if (all.loaded_dictionaries[id].file_hash == fd_hash)
    {
      all.namespace_dictionaries[(size_t)ns].push_back(all.loaded_dictionaries[id].dict);
      return;
    }
  }

  std::unique_ptr<VW::io::reader> fd;
  try
  {
    fd = VW::io::open_file_reader(fname);
  }
  catch (...)
  {
    THROW("error: cannot re-read dictionary from file '" << fname << "', opening failed");
  }
  auto map = std::make_shared<feature_dict>();
  // mimicing old v_hashmap behavior for load factor.
  // A smaller factor will generally use more memory but have faster access
  map->max_load_factor(0.25);
  example* ec = VW::alloc_examples(all.p->lp.label_size, 1);

  size_t def = (size_t)' ';

  ssize_t size = 2048, pos, nread;
  char rc;
  char* buffer = calloc_or_throw<char>(size);
  do
  {
    pos = 0;
    do
    {
      nread = fd->read(&rc, 1);
      if ((rc != EOF) && (nread > 0))
        buffer[pos++] = rc;
      if (pos >= size - 1)
      {
        size *= 2;
        const auto new_buffer = (char*)(realloc(buffer, size));
        if (new_buffer == nullptr)
        {
          free(buffer);
          VW::dealloc_example(all.p->lp.delete_label, *ec);
          free(ec);
          THROW("error: memory allocation failed in reading dictionary");
        }
        else
          buffer = new_buffer;
      }
    } while ((rc != EOF) && (rc != '\n') && (nread > 0));
    buffer[pos] = 0;

    // we now have a line in buffer
    char* c = buffer;
    while (*c == ' ' || *c == '\t') ++c;  // skip initial whitespace
    char* d = c;
    while (*d != ' ' && *d != '\t' && *d != '\n' && *d != '\0') ++d;  // gobble up initial word
    if (d == c)
      continue;  // no word
    if (*d != ' ' && *d != '\t')
      continue;  // reached end of line
    std::string word(c, d - c);
    if (map->find(word) != map->end())  // don't overwrite old values!
    {
      continue;
    }
    d--;
    *d = '|';  // set up for parser::read_line
    VW::read_line(all, ec, d);
    // now we just need to grab stuff from the default namespace of ec!
    if (ec->feature_space[def].size() == 0)
    {
      continue;
    }
    std::unique_ptr<features> arr(new features);
    arr->deep_copy_from(ec->feature_space[def]);
    map->emplace(word, std::move(arr));

    // clear up ec
    ec->tag.clear();
    ec->indices.clear();
    for (size_t i = 0; i < 256; i++)
    {
      ec->feature_space[i].clear();
    }
  } while ((rc != EOF) && (nread > 0));
  free(buffer);
  VW::dealloc_example(all.p->lp.delete_label, *ec);
  free(ec);

  if (!all.logger.quiet)
    all.oc.trace_message << "dictionary " << s << " contains " << map->size() << " item" << (map->size() == 1 ? "" : "s")
                      << endl;

  all.namespace_dictionaries[(size_t)ns].push_back(map);
  dictionary_info info = {s.to_string(), fd_hash, map};
  all.loaded_dictionaries.push_back(info);
}

void parse_affix_argument(vw& all, std::string str)
{
  if (str.length() == 0)
    return;
  char* cstr = calloc_or_throw<char>(str.length() + 1);
  strcpy(cstr, str.c_str());

  char* p = strtok(cstr, ",");

  try
  {
    while (p != 0)
    {
      char* q = p;
      uint16_t prefix = 1;
      if (q[0] == '+')
      {
        q++;
      }
      else if (q[0] == '-')
      {
        prefix = 0;
        q++;
      }
      if ((q[0] < '1') || (q[0] > '7'))
        THROW("malformed affix argument (length must be 1..7): " << p);

      uint16_t len = (uint16_t)(q[0] - '0');
      uint16_t ns = (uint16_t)' ';  // default namespace
      if (q[1] != 0)
      {
        if (valid_ns(q[1]))
          ns = (uint16_t)q[1];
        else
          THROW("malformed affix argument (invalid namespace): " << p);

        if (q[2] != 0)
          THROW("malformed affix argument (too long): " << p);
      }

      uint16_t afx = (len << 1) | (prefix & 0x1);
      all.gs.affix_features[ns] <<= 4;
      all.gs.affix_features[ns] |= afx;

      p = strtok(nullptr, ",");
    }
  }
  catch (...)
  {
    free(cstr);
    throw;
  }

  free(cstr);
}

void parse_diagnostics(options_i& options, vw& all)
{
  bool version_arg = false;
  bool help = false;
  std::string progress_arg;
  option_group_definition diagnostic_group("Diagnostic options");
  diagnostic_group.add(make_option("version", version_arg).help("Version information"))
      .add(make_option("audit", all.oc.audit).short_name("a").help("print weights of features"))
      .add(make_option("progress", progress_arg)
               .short_name("P")
               .help("Progress update frequency. int: additive, float: multiplicative"))
      .add(make_option("quiet", all.logger.quiet).help("Don't output disgnostics and progress updates"))
      .add(make_option("help", help).short_name("h").help("Look here: http://hunch.net/~vw/ and click on Tutorial."));

  options.add_and_parse(diagnostic_group);

  // pass all.logger.quiet around
  if (all.all_reduce)
    all.all_reduce->quiet = all.logger.quiet;

  // Upon direct query for version -- spit it out to stdout
  if (version_arg)
  {
    std::cout << VW::version.to_string() << " (git commit: " << VW::git_commit << ")\n";
    exit(0);
  }

  if (options.was_supplied("progress") && !all.logger.quiet)
  {
    all.progress_arg = (float)::atof(progress_arg.c_str());
    // --progress interval is dual: either integer or floating-point
    if (progress_arg.find_first_of(".") == std::string::npos)
    {
      // No "." in arg: assume integer -> additive
      all.progress_add = true;
      if (all.progress_arg < 1)
      {
        all.oc.trace_message << "warning: additive --progress <int>"
                          << " can't be < 1: forcing to 1" << endl;
        all.progress_arg = 1;
      }
      all.sd->dump_interval = all.progress_arg;
    }
    else
    {
      // A "." in arg: assume floating-point -> multiplicative
      all.progress_add = false;

      if (all.progress_arg <= 1.0)
      {
        all.oc.trace_message << "warning: multiplicative --progress <float>: " << progress_arg << " is <= 1.0: adding 1.0"
                          << endl;
        all.progress_arg += 1.0;
      }
      else if (all.progress_arg > 9.0)
      {
        all.oc.trace_message << "warning: multiplicative --progress <float>"
                          << " is > 9.0: you probably meant to use an integer" << endl;
      }
      all.sd->dump_interval = 1.0;
    }
  }
}

input_options parse_source(vw& all, options_i& options)
{
  input_options parsed_options;

  option_group_definition input_options("Input options");
  input_options.add(make_option("data", all.ic.data_filename).short_name("d").help("Example set"))
      .add(make_option("daemon", parsed_options.daemon).help("persistent daemon mode on port 26542"))
      .add(make_option("foreground", parsed_options.foreground)
               .help("in persistent daemon mode, do not run in the background"))
      .add(make_option("port", parsed_options.port).help("port to listen on; use 0 to pick unused port"))
      .add(make_option("num_children", all.rc.num_children).help("number of children for persistent daemon mode"))
      .add(make_option("pid_file", parsed_options.pid_file).help("Write pid file in persistent daemon mode"))
      .add(make_option("port_file", parsed_options.port_file).help("Write port used in persistent daemon mode"))
      .add(make_option("cache", parsed_options.cache).short_name("c").help("Use a cache.  The default is <data>.cache"))
      .add(make_option("cache_file", parsed_options.cache_files).help("The location(s) of cache_file."))
      .add(make_option("json", parsed_options.json).help("Enable JSON parsing."))
      .add(make_option("dsjson", parsed_options.dsjson).help("Enable Decision Service JSON parsing."))
      .add(make_option("kill_cache", parsed_options.kill_cache)
               .short_name("k")
               .help("do not reuse existing cache: create a new one always"))
      .add(
          make_option("compressed", parsed_options.compressed)
              .help(
                  "use gzip format whenever possible. If a cache file is being created, this option creates a "
                  "compressed cache file. A mixture of raw-text & compressed inputs are supported with autodetection."))
      .add(make_option("no_stdin", all.stdin_off).help("do not default to reading from stdin"))
      .add(make_option("no_daemon", all.rc.no_daemon).help("Force a loaded daemon or active learning model to accept local input instead of starting in daemon mode"))
      .add(make_option("chain_hash", parsed_options.chain_hash)
               .help("enable chain hash for feature name and string feature value. e.g. {'A': {'B': 'C'}} is hashed as A^B^C"));


  options.add_and_parse(input_options);

  // Check if the options provider has any positional args. Only really makes sense for command line, others just return
  // an empty list.
  const auto positional_tokens = options.get_positional_tokens();
  if (positional_tokens.size() == 1)
  {
    all.ic.data_filename = positional_tokens[0];
  }
  else if (positional_tokens.size() > 1)
  {
    all.oc.trace_message << "Warning: Multiple data files passed as positional parameters, only the first one will be read and the rest will be ignored." << endl;
  }

  if (parsed_options.daemon || options.was_supplied("pid_file") || (options.was_supplied("port") && !all.active))
  {
    all.rc.daemon = true;
    // allow each child to process up to 1e5 connections
    all.ec.numpasses = (size_t)1e5;
  }

  // Add an implicit cache file based on the data filename.
  if (parsed_options.cache)
  {
    parsed_options.cache_files.push_back(all.ic.data_filename + ".cache");
  }

  if ((parsed_options.cache || options.was_supplied("cache_file")) && options.was_supplied("invert_hash"))
    THROW("invert_hash is incompatible with a cache file.  Use it in single pass mode only.");

  if (!all.ec.holdout_set_off &&
      (options.was_supplied("output_feature_regularizer_binary") ||
          options.was_supplied("output_feature_regularizer_text")))
  {
    all.ec.holdout_set_off = true;
    all.oc.trace_message << "Making holdout_set_off=true since output regularizer specified" << endl;
  }

  return parsed_options;
}

bool interactions_settings_doubled = false;  // local setting setted in parse_modules()
namespace VW
{
const char* are_features_compatible(vw& vw1, vw& vw2)
{
  if (vw1.p->hasher != vw2.p->hasher)
    return "hasher";


  if (!std::equal(vw1.gs.spelling_features.begin(), vw1.gs.spelling_features.end(), vw2.gs.spelling_features.begin()))
    return "spelling_features";

  if (!std::equal(vw1.gs.affix_features.begin(), vw1.gs.affix_features.end(), vw2.gs.affix_features.begin()))
    return "affix_features";

  if (!std::equal(vw1.gs.ngram.begin(), vw1.gs.ngram.end(), vw2.gs.ngram.begin()))
    return "ngram";

  if (!std::equal(vw1.gs.skips.begin(), vw1.gs.skips.end(), vw2.gs.skips.begin()))
    return "skips";

  if (!std::equal(vw1.gs.limit.begin(), vw1.gs.limit.end(), vw2.gs.limit.begin()))
    return "limit";

  if (vw1.fc.num_bits != vw2.fc.num_bits)
    return "num_bits";

  if (vw1.fc.permutations != vw2.fc.permutations)
    return "permutations";

  if (vw1.gs.interactions.size() != vw2.gs.interactions.size())
    return "interactions size";

  if (vw1.gs.ignore_some != vw2.gs.ignore_some)
    return "ignore_some";

  if (vw1.gs.ignore_some && !std::equal(vw1.fc.ignore.begin(), vw1.fc.ignore.end(), vw2.fc.ignore.begin()))
    return "ignore";

  if (vw1.gs.ignore_some_linear != vw2.gs.ignore_some_linear)
    return "ignore_some_linear";

  if (vw1.gs.ignore_some_linear &&
      !std::equal(vw1.fc.ignore_linear.begin(), vw1.fc.ignore_linear.end(), vw2.fc.ignore_linear.begin()))
    return "ignore_linear";

  if (vw1.gs.redefine_some != vw2.gs.redefine_some)
    return "redefine_some";

  if (vw1.gs.redefine_some && !std::equal(vw1.fc.redefine.begin(), vw1.fc.redefine.end(), vw2.fc.redefine.begin()))
    return "redefine";

  if (vw1.gs.add_constant != vw2.gs.add_constant)
    return "add_constant";

  if (vw1.dictionary_path.size() != vw2.dictionary_path.size())
    return "dictionary_path size";

  if (!std::equal(vw1.dictionary_path.begin(), vw1.dictionary_path.end(), vw2.dictionary_path.begin()))
    return "dictionary_path";

  for (auto i = std::begin(vw1.gs.interactions), j = std::begin(vw2.gs.interactions); i != std::end(vw1.gs.interactions);
       ++i, ++j)
    if (*i != *j)
      return "interaction mismatch";

  return nullptr;
}

}  // namespace VW
// return a copy of std::string replacing \x00 sequences in it
std::string spoof_hex_encoded_namespaces(const std::string& arg)
{
  std::string res;
  int pos = 0;
  while (pos < (int)arg.size() - 3)
  {
    if (arg[pos] == '\\' && arg[pos + 1] == 'x')
    {
      std::string substr = arg.substr(pos + 2, 2);
      char* p;
      unsigned char c = (unsigned char)strtoul(substr.c_str(), &p, 16);
      if (*p == '\0')
      {
        res.push_back(c);
        pos += 4;
      }
      else
      {
        std::cerr << "Possibly malformed hex representation of a namespace: '\\x" << substr << "'\n";
        res.push_back(arg[pos++]);
      }
    }
    else
      res.push_back(arg[pos++]);
  }

  while (pos < (int)arg.size())  // copy last 2 characters
    res.push_back(arg[pos++]);

  return res;
}

void parse_feature_tweaks(options_i& options, vw& all, std::vector<std::string>& dictionary_nses)
{
  std::string hash_function("strings");
  uint32_t new_bits;
  std::vector<std::string> spelling_ns;
  std::vector<std::string> quadratics;
  std::vector<std::string> cubics;
  std::vector<std::string> interactions;
  std::vector<std::string> ignores;
  std::vector<std::string> ignore_linears;
  std::vector<std::string> keeps;
  std::vector<std::string> redefines;

  std::vector<std::string> dictionary_path;

  bool noconstant;
  bool leave_duplicate_interactions;
  std::string affix;
  std::string q_colon;

  option_group_definition feature_options("Feature options");
  feature_options
      .add(make_option("hash", hash_function).keep().help("how to hash the features. Available options: strings, all"))
      .add(make_option("hash_seed", all.fc.hash_seed).keep().default_value(0).help("seed for hash function"))
      .add(make_option("ignore", ignores).keep().help("ignore namespaces beginning with character <arg>"))
      .add(make_option("ignore_linear", ignore_linears)
               .keep()
               .help("ignore namespaces beginning with character <arg> for linear terms only"))
      .add(make_option("keep", keeps).keep().help("keep namespaces beginning with character <arg>"))
      .add(make_option("redefine", redefines)
               .keep()
               .help("redefine namespaces beginning with characters of std::string S as namespace N. <arg> shall be in "
                     "form "
                     "'N:=S' where := is operator. Empty N or S are treated as default namespace. Use ':' as a "
                     "wildcard in S.")
               .keep())
      .add(make_option("bit_precision", new_bits).short_name("b").help("number of bits in the feature table"))
      .add(make_option("noconstant", noconstant).help("Don't add a constant feature"))
      .add(make_option("constant", all.wc.initial_constant).short_name("C").help("Set initial value of constant"))
      .add(make_option("ngram", all.fc.ngram_strings)
               .help("Generate N grams. To generate N grams for a single namespace 'foo', arg should be fN."))
      .add(make_option("skips", all.fc.skip_strings)
               .help("Generate skips in N grams. This in conjunction with the ngram tag can be used to generate "
                     "generalized n-skip-k-gram. To generate n-skips for a single namespace 'foo', arg should be fN."))
      .add(make_option("feature_limit", all.fc.limit_strings)
               .help("limit to N features. To apply to a single namespace 'foo', arg should be fN"))
      .add(make_option("affix", affix)
               .keep()
               .help("generate prefixes/suffixes of features; argument '+2a,-3b,+1' means generate 2-char prefixes for "
                     "namespace a, 3-char suffixes for b and 1 char prefixes for default namespace"))
      .add(make_option("spelling", spelling_ns)
               .keep()
               .help("compute spelling features for a give namespace (use '_' for default namespace)"))
      .add(make_option("dictionary", dictionary_nses)
               .keep()
               .help("read a dictionary for additional features (arg either 'x:file' or just 'file')"))
      .add(make_option("dictionary_path", dictionary_path)
               .help("look in this directory for dictionaries; defaults to current directory or env{PATH}"))
      .add(make_option("interactions", interactions)
               .keep()
               .help("Create feature interactions of any level between namespaces."))
      .add(make_option("permutations", all.fc.permutations)
               .help("Use permutations instead of combinations for feature interactions of same namespace."))
      .add(make_option("leave_duplicate_interactions", leave_duplicate_interactions)
               .help("Don't remove interactions with duplicate combinations of namespaces. For ex. this is a "
                     "duplicate: '-q ab -q ba' and a lot more in '-q ::'."))
      .add(make_option("quadratic", quadratics).short_name("q").keep().help("Create and use quadratic features"))
      // TODO this option is unused - remove?
      .add(make_option("q:", q_colon).help("DEPRECATED ':' corresponds to a wildcard for all printable characters"))
      .add(make_option("cubic", cubics).keep().help("Create and use cubic features"));
  options.add_and_parse(feature_options);

  // feature manipulation
  all.p->hasher = getHasher(hash_function);

  if (options.was_supplied("spelling"))
  {
    for (size_t id = 0; id < spelling_ns.size(); id++)
    {
      spelling_ns[id] = spoof_hex_encoded_namespaces(spelling_ns[id]);
      if (spelling_ns[id][0] == '_')
        all.gs.spelling_features[(unsigned char)' '] = true;
      else
        all.gs.spelling_features[(size_t)spelling_ns[id][0]] = true;
    }
  }

  if (options.was_supplied("q:"))
  {
    all.oc.trace_message << "WARNING: '--q:' is deprecated and not supported. You can use : as a wildcard in interactions."
                      << endl;
  }

  if (options.was_supplied("affix"))
    parse_affix_argument(all, spoof_hex_encoded_namespaces(affix));

  if (options.was_supplied("ngram"))
  {
    if (options.was_supplied("sort_features"))
      THROW("ngram is incompatible with sort_features.");

    for (size_t i = 0; i < all.fc.ngram_strings.size(); i++)
      all.fc.ngram_strings[i] = spoof_hex_encoded_namespaces(all.fc.ngram_strings[i]);
    compile_gram(all.fc.ngram_strings, all.gs.ngram, (char*)"grams", all.logger.quiet);
  }

  if (options.was_supplied("skips"))
  {
    if (!options.was_supplied("ngram"))
      THROW("You can not skip unless ngram is > 1");

    for (size_t i = 0; i < all.fc.skip_strings.size(); i++)
      all.fc.skip_strings[i] = spoof_hex_encoded_namespaces(all.fc.skip_strings[i]);
    compile_gram(all.fc.skip_strings, all.gs.skips, (char*)"skips", all.logger.quiet);
  }

  if (options.was_supplied("feature_limit"))
    compile_limits(all.fc.limit_strings, all.gs.limit, all.logger.quiet);

  if (options.was_supplied("bit_precision"))
  {
    if (all.fc.default_bits == false && new_bits != all.fc.num_bits)
      THROW("Number of bits is set to " << new_bits << " and " << all.fc.num_bits
                                        << " by argument and model.  That does not work.");

    all.fc.default_bits = false;
    all.fc.num_bits = new_bits;

    VW::validate_num_bits(all);
  }

  // prepare namespace interactions
  std::vector<std::vector<namespace_index>> expanded_interactions;

  if ( ( (!all.gs.interactions.empty() && /*data was restored from old model file directly to v_array and will be overriden automatically*/
          (options.was_supplied("quadratic") || options.was_supplied("cubic") || options.was_supplied("interactions")) ) )
       ||
       interactions_settings_doubled /*settings were restored from model file to file_options and overriden by params from command line*/)
  {
    all.oc.trace_message << "WARNING: model file has set of {-q, --cubic, --interactions} settings stored, but they'll be "
                         "OVERRIDEN by set of {-q, --cubic, --interactions} settings from command line."
                      << endl;

    // in case arrays were already filled in with values from old model file - reset them
    if (!all.gs.interactions.empty())
      all.gs.interactions.clear();
  }

  if (options.was_supplied("quadratic"))
  {
    if (!all.logger.quiet)
      all.oc.trace_message << "creating quadratic features for pairs: ";

    for (auto& i : quadratics)
    {
      i = spoof_hex_encoded_namespaces(i);
      if (!all.logger.quiet)
        all.oc.trace_message << i << " ";
    }

    std::vector<std::vector<namespace_index>> new_quadratics;
    for (const auto& i : quadratics){
      new_quadratics.emplace_back(i.begin(), i.end());
    }

    expanded_interactions =
        INTERACTIONS::expand_interactions(new_quadratics, 2, "error, quadratic features must involve two sets.");

    if (!all.logger.quiet)
      all.oc.trace_message << endl;
  }

  if (options.was_supplied("cubic"))
  {
    if (!all.logger.quiet)
      all.oc.trace_message << "creating cubic features for triples: ";
    for (auto i = cubics.begin(); i != cubics.end(); ++i)
    {
      *i = spoof_hex_encoded_namespaces(*i);
      if (!all.logger.quiet)
        all.oc.trace_message << *i << " ";
    }

    std::vector<std::vector<namespace_index>> new_cubics;
    for (const auto& i : cubics){
      new_cubics.emplace_back(i.begin(), i.end());
    }

    std::vector<std::vector<namespace_index>> exp_cubic =
        INTERACTIONS::expand_interactions(new_cubics, 3, "error, cubic features must involve three sets.");
    expanded_interactions.insert(std::begin(expanded_interactions), std::begin(exp_cubic), std::end(exp_cubic));

    if (!all.logger.quiet)
      all.oc.trace_message << endl;
  }

  if (options.was_supplied("interactions"))
  {
    if (!all.logger.quiet)
      all.oc.trace_message << "creating features for following interactions: ";

    for (auto i = interactions.begin(); i != interactions.end(); ++i)
    {
      *i = spoof_hex_encoded_namespaces(*i);
      if (!all.logger.quiet)
        all.oc.trace_message << *i << " ";
    }

    std::vector<std::vector<namespace_index>> new_interactions;
    for (const auto& i : interactions){
      new_interactions.emplace_back(i.begin(), i.end());
    }

    std::vector<std::vector<namespace_index>> exp_inter = INTERACTIONS::expand_interactions(new_interactions, 0, "");
    expanded_interactions.insert(std::begin(expanded_interactions), std::begin(exp_inter), std::end(exp_inter));

    if (!all.logger.quiet)
      all.oc.trace_message << endl;
  }

  if (expanded_interactions.size() > 0)
  {
    size_t removed_cnt;
    size_t sorted_cnt;
    INTERACTIONS::sort_and_filter_duplicate_interactions(
        expanded_interactions, !leave_duplicate_interactions, removed_cnt, sorted_cnt);

    if (removed_cnt > 0)
      all.oc.trace_message << "WARNING: duplicate namespace interactions were found. Removed: " << removed_cnt << '.'
                        << endl
                        << "You can use --leave_duplicate_interactions to disable this behaviour." << endl;
    if (sorted_cnt > 0)
      all.oc.trace_message << "WARNING: some interactions contain duplicate characters and their characters order has "
                           "been changed. Interactions affected: "
                        << sorted_cnt << '.' << endl;

    if (all.gs.interactions.size() > 0)
    {
      // should be empty, but just in case...
      all.gs.interactions.clear();
    }

    all.gs.interactions = expanded_interactions;
  }

  for (size_t i = 0; i < 256; i++)
  {
    all.fc.ignore[i] = false;
    all.fc.ignore_linear[i] = false;
  }
  all.gs.ignore_some = false;
  all.gs.ignore_some_linear = false;

  if (options.was_supplied("ignore"))
  {
    all.gs.ignore_some = true;

    for (auto & i : ignores)
    {
      i = spoof_hex_encoded_namespaces(i);
      for (auto j : i) all.fc.ignore[(size_t)(unsigned char)j] = true;
    }

    if (!all.logger.quiet)
    {
      all.oc.trace_message << "ignoring namespaces beginning with: ";
      for (auto const& ignore : ignores)
        for (auto const character : ignore) all.oc.trace_message << character << " ";

      all.oc.trace_message << endl;
    }
  }

  if (options.was_supplied("ignore_linear"))
  {
    all.gs.ignore_some_linear = true;

    for (auto & i : ignore_linears)
    {
      i = spoof_hex_encoded_namespaces(i);
      for (auto j : i)
        all.fc.ignore_linear[(size_t)(unsigned char)j] = true;
    }

    if (!all.logger.quiet)
    {
      all.oc.trace_message << "ignoring linear terms for namespaces beginning with: ";
      for (auto const& ignore : ignore_linears)
        for (auto const character : ignore) all.oc.trace_message << character << " ";

      all.oc.trace_message << endl;
    }
  }

  if (options.was_supplied("keep"))
  {
    for (size_t i = 0; i < 256; i++) all.fc.ignore[i] = true;

    all.gs.ignore_some = true;

    for (auto & i : keeps)
    {
      i = spoof_hex_encoded_namespaces(i);
      for (const auto& j : i) all.fc.ignore[(size_t)(unsigned char)j] = false;
    }

    if (!all.logger.quiet)
    {
      all.oc.trace_message << "using namespaces beginning with: ";
      for (auto const& keep : keeps)
        for (auto const character : keep) all.oc.trace_message << character << " ";

      all.oc.trace_message << endl;
    }
  }

  // --redefine param code
  all.gs.redefine_some = false;  // false by default

  if (options.was_supplied("redefine"))
  {
    // initail values: i-th namespace is redefined to i itself
    for (size_t i = 0; i < 256; i++) all.fc.redefine[i] = (unsigned char)i;

    // note: --redefine declaration order is matter
    // so --redefine :=L --redefine ab:=M  --ignore L  will ignore all except a and b under new M namspace

    for (const auto & arg : redefines)
    {
      const std::string & argument = spoof_hex_encoded_namespaces(arg);
      size_t arg_len = argument.length();

      size_t operator_pos = 0;  // keeps operator pos + 1 to stay unsigned type
      bool operator_found = false;
      unsigned char new_namespace = ' ';

      // let's find operator ':=' position in N:=S
      for (size_t i = 0; i < arg_len; i++)
      {
        if (operator_found)
        {
          if (i > 2)
          {
            new_namespace = argument[0];
          }  // N is not empty
          break;
        }
        else if (argument[i] == ':')
          operator_pos = i + 1;
        else if ((argument[i] == '=') && (operator_pos == i))
          operator_found = true;
      }

      if (!operator_found)
        THROW("argument of --redefine is malformed. Valid format is N:=S, :=S or N:=");

      if (++operator_pos > 3)  // seek operator end
        all.oc.trace_message
            << "WARNING: multiple namespaces are used in target part of --redefine argument. Only first one ('"
            << new_namespace << "') will be used as target namespace." << endl;

      all.gs.redefine_some = true;

      // case ':=S' doesn't require any additional code as new_namespace = ' ' by default

      if (operator_pos == arg_len)  // S is empty, default namespace shall be used
        all.fc.redefine[(int)' '] = new_namespace;
      else
        for (size_t i = operator_pos; i < arg_len; i++)
        {
          // all namespaces from S are redefined to N
          unsigned char c = argument[i];
          if (c != ':')
            all.fc.redefine[c] = new_namespace;
          else
          {
            // wildcard found: redefine all except default and break
            for (size_t i = 0; i < 256; i++) all.fc.redefine[i] = new_namespace;
            break;  // break processing S
          }
        }
    }
  }

  if (options.was_supplied("dictionary"))
  {
    if (options.was_supplied("dictionary_path"))
      for (const std::string & path : dictionary_path)
        if (directory_exists(path))
          all.dictionary_path.push_back(path);
    if (directory_exists("."))
      all.dictionary_path.push_back(".");

    const std::string PATH = getenv("PATH");
#if _WIN32
    const char delimiter = ';';
#else
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

  if (noconstant)
    all.gs.add_constant = false;
}

void parse_example_tweaks(options_i& options, vw& all)
{
  std::string named_labels;
  std::string loss_function;
  float loss_parameter = 0.0;
  size_t early_terminate_passes;
  bool test_only = false;

  option_group_definition example_options("Example options");
  example_options.add(make_option("testonly", test_only).short_name("t").help("Ignore label information and just test"))
      .add(make_option("holdout_off", all.ec.holdout_set_off).help("no holdout data in multiple passes"))
      .add(make_option("holdout_period", all.ec.holdout_period).default_value(10).help("holdout period for test only"))
      .add(make_option("holdout_after", all.ec.holdout_after)
               .help("holdout after n training examples, default off (disables holdout_period)"))
      .add(
          make_option("early_terminate", early_terminate_passes)
              .default_value(3)
              .help(
                  "Specify the number of passes tolerated when holdout loss doesn't decrease before early termination"))
      .add(make_option("passes", all.ec.numpasses).help("Number of Training Passes"))
      .add(make_option("initial_pass_length", all.ec.pass_length).help("initial number of examples per pass"))
      .add(make_option("examples", all.max_examples).help("number of examples to parse"))
      .add(make_option("min_prediction", all.sd->min_label).help("Smallest prediction to output"))
      .add(make_option("max_prediction", all.sd->max_label).help("Largest prediction to output"))
      .add(make_option("sort_features", all.p->sort_features)
               .help("turn this on to disregard order in which features have been defined. This will lead to smaller "
                     "cache sizes"))
      .add(make_option("loss_function", loss_function)
               .default_value("squared")
               .help("Specify the loss function to be used, uses squared by default. Currently available ones are "
                     "squared, classic, hinge, logistic, quantile and poisson."))
      .add(make_option("quantile_tau", loss_parameter)
               .default_value(0.5f)
               .help("Parameter \\tau associated with Quantile loss. Defaults to 0.5"))
      .add(make_option("l1", all.uc.l1_lambda).help("l_1 lambda"))
      .add(make_option("l2", all.uc.l2_lambda).help("l_2 lambda"))
      .add(make_option("no_bias_regularization", all.uc.no_bias).help("no bias in regularization"))
      .add(make_option("named_labels", named_labels)
               .keep()
               .help("use names for labels (multiclass, etc.) rather than integers, argument specified all possible "
                     "labels, comma-sep, eg \"--named_labels Noun,Verb,Adj,Punc\""));
  options.add_and_parse(example_options);

  if (test_only || all.gs.eta == 0.)
  {
    if (!all.logger.quiet)
      all.oc.trace_message << "only testing" << endl;
    all.gs.training = false;
    if (all.lda > 0)
      all.gs.eta = 0;
  }
  else
    all.gs.training = true;

  if ((all.ec.numpasses > 1 || all.ec.holdout_after > 0) && !all.ec.holdout_set_off)
    all.ec.holdout_set_off = false;  // holdout is on unless explicitly off
  else
    all.ec.holdout_set_off = true;

  if (options.was_supplied("min_prediction") || options.was_supplied("max_prediction") || test_only)
    all.set_minmax = noop_mm;

  if (options.was_supplied("named_labels"))
  {
    all.sd->ldict = &calloc_or_throw<namedlabels>();
    new (all.sd->ldict) namedlabels(named_labels);
    if (!all.logger.quiet)
      all.oc.trace_message << "parsed " << all.sd->ldict->getK() << " named labels" << endl;
  }

  all.loss = getLossFunction(all, loss_function, loss_parameter);

  if (all.uc.l1_lambda < 0.)
  {
    all.oc.trace_message << "l1_lambda should be nonnegative: resetting from " << all.uc.l1_lambda << " to 0" << endl;
    all.uc.l1_lambda = 0.;
  }
  if (all.uc.l2_lambda < 0.)
  {
    all.oc.trace_message << "l2_lambda should be nonnegative: resetting from " << all.uc.l2_lambda << " to 0" << endl;
    all.uc.l2_lambda = 0.;
  }
  all.reg_mode += (all.uc.l1_lambda > 0.) ? 1 : 0;
  all.reg_mode += (all.uc.l2_lambda > 0.) ? 2 : 0;
  if (!all.logger.quiet)
  {
    if (all.reg_mode % 2 && !options.was_supplied("bfgs"))
      all.oc.trace_message << "using l1 regularization = " << all.uc.l1_lambda << endl;
    if (all.reg_mode > 1)
      all.oc.trace_message << "using l2 regularization = " << all.uc.l2_lambda << endl;
  }
}

void parse_output_preds(options_i& options, vw& all)
{
  std::string predictions;
  std::string raw_predictions;

  option_group_definition output_options("Output options");
  output_options.add(make_option("predictions", predictions).short_name("p").help("File to output predictions to"))
      .add(make_option("raw_predictions", raw_predictions)
               .short_name("r")
               .help("File to output unnormalized predictions to"));
  options.add_and_parse(output_options);

  if (options.was_supplied("predictions"))
  {
    if (!all.logger.quiet)
      all.oc.trace_message << "predictions = " << predictions << endl;

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
        all.oc.trace_message << "Error opening the predictions file: " << predictions << endl;
      }
    }
  }

  if (options.was_supplied("raw_predictions"))
  {
    if (!all.logger.quiet)
    {
      all.oc.trace_message << "raw predictions = " << raw_predictions << endl;
      if (options.was_supplied("binary"))
        all.oc.trace_message << "Warning: --raw_predictions has no defined value when --binary specified, expect no output"
                          << endl;
    }
    if (raw_predictions == "stdout")
    {
      all.raw_prediction = VW::io::open_stdout();
    }
    else
    {
      all.raw_prediction = VW::io::open_file_writer(raw_predictions);
    }
  }
}

void parse_output_model(options_i& options, vw& all)
{
  option_group_definition output_model_options("Output model");
  output_model_options
      .add(make_option("final_regressor", all.final_regressor_name).short_name("f").help("Final regressor"))
      .add(make_option("readable_model", all.text_regressor_name)
               .help("Output human-readable final regressor with numeric features"))
      .add(make_option("invert_hash", all.inv_hash_regressor_name)
               .help("Output human-readable final regressor with feature names.  Computationally expensive."))
      .add(make_option("save_resume", all.oc.save_resume)
               .help("save extra state so learning can be resumed later with new data"))
      .add(make_option("preserve_performance_counters", all.oc.preserve_performance_counters)
               .help("reset performance counters when warmstarting"))
      .add(make_option("save_per_pass", all.oc.save_per_pass).help("Save the model after every pass over data"))
      .add(make_option("output_feature_regularizer_binary", all.oc.per_feature_regularizer_output)
               .help("Per feature regularization output file"))
      .add(make_option("output_feature_regularizer_text", all.oc.per_feature_regularizer_text)
               .help("Per feature regularization output file, in text"))
      .add(make_option("id", all.oc.id).help("User supplied ID embedded into the final regressor"));
  options.add_and_parse(output_model_options);

  if (all.final_regressor_name.compare("") && !all.logger.quiet)
    all.oc.trace_message << "final_regressor = " << all.final_regressor_name << endl;

  if (options.was_supplied("invert_hash"))
    all.hash_inv = true;

  // Question: This doesn't seem necessary
  // if (options.was_supplied("id") && find(arg.args.begin(), arg.args.end(), "--id") == arg.args.end())
  // {
  //   arg.args.push_back("--id");
  //   arg.args.push_back(arg.vm["id"].as<std::string>());
  // }
}

void load_input_model(vw& all, io_buf& io_temp)
{
  // Need to see if we have to load feature mask first or second.
  // -i and -mask are from same file, load -i file first so mask can use it
  if (!all.ic.feature_mask.empty() && all.ic.initial_regressors.size() > 0 && all.ic.feature_mask == all.ic.initial_regressors[0])
  {
    // load rest of regressor
    all.l->save_load(io_temp, true, false);
    io_temp.close_file();

    parse_mask_regressor_args(all, all.ic.feature_mask, all.ic.initial_regressors);
  }
  else
  {  // load mask first
    parse_mask_regressor_args(all, all.ic.feature_mask, all.ic.initial_regressors);

    // load rest of regressor
    all.l->save_load(io_temp, true, false);
    io_temp.close_file();
  }
}

VW::LEARNER::base_learner* setup_base(options_i& options, vw& all)
{
  auto setup_func = all.reduction_stack.top();
  all.reduction_stack.pop();
  auto base = setup_func(options, all);

  if (base == nullptr)
    return setup_base(options, all);
  else
    return base;
}

void parse_reductions(options_i& options, vw& all)
{
  // Base algorithms
  all.reduction_stack.push(GD::setup);
  all.reduction_stack.push(kernel_svm_setup);
  all.reduction_stack.push(ftrl_setup);
  all.reduction_stack.push(svrg_setup);
  all.reduction_stack.push(sender_setup);
  all.reduction_stack.push(gd_mf_setup);
  all.reduction_stack.push(print_setup);
  all.reduction_stack.push(noop_setup);
  all.reduction_stack.push(lda_setup);
  all.reduction_stack.push(bfgs_setup);
  all.reduction_stack.push(OjaNewton_setup);
  // all.reduction_stack.push(VW_CNTK::setup);

  // Score Users
  all.reduction_stack.push(baseline_setup);
  all.reduction_stack.push(ExpReplay::expreplay_setup<'b', simple_label>);
  all.reduction_stack.push(active_setup);
  all.reduction_stack.push(active_cover_setup);
  all.reduction_stack.push(confidence_setup);
  all.reduction_stack.push(nn_setup);
  all.reduction_stack.push(mf_setup);
  all.reduction_stack.push(marginal_setup);
  all.reduction_stack.push(autolink_setup);
  all.reduction_stack.push(lrq_setup);
  all.reduction_stack.push(lrqfa_setup);
  all.reduction_stack.push(stagewise_poly_setup);
  all.reduction_stack.push(scorer_setup);
  // Reductions
  all.reduction_stack.push(bs_setup);
  all.reduction_stack.push(binary_setup);

  all.reduction_stack.push(ExpReplay::expreplay_setup<'m', MULTICLASS::mc_label>);
  all.reduction_stack.push(topk_setup);
  all.reduction_stack.push(oaa_setup);
  all.reduction_stack.push(boosting_setup);
  all.reduction_stack.push(ect_setup);
  all.reduction_stack.push(log_multi_setup);
  all.reduction_stack.push(recall_tree_setup);
  all.reduction_stack.push(memory_tree_setup);
  all.reduction_stack.push(classweight_setup);
  all.reduction_stack.push(multilabel_oaa_setup);

  all.reduction_stack.push(cs_active_setup);
  all.reduction_stack.push(CSOAA::csoaa_setup);
  all.reduction_stack.push(interact_setup);
  all.reduction_stack.push(CSOAA::csldf_setup);
  all.reduction_stack.push(cb_algs_setup);
  all.reduction_stack.push(cb_adf_setup);
  all.reduction_stack.push(mwt_setup);
  all.reduction_stack.push(cb_explore_setup);
  all.reduction_stack.push(VW::cb_explore_adf::greedy::setup);
  all.reduction_stack.push(VW::cb_explore_adf::softmax::setup);
  all.reduction_stack.push(VW::cb_explore_adf::rnd::setup);
  all.reduction_stack.push(VW::cb_explore_adf::regcb::setup);
  all.reduction_stack.push(VW::cb_explore_adf::first::setup);
  all.reduction_stack.push(VW::cb_explore_adf::cover::setup);
  all.reduction_stack.push(VW::cb_explore_adf::bag::setup);
  all.reduction_stack.push(cb_dro_setup);
  all.reduction_stack.push(cb_sample_setup);
  all.reduction_stack.push(VW::shared_feature_merger::shared_feature_merger_setup);
  all.reduction_stack.push(CCB::ccb_explore_adf_setup);
  all.reduction_stack.push(VW::slates::slates_setup);
  // cbify/warm_cb can generate multi-examples. Merge shared features after them
  all.reduction_stack.push(warm_cb_setup);
  all.reduction_stack.push(cbify_setup);
  all.reduction_stack.push(cbifyldf_setup);
  all.reduction_stack.push(explore_eval_setup);
  all.reduction_stack.push(ExpReplay::expreplay_setup<'c', COST_SENSITIVE::cs_label>);
  all.reduction_stack.push(Search::setup);
  all.reduction_stack.push(audit_regressor_setup);

  all.l = setup_base(options, all);
}

vw& parse_args(options_i& options, trace_message_t trace_listener, void* trace_context)
{
  vw& all = *(new vw());
  all.options = &options;

  if (trace_listener)
  {
    all.oc.trace_message.trace_listener = trace_listener;
    all.oc.trace_message.trace_context = trace_context;
  }

  try
  {
    time(&all.gs.init_time);

    bool strict_parse = false;
    int ring_size_tmp;
    option_group_definition vw_args("VW options");
    vw_args.add(make_option("ring_size", ring_size_tmp).default_value(256).help("size of example ring"))
        .add(make_option("strict_parse", strict_parse).help("throw on malformed examples"));
    options.add_and_parse(vw_args);

    if (ring_size_tmp <= 0)
    {
      THROW("ring_size should be positive");
    }
    size_t ring_size = static_cast<size_t>(ring_size_tmp);

    all.p = new parser{ring_size, strict_parse};
    all.p->_shared_data = all.sd;

    option_group_definition update_args("Update options");
    update_args.add(make_option("learning_rate", all.gs.eta).help("Set learning rate").short_name("l"))
        .add(make_option("power_t", all.uc.power_t).help("t power value"))
        .add(make_option("decay_learning_rate", all.uc.eta_decay_rate)
                 .help("Set Decay factor for learning_rate between passes"))
        .add(make_option("initial_t", all.sd->t).help("initial t value"))
        .add(make_option("feature_mask", all.ic.feature_mask)
                 .help("Use existing regressor to determine which parameters may be updated.  If no initial_regressor "
                       "given, also used for initial weights."));
    options.add_and_parse(update_args);

    option_group_definition weight_args("Weight options");
    weight_args
        .add(make_option("initial_regressor", all.ic.initial_regressors).help("Initial regressor(s)").short_name("i"))
        .add(make_option("initial_weight", all.wc.initial_weight).help("Set all weights to an initial value of arg."))
        .add(make_option("random_weights", all.wc.random_weights).help("make initial weights random"))
        .add(make_option("normal_weights", all.wc.normal_weights).help("make initial weights normal"))
        .add(make_option("truncated_normal_weights", all.wc.tnormal_weights).help("make initial weights truncated normal"))
        .add(make_option("sparse_weights", all.weights.sparse).help("Use a sparse datastructure for weights"))
        .add(make_option("input_feature_regularizer", all.ic.per_feature_regularizer_input)
                 .help("Per feature regularization input file"));
    options.add_and_parse(weight_args);

    std::string span_server_arg;
    int span_server_port_arg;
    // bool threads_arg;
    size_t unique_id_arg;
    size_t total_arg;
    size_t node_arg;
    option_group_definition parallelization_args("Parallelization options");
    parallelization_args
        .add(make_option("span_server", span_server_arg).help("Location of server for setting up spanning tree"))
        //(make_option("threads", threads_arg).help("Enable multi-threading")) Unused option?
        .add(make_option("unique_id", unique_id_arg).default_value(0).help("unique id used for cluster parallel jobs"))
        .add(
            make_option("total", total_arg).default_value(1).help("total number of nodes used in cluster parallel job"))
        .add(make_option("node", node_arg).default_value(0).help("node number in cluster parallel job"))
        .add(make_option("span_server_port", span_server_port_arg)
                 .default_value(26543)
                 .help("Port of the server for setting up spanning tree"));
    options.add_and_parse(parallelization_args);

    // total, unique_id and node must be specified together.
    if ((options.was_supplied("total") || options.was_supplied("node") || options.was_supplied("unique_id")) &&
        !(options.was_supplied("total") && options.was_supplied("node") && options.was_supplied("unique_id")))
    {
      THROW("you must specificy unique_id, total, and node if you specify any");
    }

    if (options.was_supplied("span_server"))
    {
      all.all_reduce_type = AllReduceType::Socket;
      all.all_reduce = new AllReduceSockets(
          span_server_arg, span_server_port_arg, unique_id_arg, total_arg, node_arg, all.logger.quiet);
    }

    parse_diagnostics(options, all);

    all.gs.initial_t = (float)all.sd->t;
    return all;
  }
  catch (...)
  {
    VW::finish(all);
    throw;
  }
}

bool check_interaction_settings_collision(options_i& options, std::string file_options)
{
  bool command_line_has_interaction = options.was_supplied("q") || options.was_supplied("quadratic") ||
      options.was_supplied("cubic") || options.was_supplied("interactions");

  if (!command_line_has_interaction)
    return false;

  // we don't use -q to save pairs in all.file_options, so only 3 options checked
  bool file_options_has_interaction = file_options.find("--quadratic") != std::string::npos;
  file_options_has_interaction = file_options_has_interaction || (file_options.find("--cubic") != std::string::npos);
  file_options_has_interaction =
      file_options_has_interaction || (file_options.find("--interactions") != std::string::npos);

  return file_options_has_interaction;
}

options_i& load_header_merge_options(options_i& options, vw& all, io_buf& model)
{
  std::string file_options;
  save_load_header(all, model, true, false, file_options, options);

  interactions_settings_doubled = check_interaction_settings_collision(options, file_options);

  // Convert file_options into  vector.
  std::istringstream ss{file_options};
  std::vector<std::string> container{std::istream_iterator<std::string>{ss}, std::istream_iterator<std::string>{}};

  po::options_description desc("");

  // Get list of options in file options std::string
  po::parsed_options pos = po::command_line_parser(container).options(desc).allow_unregistered().run();

  bool skipping = false;
  std::string saved_key = "";
  unsigned int count = 0;
  bool first_seen = false;
  for (auto opt : pos.options)
  {
    // If we previously encountered an option we want to skip, ignore tokens without --.
    if (skipping)
    {
      for (auto token : opt.original_tokens)
      {
        auto found = token.find("--");
        if (found != std::string::npos)
        {
          skipping = false;
        }
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
    {
      treat_as_value = true;
    }

    // If the interaction settings are doubled, the copy in the model file is ignored.
    if (interactions_settings_doubled &&
        (opt.string_key == "quadratic" || opt.string_key == "cubic" || opt.string_key == "interactions"))
    {
      // skip this option.
      skipping = true;
      continue;
    }

    // File options should always use long form.

    // If the key is empty this must be a value, otherwise set the key.
    if (!treat_as_value && opt.string_key != "")
    {
      // If the new token is a new option and there were no values previously it was a bool option. Add it as a switch.
      if (count == 0 && first_seen)
      {
        options.insert(saved_key, "");
      }

      saved_key = opt.string_key;
      count = 0;
      first_seen = true;

      if (opt.value.size() > 0)
      {
        for (auto value : opt.value)
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
      for (auto value : source)
      {
        options.insert(saved_key, value);
        count++;
      }
    }
  }

  if (count == 0 && saved_key != "")
  {
    options.insert(saved_key, "");
  }

  return options;
}

void parse_modules(options_i& options, vw& all, std::vector<std::string>& dictionary_nses)
{
  option_group_definition rand_options("Randomization options");
  rand_options.add(make_option("random_seed", all.rc.random_seed).help("seed random number generator"));
  options.add_and_parse(rand_options);
  all.get_random_state()->set_random_state(all.rc.random_seed);

  parse_feature_tweaks(options, all, dictionary_nses);  // feature tweaks

  parse_example_tweaks(options, all);  // example manipulation

  parse_output_model(options, all);

  parse_output_preds(options, all);

  parse_reductions(options, all);

  if (!all.logger.quiet)
  {
    all.oc.trace_message << "Num weight bits = " << all.fc.num_bits << endl;
    all.oc.trace_message << "learning rate = " << all.gs.eta << endl;
    all.oc.trace_message << "initial_t = " << all.sd->t << endl;
    all.oc.trace_message << "power_t = " << all.uc.power_t << endl;
    if (all.ec.numpasses > 1)
      all.oc.trace_message << "decay_learning_rate = " << all.uc.eta_decay_rate << endl;
  }
}

void parse_sources(options_i& options, vw& all, io_buf& model, bool skipModelLoad)
{
  if (!skipModelLoad)
    load_input_model(all, model);
  else
    model.close_file();

  auto parsed_source_options = parse_source(all, options);
  enable_sources(all, all.logger.quiet, all.ec.numpasses, parsed_source_options);

  // force wpp to be a power of 2 to avoid 32-bit overflow
  uint32_t i = 0;
  size_t params_per_problem = all.l->increment;
  while (params_per_problem > ((uint64_t)1 << i)) i++;
  all.gs.wpp = (1 << i) >> all.weights.stride_shift();
}

namespace VW
{
void cmd_string_replace_value(std::stringstream*& ss, std::string flag_to_replace, std::string new_value)
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
    size_t pos_after_value = cmd.find(" ", pos);
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
    sprintf(argv[i + 1], "%s", tokens[i].data());
  }

  argc = static_cast<int>(tokens.size() + 1);
  return argv;
}

char** to_argv(std::string const& s, int& argc)
{
  VW::string_view strview(s);
  std::vector<VW::string_view> foo;
  tokenize(' ', strview, foo);

  char** argv = calloc_or_throw<char*>(foo.size() + 1);
  // small optimization to avoid a string copy before tokenizing
  argv[0] = calloc_or_throw<char>(2);
  argv[0][0] = 'b';
  argv[0][1] = '\0';
  for (size_t i = 0; i < foo.size(); i++)
  {
    size_t len = foo[i].length();
    argv[i+1] = calloc_or_throw<char>(len + 1);
    memcpy(argv[i+1], foo[i].data(), len);
    // copy() is supported with boost::string_view, not with string_ref
    //foo[i].copy(argv[i], len);
    // unnecessary because of the calloc, but needed if we change stuff in the future
    // argv[i][len] = '\0';
  }

  argc = (int)foo.size() + 1;
  return argv;
}

char** get_argv_from_string(std::string s, int& argc) { return to_argv(s, argc); }

void free_args(int argc, char* argv[])
{
  for (int i = 0; i < argc; i++) free(argv[i]);
  free(argv);
}

vw* initialize(
    options_i& options, io_buf* model, bool skipModelLoad, trace_message_t trace_listener, void* trace_context)
{
  vw& all = parse_args(options, trace_listener, trace_context);

  try
  {
    // if user doesn't pass in a model, read from options
    io_buf localModel;
    if (!model)
    {
      std::vector<std::string> all_initial_regressor_files(all.ic.initial_regressors);
      if (options.was_supplied("input_feature_regularizer"))
      {
        all_initial_regressor_files.push_back(all.ic.per_feature_regularizer_input);
      }
      read_regressor_file(all, all_initial_regressor_files, localModel);
      model = &localModel;
    }

    // Loads header of model files and loads the command line options into the options object.
    load_header_merge_options(options, all, *model);

    std::vector<std::string> dictionary_nses;
    parse_modules(options, all, dictionary_nses);

    parse_sources(options, all, *model, skipModelLoad);

    // we must delay so parse_mask is fully defined.
    for (size_t id = 0; id < dictionary_nses.size(); id++) parse_dictionary_argument(all, dictionary_nses[id]);

    options.check_unregistered();

    // upon direct query for help -- spit it out to stdout;
    if (options.get_typed_option<bool>("help").value())
    {
      cout << options.help();
      exit(0);
    }

    all.l->init_driver();

    return &all;
  }
  catch (std::exception& e)
  {
    all.oc.trace_message << "Error: " << e.what() << endl;
    finish(all);
    throw;
  }
  catch (...)
  {
    finish(all);
    throw;
  }
}

vw* initialize(std::string s, io_buf* model, bool skipModelLoad, trace_message_t trace_listener, void* trace_context)
{
  int argc = 0;
  char** argv = to_argv(s, argc);
  vw* ret = nullptr;

  try
  {
    ret = initialize(argc, argv, model, skipModelLoad, trace_listener, trace_context);
  }
  catch (...)
  {
    free_args(argc, argv);
    throw;
  }

  free_args(argc, argv);
  return ret;
}

vw* initialize_escaped(
    std::string const& s, io_buf* model, bool skipModelLoad, trace_message_t trace_listener, void* trace_context)
{
  int argc = 0;
  char** argv = to_argv_escaped(s, argc);
  vw* ret = nullptr;

  try
  {
    ret = initialize(argc, argv, model, skipModelLoad, trace_listener, trace_context);
  }
  catch (...)
  {
    free_args(argc, argv);
    throw;
  }

  free_args(argc, argv);
  return ret;
}

vw* initialize(
    int argc, char* argv[], io_buf* model, bool skipModelLoad, trace_message_t trace_listener, void* trace_context)
{
  options_i* options = new config::options_boost_po(argc, argv);
  vw* all = initialize(*options, model, skipModelLoad, trace_listener, trace_context);

  // When VW is deleted the options object will be cleaned up too.
  all->rc.should_delete_options = true;
  return all;
}

// Create a new VW instance while sharing the model with another instance
// The extra arguments will be appended to those of the other VW instance
vw* seed_vw_model(vw* vw_model, const std::string extra_args, trace_message_t trace_listener, void* trace_context)
{
  options_serializer_boost_po serializer;
  for (auto const& option : vw_model->options->get_all_options())
  {
    if (vw_model->options->was_supplied(option->m_name))
    {
      // ignore no_stdin since it will be added by vw::initialize, and ignore -i since we don't want to reload the
      // model.
      if (option->m_name == "no_stdin" || option->m_name == "initial_regressor")
      {
        continue;
      }

      serializer.add(*option);
    }
  }

  auto serialized_options = serializer.str();
  serialized_options = serialized_options + " " + extra_args;

  vw* new_model =
      VW::initialize(serialized_options.c_str(), nullptr, true /* skipModelLoad */, trace_listener, trace_context);
  free_it(new_model->sd);

  // reference model states stored in the specified VW instance
  new_model->weights.shallow_copy(vw_model->weights);  // regressor
  new_model->sd = vw_model->sd;                        // shared data
  new_model->p->_shared_data = new_model->sd;

  return new_model;
}

void sync_stats(vw& all)
{
  if (all.all_reduce != nullptr)
  {
    float loss = (float)all.sd->sum_loss;
    all.sd->sum_loss = (double)accumulate_scalar(all, loss);
    float weighted_labeled_examples = (float)all.sd->weighted_labeled_examples;
    all.sd->weighted_labeled_examples = (double)accumulate_scalar(all, weighted_labeled_examples);
    float weighted_labels = (float)all.sd->weighted_labels;
    all.sd->weighted_labels = (double)accumulate_scalar(all, weighted_labels);
    float weighted_unlabeled_examples = (float)all.sd->weighted_unlabeled_examples;
    all.sd->weighted_unlabeled_examples = (double)accumulate_scalar(all, weighted_unlabeled_examples);
    float example_number = (float)all.sd->example_number;
    all.sd->example_number = (uint64_t)accumulate_scalar(all, example_number);
    float total_features = (float)all.sd->total_features;
    all.sd->total_features = (uint64_t)accumulate_scalar(all, total_features);
  }
}

void finish(vw& all, bool delete_all)
{
  // also update VowpalWabbit::PerformanceStatistics::get() (vowpalwabbit.cpp)
  if (!all.logger.quiet && !all.options->was_supplied("audit_regressor"))
  {
    all.oc.trace_message.precision(6);
    all.oc.trace_message << std::fixed;
    all.oc.trace_message << endl << "finished run";
    if (all.gs.current_pass == 0 || all.gs.current_pass == 1)
      all.oc.trace_message << endl << "number of examples = " << all.sd->example_number;
    else
    {
      all.oc.trace_message << endl << "number of examples per pass = " << all.sd->example_number / all.gs.current_pass;
      all.oc.trace_message << endl << "passes used = " << all.gs.current_pass;
    }
    all.oc.trace_message << endl << "weighted example sum = " << all.sd->weighted_examples();
    all.oc.trace_message << endl << "weighted label sum = " << all.sd->weighted_labels;
    all.oc.trace_message << endl << "average loss = ";
    if (all.ec.holdout_set_off)
      if (all.sd->weighted_labeled_examples > 0)
        all.oc.trace_message << all.sd->sum_loss / all.sd->weighted_labeled_examples;
      else
        all.oc.trace_message << "n.a.";
    else if ((all.sd->holdout_best_loss == FLT_MAX) || (all.sd->holdout_best_loss == FLT_MAX * 0.5))
      all.oc.trace_message << "undefined (no holdout)";
    else
      all.oc.trace_message << all.sd->holdout_best_loss << " h";
    if (all.sd->report_multiclass_log_loss)
    {
      if (all.ec.holdout_set_off)
        all.oc.trace_message << endl
                          << "average multiclass log loss = "
                          << all.sd->multiclass_log_loss / all.sd->weighted_labeled_examples;
      else
        all.oc.trace_message << endl
                          << "average multiclass log loss = "
                          << all.sd->holdout_multiclass_log_loss / all.sd->weighted_labeled_examples << " h";
    }

    float best_constant;
    float best_constant_loss;
    if (get_best_constant(all, best_constant, best_constant_loss))
    {
      all.oc.trace_message << endl << "best constant = " << best_constant;
      if (best_constant_loss != FLT_MIN)
        all.oc.trace_message << endl << "best constant's loss = " << best_constant_loss;
    }

    all.oc.trace_message << endl << "total feature number = " << all.sd->total_features;
    if (all.sd->queries > 0)
      all.oc.trace_message << endl << "total queries = " << all.sd->queries;
    all.oc.trace_message << endl;
  }

  // implement finally.
  // finalize_regressor can throw if it can't write the file.
  // we still want to free up all the memory.
  vw_exception finalize_regressor_exception(__FILE__, __LINE__, "empty");
  bool finalize_regressor_exception_thrown = false;
  try
  {
    finalize_regressor(all, all.final_regressor_name);
  }
  catch (vw_exception& e)
  {
    finalize_regressor_exception = e;
    finalize_regressor_exception_thrown = true;
  }

  if (delete_all)
    delete &all;

  if (finalize_regressor_exception_thrown)
    throw finalize_regressor_exception;
}
}  // namespace VW
