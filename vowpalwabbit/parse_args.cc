/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <stdio.h>
#include <float.h>
#include <sstream>
#include <fstream>
//#include <boost/filesystem.hpp>
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
#include "cb_explore.h"
#include "cb_explore_adf.h"
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
#include "stagewise_poly.h"
#include "active.h"
#include "active_cover.h"
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
// #include "cntk.h"

using namespace std;
//
// Does string end with a certain substring?
//
bool ends_with(string const &fullString, string const &ending)
{ if (fullString.length() > ending.length())
  { return (fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0);
  }
  else
  { return false;
  }
}

unsigned long long hash_file_contents(io_buf *io, int f)
{ unsigned long long v = 5289374183516789128;
  unsigned char buf[1024];
  while (true)
  { ssize_t n = io->read_file(f, buf, 1024);
    if (n <= 0) break;
    for (ssize_t i=0; i<n; i++)
    { v *= 341789041;
      v += buf[i];
    }
  }
  return v;
}

bool directory_exists(string path)
{ struct stat info;
  if (stat(path.c_str(), &info) != 0)
    return false;
  else
    return (info.st_mode & S_IFDIR) > 0;
  //  boost::filesystem::path p(path);
  //  return boost::filesystem::exists(p) && boost::filesystem::is_directory(p);
}

string find_in_path(vector<string> paths, string fname)
{
#ifdef _WIN32
  string delimiter = "\\";
#else
  string delimiter = "/";
#endif
  for (string path : paths)
  { string full = ends_with(path, delimiter) ? (path + fname) : (path + delimiter + fname);
    ifstream f(full.c_str());
    if (f.good())
      return full;
  }
  return "";
  /*
    for (string path : paths) {
      boost::filesystem::path p(path);
      p /= fname;
      if (boost::filesystem::exists(p) && !boost::filesystem::is_directory(p))
        return p;
    }
  */
}

void parse_dictionary_argument(vw&all, string str)
{ if (str.length() == 0) return;
  // expecting 'namespace:file', for instance 'w:foo.txt'
  // in the case of just 'foo.txt' it's applied to the default namespace

  char ns = ' ';
  const char*s  = str.c_str();
  if ((str.length() > 2) && (str[1] == ':'))
  { ns = str[0];
    s  += 2;
  }

  string fname = find_in_path(all.dictionary_path, string(s));
  if (fname == "")
    THROW("error: cannot find dictionary '" << s << "' in path; try adding --dictionary_path");

  bool is_gzip = ends_with(fname, ".gz");
  io_buf* io = is_gzip ? new comp_io_buf : new io_buf;
  int fd = io->open_file(fname.c_str(), all.stdin_off, io_buf::READ);
  if (fd < 0)
    THROW("error: cannot read dictionary from file '" << fname << "'" << ", opening failed");

  unsigned long long fd_hash = hash_file_contents(io, fd);
  io->close_file();

  if (! all.quiet)
    all.trace_message << "scanned dictionary '" << s << "' from '" << fname << "', hash=" << hex << fd_hash << dec << endl;

  // see if we've already read this dictionary
  for (size_t id=0; id<all.loaded_dictionaries.size(); id++)
    if (all.loaded_dictionaries[id].file_hash == fd_hash)
    { all.namespace_dictionaries[(size_t)ns].push_back(all.loaded_dictionaries[id].dict);
      io->close_file();
      delete io;
      return;
    }

  fd = io->open_file(fname.c_str(), all.stdin_off, io_buf::READ);
  if (fd < 0)
  { delete io;
    THROW("error: cannot re-read dictionary from file '" << fname << "'" << ", opening failed");
  }

  feature_dict* map = new feature_dict(1023, nullptr, substring_equal);
  example *ec = VW::alloc_examples(all.p->lp.label_size, 1);

  size_t def = (size_t)' ';

  ssize_t size = 2048, pos, nread;
  char rc;
  char*buffer = calloc_or_throw<char>(size);
  do
  { pos = 0;
    do
    { nread = io->read_file(fd, &rc, 1);
      if ((rc != EOF) && (nread > 0)) buffer[pos++] = rc;
      if (pos >= size - 1)
      { size *= 2;
        buffer = (char*)realloc(buffer, size);
        if (buffer == nullptr)
        { free(ec);
          VW::dealloc_example(all.p->lp.delete_label, *ec);
          delete map;
          io->close_file();
          delete io;
          THROW("error: memory allocation failed in reading dictionary");
        }
      }
    }
    while ( (rc != EOF) && (rc != '\n') && (nread > 0) );
    buffer[pos] = 0;

    // we now have a line in buffer
    char* c = buffer;
    while (*c == ' ' || *c == '\t') ++c; // skip initial whitespace
    char* d = c;
    while (*d != ' ' && *d != '\t' && *d != '\n' && *d != '\0') ++d; // gobble up initial word
    if (d == c) continue; // no word
    if (*d != ' ' && *d != '\t') continue; // reached end of line
    char* word = calloc_or_throw<char>(d-c);
    memcpy(word, c, d-c);
    substring ss = { word, word + (d - c) };
    uint64_t hash = uniform_hash( ss.begin, ss.end-ss.begin, quadratic_constant);
    if (map->get(ss, hash) != nullptr)   // don't overwrite old values!
    { free(word);
      continue;
    }
    d--;
    *d = '|';  // set up for parser::read_line
    VW::read_line(all, ec, d);
    // now we just need to grab stuff from the default namespace of ec!
    if (ec->feature_space[def].size() == 0)
    { free(word);
      continue;
    }
    features* arr = new features;
    arr->deep_copy_from(ec->feature_space[def]);
    map->put(ss, hash, arr);

    // clear up ec
    ec->tag.erase(); ec->indices.erase();
    for (size_t i=0; i<256; i++) { ec->feature_space[i].erase();}
  }
  while ((rc != EOF) && (nread > 0));
  free(buffer);
  io->close_file();
  delete io;
  VW::dealloc_example(all.p->lp.delete_label, *ec);
  free(ec);

  if (! all.quiet)
    all.trace_message << "dictionary " << s << " contains " << map->size() << " item" << (map->size() == 1 ? "" : "s") << endl;

  all.namespace_dictionaries[(size_t)ns].push_back(map);
  dictionary_info info = { calloc_or_throw<char>(strlen(s)+1), fd_hash, map };
  strcpy(info.name, s);
  all.loaded_dictionaries.push_back(info);
}

void parse_affix_argument(vw&all, string str)
{ if (str.length() == 0) return;
  char* cstr = calloc_or_throw<char>(str.length()+1);
  strcpy(cstr, str.c_str());

  char*p = strtok(cstr, ",");

  try
  { while (p != 0)
    { char*q = p;
      uint16_t prefix = 1;
      if (q[0] == '+') { q++; }
      else if (q[0] == '-') { prefix = 0; q++; }
      if ((q[0] < '1') || (q[0] > '7'))
        THROW("malformed affix argument (length must be 1..7): " << p);

      uint16_t len = (uint16_t)(q[0] - '0');
      uint16_t ns = (uint16_t)' ';  // default namespace
      if (q[1] != 0)
      { if (valid_ns(q[1]))
          ns = (uint16_t)q[1];
        else
          THROW("malformed affix argument (invalid namespace): " << p);

        if (q[2] != 0)
          THROW("malformed affix argument (too long): " << p);
      }

      uint16_t afx = (len << 1) | (prefix & 0x1);
      all.affix_features[ns] <<= 4;
      all.affix_features[ns] |=  afx;

      p = strtok(nullptr, ",");
    }
  }
  catch(...)
  { free(cstr);
    throw;
  }

  free(cstr);
}

void parse_diagnostics(vw& all, int argc)
{ new_options(all, "Diagnostic options")
  ("version","Version information")
  ("audit,a", "print weights of features")
  ("progress,P", po::value< string >(), "Progress update frequency. int: additive, float: multiplicative")
  ("quiet", "Don't output disgnostics and progress updates")
  ("help,h","Look here: http://hunch.net/~vw/ and click on Tutorial.");
  add_options(all);

  po::variables_map& vm = all.vm;

  if (vm.count("version"))
  { /* upon direct query for version -- spit it out to stdout */
    cout << version.to_string() << "\n";
    exit(0);
  }

  if (vm.count("quiet"))
  { all.quiet = true;
    // --quiet wins over --progress
  }
  else
  {  all.quiet = false;

    if (vm.count("progress"))
    { string progress_str = vm["progress"].as<string>();
      all.progress_arg = (float)::atof(progress_str.c_str());

      // --progress interval is dual: either integer or floating-point
      if (progress_str.find_first_of(".") == string::npos)
      { // No "." in arg: assume integer -> additive
        all.progress_add = true;
        if (all.progress_arg < 1)
        { all.trace_message    << "warning: additive --progress <int>"
                  << " can't be < 1: forcing to 1" << endl;
          all.progress_arg = 1;

        }
        all.sd->dump_interval = all.progress_arg;

      }
      else
      { // A "." in arg: assume floating-point -> multiplicative
        all.progress_add = false;

        if (all.progress_arg <= 1.0)
        { all.trace_message    << "warning: multiplicative --progress <float>: "
                  << vm["progress"].as<string>()
                  << " is <= 1.0: adding 1.0"
				  << endl;
          all.progress_arg += 1.0;

        }
        else if (all.progress_arg > 9.0)
        { all.trace_message    << "warning: multiplicative --progress <float>"
                  << " is > 9.0: you probably meant to use an integer"
				  << endl;
        }
        all.sd->dump_interval = 1.0;
      }
    }
  }

  if (vm.count("audit"))
  { all.audit = true;
  }
}

void parse_source(vw& all)
{ new_options(all, "Input options")
  ("data,d", po::value< string >(), "Example Set")
  ("daemon", "persistent daemon mode on port 26542")
  ("foreground", "in persistent daemon mode, do not run in the background")
  ("port", po::value<size_t>(),"port to listen on; use 0 to pick unused port")
  ("num_children", po::value<size_t>(&(all.num_children)), "number of children for persistent daemon mode")
  ("pid_file", po::value< string >(), "Write pid file in persistent daemon mode")
  ("port_file", po::value< string >(), "Write port used in persistent daemon mode")
  ("cache,c", "Use a cache.  The default is <data>.cache")
  ("cache_file", po::value< vector<string> >(), "The location(s) of cache_file.")
  ("json", "Enable JSON parsing.")
  ("dsjson", "Enable Decision Service JSON parsing.")
  ("kill_cache,k", "do not reuse existing cache: create a new one always")
  ("compressed", "use gzip format whenever possible. If a cache file is being created, this option creates a compressed cache file. A mixture of raw-text & compressed inputs are supported with autodetection.")
  ("no_stdin", "do not default to reading from stdin");
  add_options(all);

  // Be friendly: if -d was left out, treat positional param as data file
  po::positional_options_description p;
  p.add("data", -1);
  po::parsed_options pos = po::command_line_parser(all.args).
                           style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
                           options(all.opts).positional(p).run();
  all.vm = po::variables_map();
  po::store(pos, all.vm);
  po::variables_map& vm = all.vm;

  //begin input source
  if (vm.count("no_stdin"))
    all.stdin_off = true;

  if ( (vm.count("total") || vm.count("node") || vm.count("unique_id")) && !(vm.count("total") && vm.count("node") && vm.count("unique_id")) )
    THROW("you must specificy unique_id, total, and node if you specify any");

  if (vm.count("daemon") || vm.count("pid_file") || (vm.count("port") && !all.active) )
  { all.daemon = true;

    // allow each child to process up to 1e5 connections
    all.numpasses = (size_t) 1e5;
  }

  if (vm.count("compressed"))
    set_compressed(all.p);

  if (vm.count("data"))
  { all.data_filename = vm["data"].as<string>();
    if (ends_with(all.data_filename, ".gz"))
      set_compressed(all.p);
  }
  else
    all.data_filename = "";

  if ((vm.count("cache") || vm.count("cache_file")) && vm.count("invert_hash"))
    THROW("invert_hash is incompatible with a cache file.  Use it in single pass mode only.");

  if(!all.holdout_set_off && (vm.count("output_feature_regularizer_binary") || vm.count("output_feature_regularizer_text")))
  { all.holdout_set_off = true;
    all.trace_message<<"Making holdout_set_off=true since output regularizer specified" << endl;
  }
}

bool interactions_settings_doubled = false; // local setting setted in parse_modules()
namespace VW
{
const char* are_features_compatible(vw& vw1, vw& vw2)
{ if (vw1.p->hasher != vw2.p->hasher)
    return "hasher";

  if (!equal(vw1.spelling_features, vw1.spelling_features + (sizeof(vw1.spelling_features) / sizeof(bool)), vw2.spelling_features))
    return "spelling_features";

  if (!equal(vw1.affix_features, vw1.affix_features + (sizeof(vw1.affix_features) / sizeof(uint32_t)), vw2.affix_features))
    return "affix_features";

  if (!equal(vw1.ngram, vw1.ngram + (sizeof(vw1.ngram) / sizeof(uint32_t)), vw2.ngram))
    return "ngram";

  if (!equal(vw1.skips, vw1.skips + (sizeof(vw1.skips) / sizeof(uint32_t)), vw2.skips))
    return "skips";

  if (!equal(vw1.limit, vw1.limit + (sizeof(vw1.limit) / sizeof(uint32_t)), vw2.limit))
    return "limit";

  if (vw1.num_bits != vw2.num_bits)
    return "num_bits";

  if (vw1.permutations != vw2.permutations)
    return "permutations";

  if (vw1.interactions.size() != vw2.interactions.size())
    return "interactions size";

  if (vw1.ignore_some != vw2.ignore_some)
    return "ignore_some";

  if (vw1.ignore_some && !equal(vw1.ignore, vw1.ignore + (sizeof(vw1.ignore) / sizeof(bool)), vw2.ignore))
    return "ignore";

  if (vw1.ignore_some_linear != vw2.ignore_some_linear)
    return "ignore_some_linear";

  if (vw1.ignore_some_linear && !equal(vw1.ignore_linear, vw1.ignore_linear + (sizeof(vw1.ignore_linear) / sizeof(bool)), vw2.ignore_linear))
    return "ignore_linear";

  if (vw1.redefine_some != vw2.redefine_some)
    return "redefine_some";

  if (vw1.redefine_some && !equal(vw1.redefine, vw1.redefine + (sizeof(vw1.redefine) / sizeof(unsigned char)), vw2.redefine))
    return "redefine";

  if (vw1.add_constant != vw2.add_constant)
    return "add_constant";

  if (vw1.dictionary_path.size() != vw2.dictionary_path.size())
    return "dictionary_path size";

  if (!equal(vw1.dictionary_path.begin(), vw1.dictionary_path.end(), vw2.dictionary_path.begin()))
    return "dictionary_path";

  for (v_string *i = vw1.interactions.begin(), *j = vw2.interactions.begin(); i != vw1.interactions.end(); i++, j++)
    if (v_string2string(*i) != v_string2string(*j))
      return "interaction mismatch";

  return nullptr;
}
}
// return a copy of string replacing \x00 sequences in it
string spoof_hex_encoded_namespaces(const string& arg)
{ string res;
  int pos = 0;
  while (pos < (int)arg.size()-3)
  { if (arg[pos] == '\\' && arg[pos+1] == 'x')
    { string substr = arg.substr(pos+2,2);
      char* p;
      unsigned char c = (unsigned char) strtoul(substr.c_str(), &p, 16);
      if (*p == '\0')
      { res.push_back(c);
        pos += 4;
      }
      else
      { cerr << "Possibly malformed hex representation of a namespace: '\\x" << substr << "'\n";
        res.push_back(arg[pos++]);
      }
    }
    else
      res.push_back(arg[pos++]);
  }

  while (pos < (int)arg.size()) //copy last 2 characters
    res.push_back(arg[pos++]);

  return res;
}

void parse_feature_tweaks(vw& all)
{ new_options(all, "Feature options")
  ("hash", po::value< string > (), "how to hash the features. Available options: strings, all")
  ("ignore", po::value< vector<string> >(), "ignore namespaces beginning with character <arg>")
  ("ignore_linear", po::value< vector<string> >(), "ignore namespaces beginning with character <arg> for linear terms only")
  ("keep", po::value< vector<string> >(), "keep namespaces beginning with character <arg>")
  ("redefine", po::value< vector<string> >(), "redefine namespaces beginning with characters of string S as namespace N. <arg> shall be in form 'N:=S' where := is operator. Empty N or S are treated as default namespace. Use ':' as a wildcard in S.")
  ("bit_precision,b", po::value<size_t>(), "number of bits in the feature table")
  ("noconstant", "Don't add a constant feature")
  ("constant,C", po::value<float>(&(all.initial_constant)), "Set initial value of constant")
  ("ngram", po::value< vector<string> >(), "Generate N grams. To generate N grams for a single namespace 'foo', arg should be fN.")
  ("skips", po::value< vector<string> >(), "Generate skips in N grams. This in conjunction with the ngram tag can be used to generate generalized n-skip-k-gram. To generate n-skips for a single namespace 'foo', arg should be fN.")
  ("feature_limit", po::value< vector<string> >(), "limit to N features. To apply to a single namespace 'foo', arg should be fN")
  ("affix", po::value<string>(), "generate prefixes/suffixes of features; argument '+2a,-3b,+1' means generate 2-char prefixes for namespace a, 3-char suffixes for b and 1 char prefixes for default namespace")
  ("spelling", po::value< vector<string> >(), "compute spelling features for a give namespace (use '_' for default namespace)")
  ("dictionary", po::value< vector<string> >(), "read a dictionary for additional features (arg either 'x:file' or just 'file')")
  ("dictionary_path", po::value< vector<string> >(), "look in this directory for dictionaries; defaults to current directory or env{PATH}")
  ("interactions", po::value< vector<string> > (), "Create feature interactions of any level between namespaces.")
  ("permutations", "Use permutations instead of combinations for feature interactions of same namespace.")
  ("leave_duplicate_interactions", "Don't remove interactions with duplicate combinations of namespaces. For ex. this is a duplicate: '-q ab -q ba' and a lot more in '-q ::'.")
  ("quadratic,q", po::value< vector<string> > (), "Create and use quadratic features")
  ("q:", po::value< string >(), ": corresponds to a wildcard for all printable characters")
  ("cubic", po::value< vector<string> > (),
   "Create and use cubic features");
  add_options(all);

  po::variables_map& vm = all.vm;

  //feature manipulation
  string hash_function("strings");
  if(vm.count("hash"))
  { hash_function = vm["hash"].as<string>();
    *all.file_options << " --hash " << hash_function;
  }
  all.p->hasher = getHasher(hash_function);

  if (vm.count("spelling"))
  { vector<string> spelling_ns = vm["spelling"].as< vector<string> >();
    for (size_t id=0; id<spelling_ns.size(); id++)
    { spelling_ns[id] = spoof_hex_encoded_namespaces(spelling_ns[id]);
      if (spelling_ns[id][0] == '_') all.spelling_features[(unsigned char)' '] = true;
      else all.spelling_features[(size_t)spelling_ns[id][0]] = true;
      *all.file_options << " --spelling " << spelling_ns[id];
    }
  }

  if (vm.count("affix"))
  { parse_affix_argument(all, spoof_hex_encoded_namespaces(vm["affix"].as<string>()));
    *all.file_options << " --affix " << vm["affix"].as<string>();
  }

  if(vm.count("ngram"))
  { if(vm.count("sort_features"))
      THROW("ngram is incompatible with sort_features.");

    all.ngram_strings = vm["ngram"].as< vector<string> >();
    for (size_t i = 0; i < all.ngram_strings.size(); i++)
      all.ngram_strings[i] = spoof_hex_encoded_namespaces(all.ngram_strings[i]);
    compile_gram(all.ngram_strings, all.ngram, (char*)"grams", all.quiet);
  }

  if(vm.count("skips"))
  { if(!vm.count("ngram"))
      THROW("You can not skip unless ngram is > 1");

    all.skip_strings = vm["skips"].as<vector<string> >();
    for (size_t i = 0; i < all.skip_strings.size(); i++)
      all.skip_strings[i] = spoof_hex_encoded_namespaces(all.skip_strings[i]);
    compile_gram(all.skip_strings, all.skips, (char*)"skips", all.quiet);
  }

  if(vm.count("feature_limit"))
  { all.limit_strings = vm["feature_limit"].as< vector<string> >();
    compile_limits(all.limit_strings, all.limit, all.quiet);
  }

  if (vm.count("bit_precision"))
  { uint32_t new_bits = (uint32_t)vm["bit_precision"].as< size_t>();
    if (all.default_bits == false && new_bits != all.num_bits)
      THROW("Number of bits is set to " << new_bits << " and " << all.num_bits << " by argument and model.  That does not work.");

    all.default_bits = false;
    all.num_bits = new_bits;

    VW::validate_num_bits(all);
  }

  all.permutations = vm.count("permutations") > 0;

  // prepare namespace interactions
  v_array<v_string> expanded_interactions = v_init<v_string>();

  if ( ( ((!all.pairs.empty() || !all.triples.empty() || !all.interactions.empty()) && /*data was restored from old model file directly to v_array and will be overriden automatically*/
          (vm.count("quadratic") || vm.count("cubic") || vm.count("interactions")) ) )
       ||
       interactions_settings_doubled /*settings were restored from model file to file_options and overriden by params from command line*/)
  { all.trace_message << "WARNING: model file has set of {-q, --cubic, --interactions} settings stored, but they'll be OVERRIDEN by set of {-q, --cubic, --interactions} settings from command line." << endl;

    // in case arrays were already filled in with values from old model file - reset them
    if (!all.pairs.empty()) all.pairs.clear();
    if (!all.triples.empty()) all.triples.clear();
    if (all.interactions.size() > 0)
    { for (v_string* i = all.interactions.begin(); i != all.interactions.end(); ++i) i->delete_v();
      all.interactions.delete_v();
    }
  }

  if (vm.count("quadratic"))
  { vector<string> vec_arg = vm["quadratic"].as< vector<string> >();
    if (!all.quiet)
      all.trace_message << "creating quadratic features for pairs: ";

    for (vector<string>::iterator i = vec_arg.begin(); i != vec_arg.end(); ++i)
    { *all.file_options << " --quadratic " << *i;
      *i = spoof_hex_encoded_namespaces(*i);
      if (!all.quiet) all.trace_message << *i << " ";
    }

    expanded_interactions = INTERACTIONS::expand_interactions(vec_arg, 2, "error, quadratic features must involve two sets.");

    if (!all.quiet) all.trace_message << endl;
  }

  if (vm.count("cubic"))
  { vector<string> vec_arg = vm["cubic"].as< vector<string> >();
    if (!all.quiet)
      all.trace_message << "creating cubic features for triples: ";
    for (vector<string>::iterator i = vec_arg.begin(); i != vec_arg.end(); ++i)
    { *all.file_options << " --cubic " << *i;
      *i = spoof_hex_encoded_namespaces(*i);
      if (!all.quiet) all.trace_message << *i << " ";
    }

    v_array<v_string> exp_cubic = INTERACTIONS::expand_interactions(vec_arg, 3, "error, cubic features must involve three sets.");
    push_many(expanded_interactions, exp_cubic.begin(), exp_cubic.size());
    exp_cubic.delete_v();

    if (!all.quiet) all.trace_message << endl;
  }

  if (vm.count("interactions"))
  { vector<string> vec_arg = vm["interactions"].as< vector<string> >();
    if (!all.quiet)
      all.trace_message << "creating features for following interactions: ";
    for (vector<string>::iterator i = vec_arg.begin(); i != vec_arg.end(); ++i)
    { *all.file_options << " --interactions " << *i;
      *i = spoof_hex_encoded_namespaces(*i);
      if (!all.quiet) all.trace_message << *i << " ";
    }

    v_array<v_string> exp_inter = INTERACTIONS::expand_interactions(vec_arg, 0, "");
    push_many(expanded_interactions, exp_inter.begin(), exp_inter.size());
    exp_inter.delete_v();

    if (!all.quiet) all.trace_message << endl;
  }

  if (expanded_interactions.size() > 0)
  {

    size_t removed_cnt;
    size_t sorted_cnt;
    INTERACTIONS::sort_and_filter_duplicate_interactions(expanded_interactions, !vm.count("leave_duplicate_interactions"), removed_cnt, sorted_cnt);

    if (removed_cnt > 0)
      all.trace_message << "WARNING: duplicate namespace interactions were found. Removed: " << removed_cnt << '.' << endl << "You can use --leave_duplicate_interactions to disable this behaviour." << endl;
    if (sorted_cnt > 0)
      all.trace_message << "WARNING: some interactions contain duplicate characters and their characters order has been changed. Interactions affected: " << sorted_cnt << '.' << endl;


    if (all.interactions.size() > 0)
    { // should be empty, but just in case...
      for (v_string& i : all.interactions) i.delete_v();
      all.interactions.delete_v();
    }

    all.interactions = expanded_interactions;

    // copy interactions of size 2 and 3 to old vectors for backward compatibility
    for (v_string& i : expanded_interactions)
    { const size_t len = i.size();
      if (len == 2)
        all.pairs.push_back(v_string2string(i));
      else if (len == 3)
        all.triples.push_back(v_string2string(i));
    }
  }


  for (size_t i = 0; i < 256; i++)
    {
      all.ignore[i] = false;
      all.ignore_linear[i] = false;
    }
  all.ignore_some = false;
  all.ignore_some_linear = false;

  if (vm.count("ignore"))
  { all.ignore_some = true;

    vector<string> ignore = vm["ignore"].as< vector<string> >();
    for (vector<string>::iterator i = ignore.begin(); i != ignore.end(); i++)
    { *i = spoof_hex_encoded_namespaces(*i);
      *all.file_options << " --ignore " << *i;
      for (string::const_iterator j = i->begin(); j != i->end(); j++)
        all.ignore[(size_t)(unsigned char)*j] = true;

    }

    if (!all.quiet)
    { all.trace_message << "ignoring namespaces beginning with: ";
      for (vector<string>::iterator i = ignore.begin(); i != ignore.end(); i++)
        for (string::const_iterator j = i->begin(); j != i->end(); j++)
          all.trace_message << *j << " ";

      all.trace_message << endl;
    }
  }

  if (vm.count("ignore_linear"))
  { all.ignore_some_linear = true;

    vector<string> ignore = vm["ignore_linear"].as< vector<string> >();
    for (vector<string>::iterator i = ignore.begin(); i != ignore.end(); i++)
    { *i = spoof_hex_encoded_namespaces(*i);
      *all.file_options << " --ignore_linear " << *i;
      for (string::const_iterator j = i->begin(); j != i->end(); j++)
        all.ignore_linear[(size_t)(unsigned char)*j] = true;

    }

    if (!all.quiet)
    { all.trace_message << "ignoring linear terms for namespaces beginning with: ";
      for (vector<string>::iterator i = ignore.begin(); i != ignore.end(); i++)
        for (string::const_iterator j = i->begin(); j != i->end(); j++)
          all.trace_message << *j << " ";

      all.trace_message << endl;
    }
  }

  if (vm.count("keep"))
  { for (size_t i = 0; i < 256; i++)
      all.ignore[i] = true;

    all.ignore_some = true;

    vector<string> keep = vm["keep"].as< vector<string> >();
    for (vector<string>::iterator i = keep.begin(); i != keep.end(); i++)
    { *i = spoof_hex_encoded_namespaces(*i);
      for (string::const_iterator j = i->begin(); j != i->end(); j++)
        all.ignore[(size_t)(unsigned char)*j] = false;
    }

    if (!all.quiet)
    { all.trace_message << "using namespaces beginning with: ";
      for (vector<string>::iterator i = keep.begin(); i != keep.end(); i++)
        for (string::const_iterator j = i->begin(); j != i->end(); j++)
          all.trace_message << *j << " ";

      all.trace_message << endl;
    }
  }

  // --redefine param code
  all.redefine_some = false; // false by default

  if (vm.count("redefine"))
  { // initail values: i-th namespace is redefined to i itself
    for (size_t i = 0; i < 256; i++)
      all.redefine[i] = (unsigned char)i;

    // note: --redefine declaration order is matter
    // so --redefine :=L --redefine ab:=M  --ignore L  will ignore all except a and b under new M namspace

    vector< string > arg_list = vm["redefine"].as< vector< string > >();
    for (vector<string>::iterator arg_iter = arg_list.begin(); arg_iter != arg_list.end(); arg_iter++)
    { string arg = spoof_hex_encoded_namespaces(*arg_iter);
      size_t arg_len = arg.length();

      size_t operator_pos = 0; //keeps operator pos + 1 to stay unsigned type
      bool operator_found = false;
      unsigned char new_namespace = ' ';

      // let's find operator ':=' position in N:=S
      for (size_t i = 0; i < arg_len; i++)
      { if (operator_found)
        { if (i > 2) { new_namespace = arg[0];} //N is not empty
          break;
        }
        else if (arg[i] == ':')
          operator_pos = i+1;
        else if ( (arg[i] == '=') && (operator_pos == i) )
          operator_found = true;
      }

      if (!operator_found)
        THROW("argument of --redefine is malformed. Valid format is N:=S, :=S or N:=");

      if (++operator_pos > 3) // seek operator end
        all.trace_message << "WARNING: multiple namespaces are used in target part of --redefine argument. Only first one ('" << new_namespace << "') will be used as target namespace." << endl;

      all.redefine_some = true;

      // case ':=S' doesn't require any additional code as new_namespace = ' ' by default

      if (operator_pos == arg_len) // S is empty, default namespace shall be used
        all.redefine[(int) ' '] = new_namespace;
      else
        for (size_t i = operator_pos; i < arg_len; i++)
        { // all namespaces from S are redefined to N
          unsigned char c = arg[i];
          if (c != ':')
            all.redefine[c] = new_namespace;
          else
          { // wildcard found: redefine all except default and break
            for (size_t i = 0; i < 256; i++)
              all.redefine[i] = new_namespace;
            break; //break processing S
          }
        }

    }
  }

  if (vm.count("dictionary"))
  { if (vm.count("dictionary_path"))
      for (string path : vm["dictionary_path"].as< vector<string> >())
        if (directory_exists(path))
          all.dictionary_path.push_back(path);
    if (directory_exists("."))
      all.dictionary_path.push_back(".");

    const std::string PATH = getenv( "PATH" );
#if _WIN32
    const char delimiter = ';';
#else
    const char delimiter = ':';
#endif
    if(!PATH.empty())
    { size_t previous = 0;
      size_t index = PATH.find( delimiter );
      while( index != string::npos )
      { all.dictionary_path.push_back( PATH.substr(previous, index-previous));
        previous=index+1;
        index = PATH.find( delimiter, previous );
      }
      all.dictionary_path.push_back( PATH.substr(previous) );
    }

    vector<string> dictionary_ns = vm["dictionary"].as< vector<string> >();
    for (size_t id=0; id<dictionary_ns.size(); id++)
    { parse_dictionary_argument(all, dictionary_ns[id]);
      *all.file_options << " --dictionary " << dictionary_ns[id];
    }
  }

  if (vm.count("noconstant"))
    all.add_constant = false;
}

void parse_example_tweaks(vw& all)
{ string named_labels;
  new_options(all, "Example options")
  ("testonly,t", "Ignore label information and just test")
  ("holdout_off", "no holdout data in multiple passes")
  ("holdout_period", po::value<uint32_t>(&(all.holdout_period)), "holdout period for test only, default 10")
  ("holdout_after", po::value<uint32_t>(&(all.holdout_after)), "holdout after n training examples, default off (disables holdout_period)")
  ("early_terminate", po::value<size_t>(), "Specify the number of passes tolerated when holdout loss doesn't decrease before early termination, default is 3")
  ("passes", po::value<size_t>(&(all.numpasses)),"Number of Training Passes")
  ("initial_pass_length", po::value<size_t>(&(all.pass_length)), "initial number of examples per pass")
  ("examples", po::value<size_t>(&(all.max_examples)), "number of examples to parse")
  ("min_prediction", po::value<float>(&(all.sd->min_label)), "Smallest prediction to output")
  ("max_prediction", po::value<float>(&(all.sd->max_label)), "Largest prediction to output")
  ("sort_features", "turn this on to disregard order in which features have been defined. This will lead to smaller cache sizes")
  ("loss_function", po::value<string>()->default_value("squared"), "Specify the loss function to be used, uses squared by default. Currently available ones are squared, classic, hinge, logistic, quantile and poisson.")
  ("quantile_tau", po::value<float>()->default_value(0.5), "Parameter \\tau associated with Quantile loss. Defaults to 0.5")
  ("l1", po::value<float>(&(all.l1_lambda)), "l_1 lambda")
  ("l2", po::value<float>(&(all.l2_lambda)), "l_2 lambda")
  ("no_bias_regularization", po::value<bool>(&(all.no_bias)),"no bias in regularization")
  ("named_labels", po::value<string>(&named_labels), "use names for labels (multiclass, etc.) rather than integers, argument specified all possible labels, comma-sep, eg \"--named_labels Noun,Verb,Adj,Punc\"");
  add_options(all);

  po::variables_map& vm = all.vm;
  if (vm.count("testonly") || all.eta == 0.)
  { if (!all.quiet)
      all.trace_message << "only testing" << endl;
    all.training = false;
    if (all.lda > 0)
      all.eta = 0;
  }
  else
    all.training = true;

  if(all.numpasses > 1 || all.holdout_after > 0)
    all.holdout_set_off = false;

  if(vm.count("holdout_off"))
    all.holdout_set_off = true;

  if(vm.count("sort_features"))
    all.p->sort_features = true;

  if (vm.count("min_prediction"))
    all.sd->min_label = vm["min_prediction"].as<float>();
  if (vm.count("max_prediction"))
    all.sd->max_label = vm["max_prediction"].as<float>();
  if (vm.count("min_prediction") || vm.count("max_prediction") || vm.count("testonly"))
    all.set_minmax = noop_mm;

  if (vm.count("named_labels"))
  { *all.file_options << " --named_labels " << named_labels << ' ';
    all.sd->ldict = new namedlabels(named_labels);
    if (!all.quiet)
      all.trace_message << "parsed " << all.sd->ldict->getK() << " named labels" << endl;
  }

  string loss_function = vm["loss_function"].as<string>();
  float loss_parameter = 0.0;
  if(vm.count("quantile_tau"))
    loss_parameter = vm["quantile_tau"].as<float>();

  all.loss = getLossFunction(all, loss_function, (float)loss_parameter);

  if (all.l1_lambda < 0.)
  { all.trace_message << "l1_lambda should be nonnegative: resetting from " << all.l1_lambda << " to 0" << endl;
    all.l1_lambda = 0.;
  }
  if (all.l2_lambda < 0.)
  { all.trace_message << "l2_lambda should be nonnegative: resetting from " << all.l2_lambda << " to 0" << endl;
    all.l2_lambda = 0.;
  }
  all.reg_mode += (all.l1_lambda > 0.) ? 1 : 0;
  all.reg_mode += (all.l2_lambda > 0.) ? 2 : 0;
  if (!all.quiet)
  { if (all.reg_mode %2 && !vm.count("bfgs"))
      all.trace_message << "using l1 regularization = " << all.l1_lambda << endl;
    if (all.reg_mode > 1)
      all.trace_message << "using l2 regularization = " << all.l2_lambda << endl;
  }
}

void parse_output_preds(vw& all)
{ new_options(all, "Output options")
  ("predictions,p", po::value< string >(), "File to output predictions to")
  ("raw_predictions,r", po::value< string >(), "File to output unnormalized predictions to");
  add_options(all);

  po::variables_map& vm = all.vm;
  if (vm.count("predictions"))
  { if (!all.quiet)
      all.trace_message << "predictions = " <<  vm["predictions"].as< string >() << endl;
    if (strcmp(vm["predictions"].as< string >().c_str(), "stdout") == 0)
    { all.final_prediction_sink.push_back((size_t) 1);//stdout
    }
    else
    { const char* fstr = (vm["predictions"].as< string >().c_str());
      int f;
#ifdef _WIN32
      _sopen_s(&f, fstr, _O_CREAT|_O_WRONLY|_O_BINARY|_O_TRUNC, _SH_DENYWR, _S_IREAD|_S_IWRITE);
#else
      f = open(fstr, O_CREAT|O_WRONLY|O_LARGEFILE|O_TRUNC,0666);
#endif
      if (f < 0)
        all.trace_message << "Error opening the predictions file: " << fstr << endl;
      all.final_prediction_sink.push_back((size_t) f);
    }
  }

  if (vm.count("raw_predictions"))
  { if (!all.quiet)
    { all.trace_message << "raw predictions = " <<  vm["raw_predictions"].as< string >() << endl;
      if (vm.count("binary"))
        all.trace_message << "Warning: --raw_predictions has no defined value when --binary specified, expect no output" << endl;
    }
    if (strcmp(vm["raw_predictions"].as< string >().c_str(), "stdout") == 0)
      all.raw_prediction = 1;//stdout
    else
    { const char* t = vm["raw_predictions"].as< string >().c_str();
      int f;
#ifdef _WIN32
      _sopen_s(&f, t, _O_CREAT|_O_WRONLY|_O_BINARY|_O_TRUNC, _SH_DENYWR, _S_IREAD|_S_IWRITE);
#else
      f = open(t, O_CREAT|O_WRONLY|O_LARGEFILE|O_TRUNC,0666);
#endif
      all.raw_prediction = f;
    }
  }
}

void parse_output_model(vw& all)
{ new_options(all, "Output model")
  ("final_regressor,f", po::value< string >(), "Final regressor")
  ("readable_model", po::value< string >(), "Output human-readable final regressor with numeric features")
  ("invert_hash", po::value< string >(), "Output human-readable final regressor with feature names.  Computationally expensive.")
  ("save_resume", "save extra state so learning can be resumed later with new data")
  ("preserve_performance_counters", "reset performance counters when warmstarting")
  ("save_per_pass", "Save the model after every pass over data")
  ("output_feature_regularizer_binary", po::value< string >(&(all.per_feature_regularizer_output)), "Per feature regularization output file")
  ("output_feature_regularizer_text", po::value< string >(&(all.per_feature_regularizer_text)), "Per feature regularization output file, in text")
  ("id", po::value< string >(&(all.id)), "User supplied ID embedded into the final regressor");
  add_options(all);

  po::variables_map& vm = all.vm;
  if (vm.count("final_regressor"))
  { all.final_regressor_name = vm["final_regressor"].as<string>();
    if (!all.quiet)
      all.trace_message << "final_regressor = " << vm["final_regressor"].as<string>() << endl;
  }
  else
    all.final_regressor_name = "";

  if (vm.count("readable_model"))
    all.text_regressor_name = vm["readable_model"].as<string>();

  if (vm.count("invert_hash"))
  { all.inv_hash_regressor_name = vm["invert_hash"].as<string>();
    all.hash_inv = true;
  }

  if (vm.count("save_per_pass"))
    all.save_per_pass = true;

  if (vm.count("save_resume"))
    all.save_resume = true;

  if (vm.count("preserve_performance_counters"))
    all.preserve_performance_counters = true;
  
  if (vm.count("id") && find(all.args.begin(), all.args.end(), "--id") == all.args.end())
  { all.args.push_back("--id");
    all.args.push_back(vm["id"].as<string>());
  }
}

void load_input_model(vw& all, io_buf& io_temp)
{ // Need to see if we have to load feature mask first or second.
  // -i and -mask are from same file, load -i file first so mask can use it
  if (all.vm.count("feature_mask") && all.vm.count("initial_regressor")
      && all.vm["feature_mask"].as<string>() == all.vm["initial_regressor"].as< vector<string> >()[0])
  { // load rest of regressor
    all.l->save_load(io_temp, true, false);
    io_temp.close_file();

    // set the mask, which will reuse -i file we just loaded
    parse_mask_regressor_args(all);
  }
  else
  { // load mask first
    parse_mask_regressor_args(all);

    // load rest of regressor
    all.l->save_load(io_temp, true, false);
    io_temp.close_file();
  }
}

LEARNER::base_learner* setup_base(vw& all)
{ LEARNER::base_learner* ret = all.reduction_stack.pop()(all);
  if (ret == nullptr)
    return setup_base(all);
  else
    return ret;
}

void parse_reductions(vw& all)
{ new_options(all, "Reduction options, use [option] --help for more info");
  add_options(all);
  //Base algorithms
  all.reduction_stack.push_back(GD::setup);
  all.reduction_stack.push_back(kernel_svm_setup);
  all.reduction_stack.push_back(ftrl_setup);
  all.reduction_stack.push_back(svrg_setup);
  all.reduction_stack.push_back(sender_setup);
  all.reduction_stack.push_back(gd_mf_setup);
  all.reduction_stack.push_back(print_setup);
  all.reduction_stack.push_back(noop_setup);
  all.reduction_stack.push_back(lda_setup);
  all.reduction_stack.push_back(bfgs_setup);
  all.reduction_stack.push_back(OjaNewton_setup);
  // all.reduction_stack.push_back(VW_CNTK::setup);

  //Score Users
  all.reduction_stack.push_back(ExpReplay::expreplay_setup<'b', simple_label>);
  all.reduction_stack.push_back(active_setup);
  all.reduction_stack.push_back(active_cover_setup);
  all.reduction_stack.push_back(confidence_setup);
  all.reduction_stack.push_back(nn_setup);
  all.reduction_stack.push_back(mf_setup);
  all.reduction_stack.push_back(marginal_setup);
  all.reduction_stack.push_back(autolink_setup);
  all.reduction_stack.push_back(lrq_setup);
  all.reduction_stack.push_back(lrqfa_setup);
  all.reduction_stack.push_back(stagewise_poly_setup);
  all.reduction_stack.push_back(scorer_setup);

  //Reductions
  all.reduction_stack.push_back(binary_setup);

  all.reduction_stack.push_back(ExpReplay::expreplay_setup<'m', MULTICLASS::mc_label>);
  all.reduction_stack.push_back(topk_setup);
  all.reduction_stack.push_back(oaa_setup);
  all.reduction_stack.push_back(boosting_setup);
  all.reduction_stack.push_back(ect_setup);
  all.reduction_stack.push_back(log_multi_setup);
  all.reduction_stack.push_back(recall_tree_setup);
  all.reduction_stack.push_back(multilabel_oaa_setup);

  all.reduction_stack.push_back(csoaa_setup);
  all.reduction_stack.push_back(interact_setup);
  all.reduction_stack.push_back(csldf_setup);
  all.reduction_stack.push_back(cb_algs_setup);
  all.reduction_stack.push_back(cb_adf_setup);
  all.reduction_stack.push_back(mwt_setup);
  all.reduction_stack.push_back(cb_explore_setup);
  all.reduction_stack.push_back(cb_explore_adf_setup);
  all.reduction_stack.push_back(cbify_setup);
  all.reduction_stack.push_back(explore_eval_setup);

  all.reduction_stack.push_back(ExpReplay::expreplay_setup<'c', COST_SENSITIVE::cs_label>);
  all.reduction_stack.push_back(Search::setup);
  all.reduction_stack.push_back(bs_setup);

  all.reduction_stack.push_back(audit_regressor_setup);

  all.l = setup_base(all);
}

void add_to_args(vw& all, int argc, char* argv[], int excl_param_count = 0, const char* excl_params[] = NULL)
{ bool skip_next = false;

  for (int i = 1; i < argc; i++)
  { if (skip_next)
    { skip_next = false;
      continue;
    }

    for (int j = 0; j < excl_param_count; j++)
      if (std::strcmp(argv[i], excl_params[j]) == 0)
      { skip_next = true; //skip param arguement
        break;
      }

    if (skip_next) continue;

    all.args.push_back(string(argv[i]));
  }
}

vw& parse_args(int argc, char *argv[], trace_message_t trace_listener, void* trace_context)
{ vw& all = *(new vw());

  if (trace_listener)
  { all.trace_message.trace_listener = trace_listener;
    all.trace_message.trace_context = trace_context;
  }

  try
  { all.vw_is_main = false;
    add_to_args(all, argc, argv);

    all.program_name = argv[0];

    time(&all.init_time);

    new_options(all, "VW options")
    ("random_seed", po::value<uint64_t>(&(all.random_seed)), "seed random number generator")
    ("ring_size", po::value<size_t>(&(all.p->ring_size)), "size of example ring");
    add_options(all);

    new_options(all, "Update options")
    ("learning_rate,l", po::value<float>(&(all.eta)), "Set learning rate")
    ("power_t", po::value<float>(&(all.power_t)), "t power value")
    ("decay_learning_rate", po::value<float>(&(all.eta_decay_rate)),
     "Set Decay factor for learning_rate between passes")
    ("initial_t", po::value<double>(&((all.sd->t))), "initial t value")
    ("feature_mask", po::value< string >(), "Use existing regressor to determine which parameters may be updated.  If no initial_regressor given, also used for initial weights.");
    add_options(all);

    new_options(all, "Weight options")
    ("initial_regressor,i", po::value< vector<string> >(), "Initial regressor(s)")
    ("initial_weight", po::value<float>(&(all.initial_weight)), "Set all weights to an initial value of arg.")
    ("random_weights", po::value<bool>(&(all.random_weights)), "make initial weights random")
    ("sparse_weights", "Use a sparse datastructure for weights")
    ("input_feature_regularizer", po::value< string >(&(all.per_feature_regularizer_input)), "Per feature regularization input file");
    add_options(all);
 
    po::variables_map& vm = all.vm;
    if (vm.count("sparse_weights"))
      all.weights.sparse = true;
    else
      all.weights.sparse = false;
    
    new_options(all, "Parallelization options")
    ("span_server", po::value<string>(), "Location of server for setting up spanning tree")
    ("threads", "Enable multi-threading")
    ("unique_id", po::value<size_t>()->default_value(0), "unique id used for cluster parallel jobs")
    ("total", po::value<size_t>()->default_value(1), "total number of nodes used in cluster parallel job")
    ("node", po::value<size_t>()->default_value(0), "node number in cluster parallel job");
    add_options(all);

    if (vm.count("span_server"))
    { all.all_reduce_type = AllReduceType::Socket;
      all.all_reduce = new AllReduceSockets(
        vm["span_server"].as<string>(),
        vm["unique_id"].as<size_t>(),
        vm["total"].as<size_t>(),
        vm["node"].as<size_t>());
    }

    all.random_state = all.random_seed;
    parse_diagnostics(all, argc);

    //    all.sd->weighted_unlabeled_examples = all.sd->t;
    all.initial_t = (float)all.sd->t;

    return all;
  }
  catch (...)
  { VW::finish(all);
    throw;
  }
}

bool check_interaction_settings_collision(vw& all)
{ bool args_has_inter = std::find(all.args.begin(), all.args.end(), std::string("-q")) != all.args.end();
  args_has_inter = args_has_inter || ( std::find(all.args.begin(), all.args.end(), std::string("--quadratic")) != all.args.end() );
  args_has_inter = args_has_inter || ( std::find(all.args.begin(), all.args.end(), std::string("--cubic")) != all.args.end() );
  args_has_inter = args_has_inter || ( std::find(all.args.begin(), all.args.end(), std::string("--interactions")) != all.args.end() );

  if (!args_has_inter) return false;

  // we don't use -q to save pairs in all.file_options, so only 3 options checked
  bool opts_has_inter = all.file_options->str().find("--quadratic") != std::string::npos;
  opts_has_inter = opts_has_inter || (all.file_options->str().find("--cubic") != std::string::npos);
  opts_has_inter = opts_has_inter || (all.file_options->str().find("--interactions") != std::string::npos);

  return opts_has_inter;
}

void parse_modules(vw& all, io_buf& model)
{ save_load_header(all, model, true, false);

  interactions_settings_doubled = check_interaction_settings_collision(all);

  int temp_argc = 0;
  char** temp_argv = VW::get_argv_from_string(all.file_options->str(), temp_argc);

  if (interactions_settings_doubled)
  { //remove
    const char* interaction_params[] = {"--quadratic", "--cubic", "--interactions"};
    add_to_args(all, temp_argc, temp_argv, 3, interaction_params);
  }
  else
    add_to_args(all, temp_argc, temp_argv);
  for (int i = 0; i < temp_argc; i++)
    free(temp_argv[i]);
  free(temp_argv);

  po::parsed_options pos = po::command_line_parser(all.args).
                           style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
                           options(all.opts).allow_unregistered().run();

  po::variables_map& vm = all.vm;
  vm = po::variables_map();

  po::store(pos, vm);
  po::notify(vm);
  all.file_options->str("");

  parse_feature_tweaks(all); //feature tweaks

  parse_example_tweaks(all); //example manipulation

  parse_output_model(all);

  parse_output_preds(all);

  parse_reductions(all);

  if (!all.quiet)
  { all.trace_message << "Num weight bits = " << all.num_bits << endl;
    all.trace_message << "learning rate = " << all.eta << endl;
    all.trace_message << "initial_t = " << all.sd->t << endl;
    all.trace_message << "power_t = " << all.power_t << endl;
    if (all.numpasses > 1)
      all.trace_message << "decay_learning_rate = " << all.eta_decay_rate << endl;
  }
}

void parse_sources(vw& all, io_buf& model, bool skipModelLoad)
{ if (!skipModelLoad)
    load_input_model(all, model);

  parse_source(all);

  enable_sources(all, all.quiet, all.numpasses);

  // force wpp to be a power of 2 to avoid 32-bit overflow
  uint32_t i = 0;
  size_t params_per_problem = all.l->increment;
  while (params_per_problem > (uint32_t)(1 << i))
    i++;
  all.wpp = (1 << i) >> all.weights.stride_shift();
  
  if (all.vm.count("help"))
  { /* upon direct query for help -- spit it out to stdout */
    cout << "\n" << all.opts << "\n";
    exit(0);
  }
}


namespace VW
{
void cmd_string_replace_value( std::stringstream*& ss, string flag_to_replace, string new_value )
{ flag_to_replace.append(" "); //add a space to make sure we obtain the right flag in case 2 flags start with the same set of characters
  string cmd = ss->str();
  size_t pos = cmd.find(flag_to_replace);
  if( pos == string::npos )
    //flag currently not present in command string, so just append it to command string
    *ss << " " << flag_to_replace << new_value;
  else
  { //flag is present, need to replace old value with new value

    //compute position after flag_to_replace
    pos += flag_to_replace.size();

    //now pos is position where value starts
    //find position of next space
    size_t pos_after_value = cmd.find(" ",pos);
    if(pos_after_value == string::npos)
      //we reach the end of the string, so replace the all characters after pos by new_value
      cmd.replace(pos,cmd.size()-pos,new_value);
    else
      //replace characters between pos and pos_after_value by new_value
      cmd.replace(pos,pos_after_value-pos,new_value);
    ss->str(cmd);
  }
}

char** get_argv_from_string(string s, int& argc)
{ char* c = calloc_or_throw<char>(s.length()+3);
  c[0] = 'b';
  c[1] = ' ';
  strcpy(c+2, s.c_str());
  substring ss = {c, c+s.length()+2};
  v_array<substring> foo = v_init<substring>();
  tokenize(' ', ss, foo);

  char** argv = calloc_or_throw<char*>(foo.size());
  for (size_t i = 0; i < foo.size(); i++)
  { *(foo[i].end) = '\0';
    argv[i] = calloc_or_throw<char>(foo[i].end-foo[i].begin+1);
    sprintf(argv[i],"%s",foo[i].begin);
  }

  argc = (int)foo.size();
  free(c);
  foo.delete_v();
  return argv;
}

void free_args(int argc, char* argv[])
{ for (int i = 0; i < argc; i++)
    free(argv[i]);
  free(argv);
}

vw* initialize(string s, io_buf* model, bool skipModelLoad, trace_message_t trace_listener, void* trace_context)
{ int argc = 0;
  char** argv = get_argv_from_string(s,argc);
  vw* ret = nullptr;

  try
  { ret = initialize(argc, argv, model, skipModelLoad, trace_listener, trace_context); }
  catch(...)
  { free_args(argc, argv);
    throw;
  }

  free_args(argc, argv);
  return ret;
}

vw* initialize(int argc, char* argv[], io_buf* model, bool skipModelLoad, trace_message_t trace_listener, void* trace_context)
{ vw& all = parse_args(argc, argv, trace_listener, trace_context);

  try
  { // if user doesn't pass in a model, read from arguments
    io_buf localModel;
    if (!model)
    { parse_regressor_args(all, localModel);
      model = &localModel;
    }

    parse_modules(all, *model);
    parse_sources(all, *model, skipModelLoad);

    initialize_parser_datastructures(all);

    all.l->init_driver();

    return &all;
  }
  catch (std::exception& e)
  { all.trace_message << "Error: " << e.what() << endl;
    finish(all);
    throw;
  }
  catch (...)
  { finish(all);
    throw;
  }
}

// Create a new VW instance while sharing the model with another instance
// The extra arguments will be appended to those of the other VW instance
vw* seed_vw_model(vw* vw_model, const string extra_args, trace_message_t trace_listener, void* trace_context)
{ vector<string> model_args = vw_model->args;
  model_args.push_back(extra_args);

  std::ostringstream init_args;
  for (size_t i = 0; i < model_args.size(); i++)
  { if (model_args[i] == "--no_stdin" || // ignore this since it will be added by vw::initialize
        model_args[i] == "-i" || // ignore -i since we don't want to reload the model
        (i > 0 && model_args[i - 1] == "-i"))
    { continue;
    }
    init_args << model_args[i] << " ";
  }

  vw* new_model = VW::initialize(init_args.str().c_str(), nullptr, true /* skipModelLoad */, trace_listener, trace_context);
  free_it(new_model->sd);

  // reference model states stored in the specified VW instance
  new_model->weights.shallow_copy(vw_model->weights); // regressor
  new_model->sd = vw_model->sd; // shared data

  return new_model;
}

void delete_dictionary_entry(substring ss, features* A)
{ free(ss.begin);
  A->delete_v();
  delete A;
}

void sync_stats(vw& all)
{ if (all.all_reduce != nullptr)
  { float loss = (float)all.sd->sum_loss;
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
{ // also update VowpalWabbit::PerformanceStatistics::get() (vowpalwabbit.cpp)
  if (!all.quiet && !all.vm.count("audit_regressor"))
  { all.trace_message.precision(6);
	all.trace_message << std::fixed;
    all.trace_message << endl << "finished run";
    if(all.current_pass == 0)
      all.trace_message << endl << "number of examples = " << all.sd->example_number;
    else
    { all.trace_message << endl << "number of examples per pass = " << all.sd->example_number / all.current_pass;
      all.trace_message << endl << "passes used = " << all.current_pass;
    }
    all.trace_message << endl << "weighted example sum = " << all.sd->weighted_examples();
    all.trace_message << endl << "weighted label sum = " << all.sd->weighted_labels;
    all.trace_message << endl << "average loss = ";
    if(all.holdout_set_off)
      if (all.sd->weighted_labeled_examples > 0)
	all.trace_message << all.sd->sum_loss / all.sd->weighted_labeled_examples;
      else
	all.trace_message << "n.a.";
    else if  ((all.sd->holdout_best_loss == FLT_MAX) || (all.sd->holdout_best_loss == FLT_MAX * 0.5))
      all.trace_message << "undefined (no holdout)";
    else
      all.trace_message << all.sd->holdout_best_loss << " h";
    if (all.sd->report_multiclass_log_loss)
    { if (all.holdout_set_off)
        all.trace_message << endl << "average multiclass log loss = " << all.sd->multiclass_log_loss / all.sd->weighted_labeled_examples;
      else
        all.trace_message << endl << "average multiclass log loss = " << all.sd->holdout_multiclass_log_loss / all.sd->weighted_labeled_examples << " h";
    }

    float best_constant; float best_constant_loss;
    if (get_best_constant(all, best_constant, best_constant_loss))
    { all.trace_message << endl << "best constant = " << best_constant;
      if (best_constant_loss != FLT_MIN)
        all.trace_message << endl << "best constant's loss = " << best_constant_loss;
    }

    all.trace_message << endl << "total feature number = " << all.sd->total_features;
    if (all.sd->queries > 0)
      all.trace_message << endl << "total queries = " << all.sd->queries << endl;
    all.trace_message << endl;
  }

  // implement finally.
  // finalize_regressor can throw if it can't write the file.
  // we still want to free up all the memory.
  vw_exception finalize_regressor_exception(__FILE__, __LINE__, "empty");
  bool finalize_regressor_exception_thrown = false;
  try
  { finalize_regressor(all, all.final_regressor_name);
  }
  catch (vw_exception& e)
  { finalize_regressor_exception = e;
    finalize_regressor_exception_thrown = true;
  }

  if (all.l != nullptr)
  { all.l->finish();
    free_it(all.l);
  }

  free_parser(all);
  finalize_source(all.p);
  all.p->parse_name.erase();
  all.p->parse_name.delete_v();
  free(all.p);
  bool seeded;
  if (all.weights.seeded() > 0)
	  seeded = true;
  else
	  seeded = false;
  if (!seeded)
  { delete(all.sd->ldict);
    free(all.sd);
  }
  all.reduction_stack.delete_v();
  delete all.file_options;
  for (size_t i = 0; i < all.final_prediction_sink.size(); i++)
    if (all.final_prediction_sink[i] != 1)
      io_buf::close_file_or_socket(all.final_prediction_sink[i]);
  all.final_prediction_sink.delete_v();
  for (size_t i=0; i<all.loaded_dictionaries.size(); i++)
  { free(all.loaded_dictionaries[i].name);
    all.loaded_dictionaries[i].dict->iter(delete_dictionary_entry);
    all.loaded_dictionaries[i].dict->delete_v();
    delete all.loaded_dictionaries[i].dict;
  }
  delete all.loss;

  delete all.all_reduce;

  // destroy all interactions and array of them
  for (v_string& i : all.interactions) i.delete_v();
  all.interactions.delete_v();

  if (delete_all) delete &all;

  if (finalize_regressor_exception_thrown)
    throw finalize_regressor_exception;
}
}
