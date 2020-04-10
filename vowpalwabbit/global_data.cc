// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cstdio>
#include <cfloat>
#include <cerrno>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cassert>

#include "global_data.h"
#include "gd.h"
#include "vw_exception.h"
#include "future_compat.h"
#include "vw_allreduce.h"
#include "parse_args.h"
#include "vw.h"

#include "options_boost_po.h"
#include "options_serializer_boost_po.h"

#ifdef _WIN32
#define NOMINMAX
#include <WinSock2.h>
#include <Windows.h>
#else
#include <sys/socket.h>
#endif

struct global_prediction
{
  float p;
  float weight;
};

size_t really_read(int sock, void* in, size_t count)
{
  char* buf = (char*)in;
  size_t done = 0;
  int r = 0;
  while (done < count)
  {
    if ((r =
#ifdef _WIN32
                recv(sock, buf, (unsigned int)(count - done), 0)
#else
                read(sock, buf, (unsigned int)(count - done))
#endif
                ) == 0)
      return 0;
    else if (r < 0)
    {
      THROWERRNO("read(" << sock << "," << count << "-" << done << ")");
    }
    else
    {
      done += r;
      buf += r;
    }
  }
  return done;
}

void get_prediction(int sock, float& res, float& weight)
{
  global_prediction p;
  really_read(sock, &p, sizeof(p));
  res = p.p;
  weight = p.weight;
}

void send_prediction(int sock, global_prediction p)
{
  if (
#ifdef _WIN32
      send(sock, reinterpret_cast<const char*>(&p), sizeof(p), 0)
#else
      write(sock, &p, sizeof(p))
#endif
      < (int)sizeof(p))
    THROWERRNO("send_prediction write(" << sock << ")");
}

void binary_print_result(int f, float res, float weight, v_array<char> array)
{
  binary_print_result_by_ref(f, res, weight, array);
}

void binary_print_result_by_ref(int f, float res, float weight, const v_array<char>&)
{
  if (f >= 0)
  {
    global_prediction ps = {res, weight};
    send_prediction(f, ps);
  }
}

int print_tag_by_ref(std::stringstream& ss, const v_array<char>& tag)
{
  if (tag.begin() != tag.end())
  {
    ss << ' ';
    ss.write(tag.begin(), sizeof(char) * tag.size());
  }
  return tag.begin() != tag.end();
}

int print_tag(std::stringstream& ss, v_array<char> tag)
{
  return print_tag_by_ref(ss, tag);
}

void print_result(int f, float res, float unused, v_array<char> tag)
{
  print_result_by_ref(f, res, unused, tag);
}

void print_result_by_ref(int f, float res, float, const v_array<char>& tag)
{
  if (f >= 0)
  {
    std::stringstream ss;
    auto saved_precision = ss.precision();
    if (floorf(res) == res)
      ss << std::setprecision(0);
    ss << std::fixed << res << std::setprecision(saved_precision);
    print_tag_by_ref(ss, tag);
    ss << '\n';
    ssize_t len = ss.str().size();
    ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
    if (t != len)
    {
      std::cerr << "write error: " << strerror(errno) << std::endl;
    }
  }
}

void print_raw_text(int f, std::string s, v_array<char> tag)
{
  if (f < 0)
    return;

  std::stringstream ss;
  ss << s;
  print_tag_by_ref(ss, tag);
  ss << '\n';
  ssize_t len = ss.str().size();
  ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
  if (t != len)
  {
    std::cerr << "write error: " << strerror(errno) << std::endl;
  }
}


void print_raw_text_by_ref(int f, const std::string& s, const v_array<char>& tag)
{
  if (f < 0)
    return;

  std::stringstream ss;
  ss << s;
  print_tag_by_ref(ss, tag);
  ss << '\n';
  ssize_t len = ss.str().size();
  ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
  if (t != len)
  {
    std::cerr << "write error: " << strerror(errno) << std::endl;
  }
}


void set_mm(shared_data* sd, float label)
{
  sd->min_label = std::min(sd->min_label, label);
  if (label != FLT_MAX)
    sd->max_label = std::max(sd->max_label, label);
}

void noop_mm(shared_data*, float) {}

void vw::learn(example& ec)
{
  if (l->is_multiline)
    THROW("This reduction does not support single-line examples.");

  if (ec.test_only || !training)
    VW::LEARNER::as_singleline(l)->predict(ec);
  else
    VW::LEARNER::as_singleline(l)->learn(ec);
}

void vw::learn(multi_ex& ec)
{
  if (!l->is_multiline)
    THROW("This reduction does not support multi-line example.");

  if (!training)
    VW::LEARNER::as_multiline(l)->predict(ec);
  else
    VW::LEARNER::as_multiline(l)->learn(ec);
}

void vw::predict(example& ec)
{
  if (l->is_multiline)
    THROW("This reduction does not support single-line examples.");

  // be called directly in library mode, test_only must be explicitly set here. If the example has a label but is passed
  // to predict it would otherwise be incorrectly labelled as test_only = false.
  ec.test_only = true;

  VW::LEARNER::as_singleline(l)->predict(ec);
}

void vw::predict(multi_ex& ec)
{
  if (!l->is_multiline)
    THROW("This reduction does not support multi-line example.");

  // be called directly in library mode, test_only must be explicitly set here. If the example has a label but is passed
  // to predict it would otherwise be incorrectly labelled as test_only = false.
  for (auto& ex : ec)
  {
    ex->test_only = true;
  }

  VW::LEARNER::as_multiline(l)->predict(ec);
}

void vw::finish_example(example& ec)
{
  if (l->is_multiline)
    THROW("This reduction does not support single-line examples.");

  VW::LEARNER::as_singleline(l)->finish_example(*this, ec);
}

void vw::finish_example(multi_ex& ec)
{
  if (!l->is_multiline)
    THROW("This reduction does not support multi-line example.");

  VW::LEARNER::as_multiline(l)->finish_example(*this, ec);
}

void compile_gram(
    std::vector<std::string> grams, std::array<uint32_t, NUM_NAMESPACES>& dest, char* descriptor, bool quiet)
{
  for (size_t i = 0; i < grams.size(); i++)
  {
    std::string ngram = grams[i];
    if (isdigit(ngram[0]))
    {
      int n = atoi(ngram.c_str());
      if (!quiet)
        std::cerr << "Generating " << n << "-" << descriptor << " for all namespaces." << std::endl;
      for (size_t j = 0; j < 256; j++) dest[j] = n;
    }
    else if (ngram.size() == 1)
      std::cout << "You must specify the namespace index before the n" << std::endl;
    else
    {
      int n = atoi(ngram.c_str() + 1);
      dest[(uint32_t)(unsigned char)*ngram.c_str()] = n;
      if (!quiet)
        std::cerr << "Generating " << n << "-" << descriptor << " for " << ngram[0] << " namespaces." << std::endl;
    }
  }
}

void compile_limits(std::vector<std::string> limits, std::array<uint32_t, NUM_NAMESPACES>& dest, bool quiet)
{
  for (size_t i = 0; i < limits.size(); i++)
  {
    std::string limit = limits[i];
    if (isdigit(limit[0]))
    {
      int n = atoi(limit.c_str());
      if (!quiet)
        std::cerr << "limiting to " << n << "features for each namespace." << std::endl;
      for (size_t j = 0; j < 256; j++) dest[j] = n;
    }
    else if (limit.size() == 1)
      std::cout << "You must specify the namespace index before the n" << std::endl;
    else
    {
      int n = atoi(limit.c_str() + 1);
      dest[(uint32_t)limit[0]] = n;
      if (!quiet)
        std::cerr << "limiting to " << n << " for namespaces " << limit[0] << std::endl;
    }
  }
}

void trace_listener_cerr(void*, const std::string& message)
{
  std::cerr << message;
  std::cerr.flush();
}

int vw_ostream::vw_streambuf::sync()
{
  int ret = std::stringbuf::sync();
  if (ret)
    return ret;

  parent.trace_listener(parent.trace_context, str());
  str("");
  return 0;  // success
}

vw_ostream::vw_ostream() : std::ostream(&buf), buf(*this), trace_context(nullptr)
{
  trace_listener = trace_listener_cerr;
}

IGNORE_DEPRECATED_USAGE_START
vw::vw()
{
  sd = &calloc_or_throw<shared_data>();
  sd->dump_interval = 1.;  // next update progress dump
  sd->contraction = 1.;
  sd->first_observed_label = FLT_MAX;
  sd->is_more_than_two_labels_observed = false;
  sd->max_label = 0;
  sd->min_label = 0;

  is_managed_by_unique = true;
  label_type = label_type_t::simple;

  l = nullptr;
  scorer = nullptr;
  cost_sensitive = nullptr;
  loss = nullptr;
  p = nullptr;

  reg_mode = 0;
  current_pass = 0;

  data_filename = "";
  delete_prediction = nullptr;

  bfgs = false;
  no_bias = false;
  hessian_on = false;
  active = false;
  num_bits = 18;
  default_bits = true;
  daemon = false;
  num_children = 10;
  save_resume = false;
  preserve_performance_counters = false;

  random_positive_weights = false;

  weights.sparse = false;

  set_minmax = set_mm;

  power_t = 0.5;
  eta = 0.5;  // default learning rate for normalized adaptive updates, this is switched to 10 by default for the other
              // updates (see parse_args.cc)
  numpasses = 1;

  final_prediction_sink.begin() = final_prediction_sink.end() = final_prediction_sink.end_array = nullptr;
  raw_prediction = -1;

  print = print_result;
  print_text = print_raw_text;
  print_by_ref = print_result_by_ref;
  print_text_by_ref = print_raw_text_by_ref;
  lda = 0;
  random_seed = 0;
  random_weights = false;
  normal_weights = false;
  tnormal_weights = false;
  per_feature_regularizer_input = "";
  per_feature_regularizer_output = "";
  per_feature_regularizer_text = "";

#ifdef _WIN32
  stdout_fileno = _fileno(stdout);
#else
  stdout_fileno = fileno(stdout);
#endif

  searchstr = nullptr;

  nonormalize = false;
  l1_lambda = 0.0;
  l2_lambda = 0.0;

  eta_decay_rate = 1.0;
  initial_weight = 0.0;
  initial_constant = 0.0;

  all_reduce = nullptr;

  for (size_t i = 0; i < 256; i++)
  {
    ngram[i] = 0;
    skips[i] = 0;
    limit[i] = INT_MAX;
    affix_features[i] = 0;
    spelling_features[i] = 0;
  }

  invariant_updates = true;
  normalized_idx = 2;

  add_constant = true;
  audit = false;

  pass_length = std::numeric_limits<size_t>::max();
  passes_complete = 0;

  save_per_pass = false;

  stdin_off = false;
  do_reset_source = false;
  holdout_set_off = true;
  holdout_after = 0;
  check_holdout_every_n_passes = 1;
  early_terminate = false;

  max_examples = std::numeric_limits<size_t>::max();

  hash_inv = false;
  print_invert = false;

  // Set by the '--progress <arg>' option and affect sd->dump_interval
  progress_add = false;  // default is multiplicative progress dumps
  progress_arg = 2.0;    // next update progress dump multiplier

  sd->is_more_than_two_labels_observed = false;
  sd->first_observed_label = FLT_MAX;
  sd->second_observed_label = FLT_MAX;

  sd->report_multiclass_log_loss = false;
  sd->multiclass_log_loss = 0;
  sd->holdout_multiclass_log_loss = 0;
}
IGNORE_DEPRECATED_USAGE_END

vw::~vw()
{
  if (l != nullptr)
  {
    l->finish();
    free(l);
  }

  // Check if options object lifetime is managed internally.
  if (should_delete_options)
    delete options;

  // TODO: migrate all finalization into parser destructor
  if (p != nullptr)
  {
    free_parser(*this);
    finalize_source(p);
    p->parse_name.clear();
    p->parse_name.delete_v();
    delete p;
  }

  const bool seeded = weights.seeded() > 0;
  if (!seeded)
  {
    if (sd->ldict)
    {
      sd->ldict->~namedlabels();
      free(sd->ldict);
    }
    free(sd);
  }

  for (auto& sink : final_prediction_sink)
  {
    // This is checking if the sink corresponds to stdout. TODO: abstract this.
    if (sink != 1)
    {
      io_buf::close_file_or_socket(sink);
    }
  }
  final_prediction_sink.delete_v();

  delete loss;
  delete all_reduce;
}

std::unique_ptr<vw> vw::initialize(VW::config::options_i& options, io_buf* model, bool skipModelLoad, trace_message_t trace_listener,
    void* trace_context)
{
  // Can't use make_unique here as it requires access to the constructor which is private.
  auto all = std::unique_ptr<vw>(new vw);
  parse_args(*all, options, trace_listener, trace_context);

  try
  {
    // if user doesn't pass in a model, read from options
    io_buf localModel;
    if (!model)
    {
      std::vector<std::string> all_initial_regressor_files(all->initial_regressors);
      if (options.was_supplied("input_feature_regularizer"))
      {
        all_initial_regressor_files.push_back(all->per_feature_regularizer_input);
      }
      read_regressor_file(*all, all_initial_regressor_files, localModel);
      model = &localModel;
    }

    // Loads header of model files and loads the command line options into the options object.
    load_header_merge_options(options, *all, *model);

    std::vector<std::string> dictionary_nses;
    parse_modules(options, *all, dictionary_nses);

    parse_sources(options, *all, *model, skipModelLoad);

    // we must delay so parse_mask is fully defined.
    for (size_t id = 0; id < dictionary_nses.size(); id++) parse_dictionary_argument(*all, dictionary_nses[id]);

    options.check_unregistered();

    // upon direct query for help -- spit it out to stdout;
    if (options.get_typed_option<bool>("help").value())
    {
      std::cout << options.help();
      exit(0);
    }

    all->l->init_driver();

    return std::move(all);
  }
  catch (std::exception& e)
  {
    all->trace_message << "Error: " << e.what() << std::endl;
    VW::finish(std::move(all));
    throw;
  }
  catch (...)
  {
    VW::finish(std::move(all));
    throw;
  }
}

std::unique_ptr<vw> vw::initialize(
    const std::string& s, io_buf* model, bool skipModelLoad, trace_message_t trace_listener, void* trace_context)
{
  int argc = 0;
  char** argv = VW::to_argv(s, argc);
  std::unique_ptr<vw> ret = nullptr;

  try
  {
    ret = initialize(argc, argv, model, skipModelLoad, trace_listener, trace_context);
  }
  catch (...)
  {
    VW::free_args(argc, argv);
    throw;
  }

  VW::free_args(argc, argv);
  return ret;
}

std::unique_ptr<vw> vw::initialize(
    int argc, char* argv[], io_buf* model, bool skipModelLoad, trace_message_t trace_listener, void* trace_context)
{
  VW::config::options_i* options = new VW::config::options_boost_po(argc, argv);
  auto all = initialize(*options, model, skipModelLoad, trace_listener, trace_context);

  // When VW is deleted the options object will be cleaned up too.
  all->should_delete_options = true;
  return all;
}

std::unique_ptr<vw> vw::seed_vw_model(vw* vw_model, const std::string& extra_args, trace_message_t trace_listener, void* trace_context)
{
  VW::config::options_serializer_boost_po serializer;
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

  auto new_model =
      vw::initialize(serialized_options.c_str(), nullptr, true /* skipModelLoad */, trace_listener, trace_context);
  free_it(new_model->sd);

  // reference model states stored in the specified VW instance
  new_model->weights.shallow_copy(vw_model->weights);  // regressor
  new_model->sd = vw_model->sd;                        // shared data
  new_model->p->_shared_data = new_model->sd;

  return new_model;
}

// Allows the input command line string to have spaces escaped by '\'
std::unique_ptr<vw> vw::initialize_escaped(
    const std::string& s, io_buf* model, bool skipModelLoad, trace_message_t trace_listener, void* trace_context)
{
  int argc = 0;
  char** argv = VW::to_argv_escaped(s, argc);
  std::unique_ptr<vw> ret = nullptr;

  try
  {
    ret = initialize(argc, argv, model, skipModelLoad, trace_listener, trace_context);
  }
  catch (...)
  {
    VW::free_args(argc, argv);
    throw;
  }

  VW::free_args(argc, argv);
  return ret;
}
