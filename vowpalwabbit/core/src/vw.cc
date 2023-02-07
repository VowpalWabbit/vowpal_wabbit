// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/vw.h"

#include "vw/config/cli_help_formatter.h"
#include "vw/config/cli_options_serializer.h"
#include "vw/config/options_cli.h"
#include "vw/core/accumulate.h"
#include "vw/core/crossplat_compat.h"
#include "vw/core/kskip_ngram_transformer.h"
#include "vw/core/learner.h"
#include "vw/core/memory.h"
#include "vw/core/parse_args.h"
#include "vw/core/parse_dispatch_loop.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/reductions/conditional_contextual_bandit.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/reductions/metrics.h"
#include "vw/core/shared_data.h"
#include "vw/core/unique_sort.h"
#include "vw/text_parser/parse_example_text.h"

#include <iostream>

namespace
{

std::unique_ptr<VW::workspace> initialize_internal(
    std::unique_ptr<VW::config::options_i, VW::options_deleter_type> options, VW::io_buf* model, bool skip_model_load,
    VW::trace_message_t trace_listener, void* trace_context, VW::io::logger* custom_logger,
    std::unique_ptr<VW::setup_base_i> setup_base = nullptr)
{
  // Set up logger as early as possible
  auto all = VW::details::parse_args(std::move(options), trace_listener, trace_context, custom_logger);

  // if user doesn't pass in a model, read from options
  VW::io_buf local_model;
  if (model == nullptr)
  {
    std::vector<std::string> all_initial_regressor_files(all->initial_regressors);
    if (all->options->was_supplied("input_feature_regularizer"))
    {
      all_initial_regressor_files.push_back(all->per_feature_regularizer_input);
    }
    VW::details::read_regressor_file(*all, all_initial_regressor_files, local_model);
    model = &local_model;
  }

  std::vector<std::string> dictionary_namespaces;
  try
  {
    // Loads header of model files and loads the command line options into the options object.
    bool interactions_settings_duplicated{};
    VW::details::load_header_merge_options(*all->options, *all, *model, interactions_settings_duplicated);

    VW::details::parse_modules(*all->options, *all, interactions_settings_duplicated, dictionary_namespaces);
    VW::details::instantiate_learner(*all, std::move(setup_base));
    VW::details::parse_sources(*all->options, *all, *model, skip_model_load);
  }
  catch (VW::save_load_model_exception& e)
  {
    auto msg = fmt::format("{}, model files = {}", e.what(), fmt::join(all->initial_regressors, ", "));
    throw VW::save_load_model_exception(e.filename(), e.line_number(), msg);
  }

  if (!all->quiet)
  {
    *(all->trace_message) << "Num weight bits = " << all->num_bits << std::endl;
    *(all->trace_message) << "learning rate = " << all->eta << std::endl;
    *(all->trace_message) << "initial_t = " << all->sd->t << std::endl;
    *(all->trace_message) << "power_t = " << all->power_t << std::endl;
    if (all->numpasses > 1) { *(all->trace_message) << "decay_learning_rate = " << all->eta_decay_rate << std::endl; }
    if (all->options->was_supplied("cb_type"))
    {
      *(all->trace_message) << "cb_type = " << all->options->get_typed_option<std::string>("cb_type").value()
                            << std::endl;
    }
  }

  // we must delay so parse_mask is fully defined.
  for (const auto& name_space : dictionary_namespaces) { VW::details::parse_dictionary_argument(*all, name_space); }

  std::vector<std::string> enabled_learners;
  if (all->l != nullptr) { all->l->get_enabled_learners(enabled_learners); }

  // upon direct query for help -- spit it out to stdout;
  if (all->options->get_typed_option<bool>("help").value())
  {
    size_t num_supplied = 0;
    for (auto const& option : all->options->get_all_options())
    {
      num_supplied += all->options->was_supplied(option->m_name) ? 1 : 0;
    }

    auto option_groups = all->options->get_all_option_group_definitions();
    std::sort(option_groups.begin(), option_groups.end(),
        [](const VW::config::option_group_definition& a, const VW::config::option_group_definition& b)
        { return a.m_name < b.m_name; });
    // Help is added as help and h. So greater than 2 means there is more command line there.
    if (num_supplied > 2) { option_groups = remove_disabled_necessary_options(*all->options, option_groups); }

    VW::config::cli_help_formatter formatter;
    std::cout << formatter.format_help(option_groups);
    std::exit(0);
  }

  if (all->options->was_supplied("automl") && all->options->was_supplied("aml_predict_only_model"))
  {
    std::string automl_predict_only_filename =
        all->options->get_typed_option<std::string>("aml_predict_only_model").value();
    if (automl_predict_only_filename.empty())
    {
      THROW("error: --aml_predict_only_model has to be non-zero string representing filename to write");
    }

    VW::details::finalize_regressor(*all, automl_predict_only_filename);
    std::exit(0);
  }

  VW::details::print_enabled_learners(*all, enabled_learners);

  if (!all->quiet)
  {
    *(all->trace_message) << "Input label = " << VW::to_string(all->l->get_input_label_type()).substr(14) << std::endl;
    *(all->trace_message) << "Output pred = " << VW::to_string(all->l->get_output_prediction_type()).substr(19)
                          << std::endl;
  }

  if (!all->options->get_typed_option<bool>("dry_run").value())
  {
    if (!all->quiet && !all->bfgs && (all->searchstr == nullptr) && !all->options->was_supplied("audit_regressor"))
    {
      all->sd->print_update_header(*all->trace_message);
    }
    all->l->init_driver();
  }

  return all;
}
VW::workspace* initialize_with_builder(std::unique_ptr<VW::config::options_i, VW::options_deleter_type> options,
    VW::io_buf* model, bool skip_model_load, VW::trace_message_t trace_listener, void* trace_context,

    std::unique_ptr<VW::setup_base_i> setup_base = nullptr)
{
  return initialize_internal(
      std::move(options), model, skip_model_load, trace_listener, trace_context, nullptr, std::move(setup_base))
      .release();
}

VW::workspace* initialize_with_builder(int argc, char* argv[], VW::io_buf* model, bool skip_model_load,
    VW::trace_message_t trace_listener, void* trace_context, std::unique_ptr<VW::setup_base_i> setup_base)
{
  std::unique_ptr<VW::config::options_i, VW::options_deleter_type> options(
      new VW::config::options_cli(std::vector<std::string>(argv + 1, argv + argc)),
      [](VW::config::options_i* ptr) { delete ptr; });
  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  return initialize_with_builder(
      std::move(options), model, skip_model_load, trace_listener, trace_context, std::move(setup_base));
  VW_WARNING_STATE_POP
}
}  // namespace

VW::workspace* VW::initialize(std::unique_ptr<config::options_i, options_deleter_type> options, io_buf* model,
    bool skip_model_load, VW::trace_message_t trace_listener, void* trace_context)
{
  return ::initialize_with_builder(std::move(options), model, skip_model_load, trace_listener, trace_context, nullptr);
}

VW::workspace* VW::initialize(config::options_i& options, io_buf* model, bool skip_model_load,
    VW::trace_message_t trace_listener, void* trace_context)
{
  std::unique_ptr<config::options_i, options_deleter_type> opts(&options, [](VW::config::options_i*) {});
  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  return initialize(std::move(opts), model, skip_model_load, trace_listener, trace_context);
  VW_WARNING_STATE_POP
}
VW::workspace* VW::initialize(
    const std::string& s, io_buf* model, bool skip_model_load, VW::trace_message_t trace_listener, void* trace_context)
{
  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  return initialize_with_builder(s, model, skip_model_load, trace_listener, trace_context, nullptr);
  VW_WARNING_STATE_POP
}
VW::workspace* VW::initialize(int argc, char* argv[], io_buf* model, bool skip_model_load,
    VW::trace_message_t trace_listener, void* trace_context)
{
  return ::initialize_with_builder(argc, argv, model, skip_model_load, trace_listener, trace_context, nullptr);
}

// Create a new VW instance while sharing the model with another instance
// The extra arguments will be appended to those of the other VW instance
VW::workspace* VW::seed_vw_model(
    VW::workspace* vw_model, const std::string& extra_args, VW::trace_message_t trace_listener, void* trace_context)
{
  config::cli_options_serializer serializer;
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

  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  VW::workspace* new_model =
      VW::initialize(serialized_options, nullptr, true /* skip_model_load */, trace_listener, trace_context);
  VW_WARNING_STATE_POP

  // reference model states stored in the specified VW instance
  new_model->weights.shallow_copy(vw_model->weights);  // regressor
  new_model->sd = vw_model->sd;                        // shared data

  return new_model;
}

VW::workspace* VW::initialize_escaped(
    std::string const& s, io_buf* model, bool skip_model_load, VW::trace_message_t trace_listener, void* trace_context)
{
  int argc = 0;
  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  char** argv = to_argv_escaped(s, argc);
  VW_WARNING_STATE_POP

  VW::workspace* ret = nullptr;

  try
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_DEPRECATED_USAGE
    ret = initialize(argc, argv, model, skip_model_load, trace_listener, trace_context);
    VW_WARNING_STATE_POP
  }
  catch (...)
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_DEPRECATED_USAGE
    free_args(argc, argv);
    VW_WARNING_STATE_POP

    throw;
  }

  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  free_args(argc, argv);
  VW_WARNING_STATE_POP

  return ret;
}

std::unique_ptr<VW::workspace> VW::initialize_experimental(std::unique_ptr<config::options_i> options,
    std::unique_ptr<VW::io::reader> model_override_reader, driver_output_func_t driver_output_func,
    void* driver_output_func_context, VW::io::logger* custom_logger, std::unique_ptr<VW::setup_base_i> setup_base)
{
  auto* released_options = options.release();
  std::unique_ptr<config::options_i, options_deleter_type> options_custom_deleter(
      released_options, [](VW::config::options_i* ptr) { delete ptr; });

  // Skip model load should be implemented by a caller not passing model loading args.
  std::unique_ptr<io_buf> model(nullptr);
  if (model_override_reader != nullptr)
  {
    model = VW::make_unique<io_buf>();
    model->add_file(std::move(model_override_reader));
  }
  return initialize_internal(std::move(options_custom_deleter), model.get(), false /* skip model load */,
      driver_output_func, driver_output_func_context, custom_logger, std::move(setup_base));
}

std::unique_ptr<VW::workspace> VW::initialize(std::unique_ptr<config::options_i> options,
    std::unique_ptr<VW::io::reader> model_override_reader, driver_output_func_t driver_output_func,
    void* driver_output_func_context, VW::io::logger* custom_logger)
{
  auto* released_options = options.release();
  std::unique_ptr<config::options_i, options_deleter_type> options_custom_deleter(
      released_options, [](VW::config::options_i* ptr) { delete ptr; });

  // Skip model load should be implemented by a caller not passing model loading args.
  std::unique_ptr<io_buf> model(nullptr);
  if (model_override_reader != nullptr)
  {
    model = VW::make_unique<io_buf>();
    model->add_file(std::move(model_override_reader));
  }
  return initialize_internal(std::move(options_custom_deleter), model.get(), false /* skip model load */,
      driver_output_func, driver_output_func_context, custom_logger, nullptr);
}

std::unique_ptr<VW::workspace> VW::seed_vw_model(VW::workspace& vw_model, const std::vector<std::string>& extra_args,
    driver_output_func_t driver_output_func, void* driver_output_func_context, VW::io::logger* custom_logger)
{
  config::cli_options_serializer serializer;
  for (auto const& option : vw_model.options->get_all_options())
  {
    if (vw_model.options->was_supplied(option->m_name))
    {
      // ignore no_stdin since it will be added by vw::initialize, and ignore -i since we don't want to reload the
      // model.
      if (option->m_name == "no_stdin" || option->m_name == "initial_regressor") { continue; }

      serializer.add(*option);
    }
  }

  auto serialized_options = VW::split_command_line(serializer.str());
  serialized_options.insert(serialized_options.end(), extra_args.begin(), extra_args.end());

  auto options = VW::make_unique<config::options_cli>(serialized_options);

  std::unique_ptr<config::options_i, options_deleter_type> options_custom_deleter(
      new VW::config::options_cli(serialized_options), [](VW::config::options_i* ptr) { delete ptr; });

  auto new_model = initialize_internal(std::move(options_custom_deleter), nullptr, false /* skip model load */,
      driver_output_func, driver_output_func_context, custom_logger, nullptr);

  // reference model states stored in the specified VW instance
  new_model->weights.shallow_copy(vw_model.weights);  // regressor
  new_model->sd = vw_model.sd;                        // shared data

  return new_model;
}

VW::workspace* VW::initialize_with_builder(const std::string& s, io_buf* model, bool skip_model_load,
    VW::trace_message_t trace_listener, void* trace_context, std::unique_ptr<VW::setup_base_i> setup_base)
{
  int argc = 0;
  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  char** argv = to_argv(s, argc);
  VW_WARNING_STATE_POP

  VW::workspace* ret = nullptr;

  try
  {
    ret = ::initialize_with_builder(
        argc, argv, model, skip_model_load, trace_listener, trace_context, std::move(setup_base));
  }
  catch (...)
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_DEPRECATED_USAGE
    free_args(argc, argv);
    VW_WARNING_STATE_POP

    throw;
  }

  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  free_args(argc, argv);
  VW_WARNING_STATE_POP

  return ret;
}

void VW::cmd_string_replace_value(std::stringstream*& ss, std::string flag_to_replace, const std::string& new_value)
{
  flag_to_replace.append(
      " ");  // add a space to make sure we obtain the right flag in case 2 flags start with the same set of characters
  std::string cmd = ss->str();
  size_t pos = cmd.find(flag_to_replace);
  if (pos == std::string::npos)
  {
    // flag currently not present in command string, so just append it to command string
    *ss << " " << flag_to_replace << new_value;
  }
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

char** VW::to_argv_escaped(std::string const& s, int& argc)
{
  std::vector<std::string> tokens = VW::details::escaped_tokenize(' ', s);
  char** argv = VW::details::calloc_or_throw<char*>(tokens.size() + 1);
  argv[0] = VW::details::calloc_or_throw<char>(2);
  argv[0][0] = 'b';
  argv[0][1] = '\0';

  for (size_t i = 0; i < tokens.size(); i++)
  {
    argv[i + 1] = VW::details::calloc_or_throw<char>(tokens[i].length() + 1);
    sprintf_s(argv[i + 1], (tokens[i].length() + 1), "%s", tokens[i].data());
  }

  argc = static_cast<int>(tokens.size() + 1);
  return argv;
}

char** VW::to_argv(std::string const& s, int& argc)
{
  const VW::string_view strview(s);
  std::vector<VW::string_view> foo;
  VW::tokenize(' ', strview, foo);

  char** argv = VW::details::calloc_or_throw<char*>(foo.size() + 1);
  // small optimization to avoid a string copy before tokenizing
  argv[0] = VW::details::calloc_or_throw<char>(2);
  argv[0][0] = 'b';
  argv[0][1] = '\0';
  for (size_t i = 0; i < foo.size(); i++)
  {
    const size_t len = foo[i].length();
    argv[i + 1] = VW::details::calloc_or_throw<char>(len + 1);
    memcpy(argv[i + 1], foo[i].data(), len);
    // copy() is supported with boost::string_view, not with string_ref
    // foo[i].copy(argv[i], len);
    // unnecessary because of the calloc, but needed if we change stuff in the future
    // argv[i][len] = '\0';
  }

  argc = static_cast<int>(foo.size()) + 1;
  return argv;
}

void VW::free_args(int argc, char* argv[])
{
  for (int i = 0; i < argc; i++) { free(argv[i]); }
  free(argv);
}

const char* VW::are_features_compatible(const VW::workspace& vw1, const VW::workspace& vw2)
{
  if (vw1.example_parser->hasher != vw2.example_parser->hasher) { return "hasher"; }

  if (!std::equal(vw1.spelling_features.begin(), vw1.spelling_features.end(), vw2.spelling_features.begin()))
  {
    return "spelling_features";
  }

  if (!std::equal(vw1.affix_features.begin(), vw1.affix_features.end(), vw2.affix_features.begin()))
  {
    return "affix_features";
  }

  if (vw1.skip_gram_transformer != nullptr && vw2.skip_gram_transformer != nullptr)
  {
    const auto& vw1_ngram_strings = vw1.skip_gram_transformer->get_initial_ngram_definitions();
    const auto& vw2_ngram_strings = vw2.skip_gram_transformer->get_initial_ngram_definitions();
    const auto& vw1_skips_strings = vw1.skip_gram_transformer->get_initial_skip_definitions();
    const auto& vw2_skips_strings = vw2.skip_gram_transformer->get_initial_skip_definitions();

    if (!std::equal(vw1_ngram_strings.begin(), vw1_ngram_strings.end(), vw2_ngram_strings.begin())) { return "ngram"; }

    if (!std::equal(vw1_skips_strings.begin(), vw1_skips_strings.end(), vw2_skips_strings.begin())) { return "skips"; }
  }
  else if (vw1.skip_gram_transformer != nullptr || vw2.skip_gram_transformer != nullptr)
  {
    // If one of them didn't define the ngram transformer then they differ by ngram (skips depends on ngram)
    return "ngram";
  }

  if (!std::equal(vw1.limit.begin(), vw1.limit.end(), vw2.limit.begin())) { return "limit"; }

  if (vw1.num_bits != vw2.num_bits) { return "num_bits"; }

  if (vw1.permutations != vw2.permutations) { return "permutations"; }

  if (vw1.interactions.size() != vw2.interactions.size()) { return "interactions size"; }

  if (vw1.ignore_some != vw2.ignore_some) { return "ignore_some"; }

  if (vw1.ignore_some && !std::equal(vw1.ignore.begin(), vw1.ignore.end(), vw2.ignore.begin())) { return "ignore"; }

  if (vw1.ignore_some_linear != vw2.ignore_some_linear) { return "ignore_some_linear"; }

  if (vw1.ignore_some_linear &&
      !std::equal(vw1.ignore_linear.begin(), vw1.ignore_linear.end(), vw2.ignore_linear.begin()))
  {
    return "ignore_linear";
  }

  if (vw1.redefine_some != vw2.redefine_some) { return "redefine_some"; }

  if (vw1.redefine_some && !std::equal(vw1.redefine.begin(), vw1.redefine.end(), vw2.redefine.begin()))
  {
    return "redefine";
  }

  if (vw1.add_constant != vw2.add_constant) { return "add_constant"; }

  if (vw1.dictionary_path.size() != vw2.dictionary_path.size()) { return "dictionary_path size"; }

  if (!std::equal(vw1.dictionary_path.begin(), vw1.dictionary_path.end(), vw2.dictionary_path.begin()))
  {
    return "dictionary_path";
  }

  for (auto i = std::begin(vw1.interactions), j = std::begin(vw2.interactions); i != std::end(vw1.interactions);
       ++i, ++j)
  {
    if (*i != *j) { return "interaction mismatch"; }
  }

  return nullptr;
}

void VW::finish(VW::workspace& all, bool delete_all)
{
  auto deleter = VW::scope_exit(
      [&]
      {
        if (delete_all) { delete &all; }
      });

  all.finish();
}

void VW::sync_stats(VW::workspace& all)
{
  if (all.all_reduce != nullptr)
  {
    const auto loss = static_cast<float>(all.sd->sum_loss);
    all.sd->sum_loss = static_cast<double>(VW::details::accumulate_scalar(all, loss));
    const auto weighted_labeled_examples = static_cast<float>(all.sd->weighted_labeled_examples);
    all.sd->weighted_labeled_examples =
        static_cast<double>(VW::details::accumulate_scalar(all, weighted_labeled_examples));
    const auto weighted_labels = static_cast<float>(all.sd->weighted_labels);
    all.sd->weighted_labels = static_cast<double>(VW::details::accumulate_scalar(all, weighted_labels));
    const auto weighted_unlabeled_examples = static_cast<float>(all.sd->weighted_unlabeled_examples);
    all.sd->weighted_unlabeled_examples =
        static_cast<double>(VW::details::accumulate_scalar(all, weighted_unlabeled_examples));
    const auto example_number = static_cast<float>(all.sd->example_number);
    all.sd->example_number = static_cast<uint64_t>(VW::details::accumulate_scalar(all, example_number));
    const auto total_features = static_cast<float>(all.sd->total_features);
    all.sd->total_features = static_cast<uint64_t>(VW::details::accumulate_scalar(all, total_features));
  }
}

namespace
{

void thread_dispatch(VW::workspace& all, const VW::multi_ex& examples)
{
  for (auto* example : examples) { all.example_parser->ready_parsed_examples.push(example); }
}
void main_parse_loop(VW::workspace* all) { VW::details::parse_dispatch(*all, thread_dispatch); }
}  // namespace

void VW::start_parser(VW::workspace& all) { all.parse_thread = std::thread(main_parse_loop, &all); }
void VW::end_parser(VW::workspace& all) { all.parse_thread.join(); }

bool VW::is_ring_example(const VW::workspace& all, const example* ae)
{
  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  return all.example_parser->example_pool.is_from_pool(ae);
  VW_WARNING_STATE_POP
}

VW::example* VW::read_example(VW::workspace& all, const char* example_line)
{
  VW::example* ret = &get_unused_example(&all);

  VW::parsers::text::read_line(all, ret, example_line);
  setup_example(all, ret);

  return ret;
}

VW::example* VW::read_example(VW::workspace& all, const std::string& example_line)
{
  return read_example(all, example_line.c_str());
}
// The more complex way to create an example.

VW::example* VW::import_example(
    VW::workspace& all, const std::string& label, primitive_feature_space* features, size_t len)
{
  VW::example* ret = &get_unused_example(&all);
  all.example_parser->lbl_parser.default_label(ret->l);

  if (label.length() > 0) { parse_example_label(all, *ret, label); }

  for (size_t i = 0; i < len; i++)
  {
    unsigned char index = features[i].name;
    ret->indices.push_back(index);
    for (size_t j = 0; j < features[i].len; j++)
    {
      ret->feature_space[index].push_back(features[i].fs[j].x, features[i].fs[j].weight_index);
    }
  }

  setup_example(all, ret);
  return ret;
}
VW::example* VW::alloc_examples(size_t count) { return new VW::example[count]; }

void VW::dealloc_examples(example* example_ptr, size_t /* count */) { delete[] example_ptr; }

void VW::parse_example_label(VW::workspace& all, example& ec, const std::string& label)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  all.example_parser->lbl_parser.parse_label(ec.l, ec.ex_reduction_features, all.example_parser->parser_memory_to_reuse,
      all.sd->ldict.get(), words, all.logger);
}

void VW::setup_examples(VW::workspace& all, VW::multi_ex& examples)
{
  for (VW::example* ae : examples) { setup_example(all, ae); }
}

namespace
{
bool is_test_only(uint32_t counter, uint32_t period, uint32_t after, bool holdout_off,
    uint32_t target_modulus)  // target should be 0 in the normal case, or period-1 in the case that emptylines separate
                              // examples
{
  if (holdout_off) { return false; }
  if (after == 0)
  {  // hold out by period
    return (counter % period == target_modulus);
  }

  // hold out by position
  return (counter > after);
}

void feature_limit(VW::workspace& all, VW::example* ex)
{
  for (VW::namespace_index index : ex->indices)
  {
    if (all.limit[index] < ex->feature_space[index].size())
    {
      auto& fs = ex->feature_space[index];
      fs.sort(all.parse_mask);
      VW::unique_features(fs, all.limit[index]);
    }
  }
}

}  // namespace

void VW::setup_example(VW::workspace& all, VW::example* ae)
{
  assert(ae != nullptr);
  if (all.example_parser->sort_features && !ae->sorted) { unique_sort_features(all.parse_mask, *ae); }

  if (all.example_parser->write_cache)
  {
    VW::parsers::cache::write_example_to_cache(all.example_parser->output, ae, all.example_parser->lbl_parser,
        all.parse_mask, all.example_parser->cache_temp_buffer_obj);
  }

  // Require all extents to be complete in an VW::example.
#ifndef NDEBUG
  for (auto& fg : *ae) { assert(fg.validate_extents()); }
#endif

  ae->partial_prediction = 0.;
  ae->num_features = 0;
  ae->reset_total_sum_feat_sq();
  ae->loss = 0.;
  ae->debug_current_reduction_depth = 0;
  ae->_use_permutations = all.permutations;

  all.example_parser->num_setup_examples++;
  if (!all.example_parser->emptylines_separate_examples) { all.example_parser->in_pass_counter++; }

  // Determine if this example is part of the holdout set.
  ae->test_only = is_test_only(all.example_parser->in_pass_counter, all.holdout_period, all.holdout_after,
      all.holdout_set_off, all.example_parser->emptylines_separate_examples ? (all.holdout_period - 1) : 0);
  // If this example has a test only label then it is true regardless.
  ae->test_only |= all.example_parser->lbl_parser.test_label(ae->l);

  if (all.example_parser->emptylines_separate_examples &&
      (example_is_newline(*ae) &&
          (all.example_parser->lbl_parser.label_type != label_type_t::CCB ||
              VW::reductions::ccb::ec_is_example_unset(*ae))))
  {
    all.example_parser->in_pass_counter++;
  }

  ae->weight = all.example_parser->lbl_parser.get_weight(ae->l, ae->ex_reduction_features);

  if (all.ignore_some)
  {
    for (unsigned char* i = ae->indices.begin(); i != ae->indices.end(); i++)
    {
      if (all.ignore[*i])
      {
        // Delete namespace
        ae->feature_space[*i].clear();
        i = ae->indices.erase(i);
        // Offset the increment for this iteration so that is processes this index again which is actually the next
        // item.
        i--;
      }
    }
  }

  if (all.skip_gram_transformer != nullptr) { all.skip_gram_transformer->generate_grams(ae); }

  if (all.add_constant)
  {  // add constant feature
    VW::add_constant_feature(all, ae);
  }

  if (!all.limit_strings.empty()) { feature_limit(all, ae); }

  uint64_t multiplier = static_cast<uint64_t>(all.wpp) << all.weights.stride_shift();

  if (multiplier != 1)
  {  // make room for per-feature information.
    for (features& fs : *ae)
    {
      for (auto& j : fs.indices) { j *= multiplier; }
    }
  }
  ae->num_features = 0;
  for (const features& fs : *ae) { ae->num_features += fs.size(); }

  // Set the interactions for this example to the global set.
  ae->interactions = &all.interactions;
  ae->extent_interactions = &all.extent_interactions;
}

VW::example* VW::new_unused_example(VW::workspace& all)
{
  VW::example* ec = &get_unused_example(&all);
  all.example_parser->lbl_parser.default_label(ec->l);
  return ec;
}

VW::example* VW::get_example(parser* p)
{
  example* ex = nullptr;
  p->ready_parsed_examples.try_pop(ex);
  return ex;
}

float VW::get_topic_prediction(example* ec, size_t i) { return ec->pred.scalars[i]; }

float VW::get_label(example* ec) { return ec->l.simple.label; }

float VW::get_importance(example* ec) { return ec->weight; }

float VW::get_initial(example* ec)
{
  const auto& simple_red_features = ec->ex_reduction_features.template get<VW::simple_label_reduction_features>();
  return simple_red_features.initial;
}

float VW::get_prediction(example* ec) { return ec->pred.scalar; }

float VW::get_cost_sensitive_prediction(example* ec) { return static_cast<float>(ec->pred.multiclass); }

VW::v_array<float>& VW::get_cost_sensitive_prediction_confidence_scores(example* ec) { return ec->pred.scalars; }

uint32_t* VW::get_multilabel_predictions(example* ec, size_t& len)
{
  auto& labels = ec->pred.multilabels;
  len = labels.label_v.size();
  return labels.label_v.begin();
}

float VW::get_action_score(example* ec, size_t i)
{
  VW::action_scores scores = ec->pred.a_s;

  if (i < scores.size()) { return scores[i].score; }
  return 0.0;
}

size_t VW::get_action_score_length(example* ec) { return ec->pred.a_s.size(); }

size_t VW::get_tag_length(example* ec) { return ec->tag.size(); }

const char* VW::get_tag(example* ec) { return ec->tag.begin(); }

size_t VW::get_feature_number(example* ec) { return ec->get_num_features(); }

float VW::get_confidence(example* ec) { return ec->confidence; }

namespace
{

class features_and_source
{
public:
  VW::v_array<VW::feature> feature_map;  // map to store sparse feature vectors
  uint32_t stride_shift{};
  uint64_t mask{};
};

void vec_store(features_and_source& p, float fx, uint64_t fi)
{
  p.feature_map.push_back(VW::feature(fx, (fi >> p.stride_shift) & p.mask));
}
}  // namespace

VW::feature* VW::get_features(VW::workspace& all, example* ec, size_t& feature_number)
{
  features_and_source fs;
  fs.stride_shift = all.weights.stride_shift();
  fs.mask = all.weights.mask() >> all.weights.stride_shift();
  VW::foreach_feature<::features_and_source, uint64_t, vec_store>(all, *ec, fs);

  auto* features_array = new feature[fs.feature_map.size()];
  std::memcpy(features_array, fs.feature_map.data(), fs.feature_map.size() * sizeof(feature));
  feature_number = fs.feature_map.size();
  return features_array;
}

void VW::return_features(feature* f) { delete[] f; }

void VW::add_constant_feature(const VW::workspace& all, VW::example* ec)
{
  ec->indices.push_back(VW::details::CONSTANT_NAMESPACE);
  ec->feature_space[VW::details::CONSTANT_NAMESPACE].push_back(
      1, VW::details::CONSTANT, VW::details::CONSTANT_NAMESPACE);
  ec->num_features++;
  if (all.audit || all.hash_inv)
  {
    ec->feature_space[VW::details::CONSTANT_NAMESPACE].space_names.emplace_back("", "Constant");
  }
}
void VW::add_label(VW::example* ec, float label, float weight, float base)
{
  ec->l.simple.label = label;
  auto& simple_red_features = ec->ex_reduction_features.template get<VW::simple_label_reduction_features>();
  simple_red_features.initial = base;
  ec->weight = weight;
}

// notify VW that you are done with the example.
void VW::finish_example(VW::workspace& all, example& ec)
{
  // only return examples to the pool that are from the pool and not externally allocated
  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  if (!is_ring_example(all, &ec)) { return; }
  VW_WARNING_STATE_POP

  details::clean_example(all, ec);

  {
    std::lock_guard<std::mutex> lock(all.example_parser->output_lock);
    ++all.example_parser->num_finished_examples;
    all.example_parser->output_done.notify_one();
  }
}

void VW::finish_example(VW::workspace& all, multi_ex& ec_seq)
{
  for (example* ecc : ec_seq) { VW::finish_example(all, *ecc); }
}

void VW::empty_example(VW::workspace& /*all*/, example& ec)
{
  for (features& fs : ec) { fs.clear(); }

  ec.indices.clear();
  ec.tag.clear();
  ec.sorted = false;
  ec.end_pass = false;
  ec.is_newline = false;
  ec.ex_reduction_features.clear();
  ec.num_features_from_interactions = 0;
}

void VW::move_feature_namespace(example* dst, example* src, namespace_index c)
{
  if (std::find(src->indices.begin(), src->indices.end(), c) == src->indices.end())
  {
    return;  // index not present in src
  }
  if (std::find(dst->indices.begin(), dst->indices.end(), c) == dst->indices.end()) { dst->indices.push_back(c); }

  auto& fdst = dst->feature_space[c];
  auto& fsrc = src->feature_space[c];

  src->num_features -= fsrc.size();
  src->reset_total_sum_feat_sq();
  std::swap(fdst, fsrc);
  dst->num_features += fdst.size();
  dst->reset_total_sum_feat_sq();
}

void VW::copy_example_metadata(example* dst, const example* src)
{
  dst->tag = src->tag;
  dst->example_counter = src->example_counter;

  dst->ft_offset = src->ft_offset;

  dst->partial_prediction = src->partial_prediction;
  if (src->passthrough == nullptr) { dst->passthrough = nullptr; }
  else { dst->passthrough = new features(*src->passthrough); }
  dst->loss = src->loss;
  dst->weight = src->weight;
  dst->confidence = src->confidence;
  dst->test_only = src->test_only;
  dst->end_pass = src->end_pass;
  dst->is_newline = src->is_newline;
  dst->sorted = src->sorted;
}

void VW::copy_example_data(example* dst, const example* src)
{
  copy_example_metadata(dst, src);

  // copy feature data
  dst->indices = src->indices;
  for (namespace_index c : src->indices) { dst->feature_space[c] = src->feature_space[c]; }
  dst->num_features = src->num_features;
  dst->total_sum_feat_sq = src->total_sum_feat_sq;
  dst->_total_sum_feat_sq_calculated = src->_total_sum_feat_sq_calculated;
  dst->_use_permutations = src->_use_permutations;
  dst->interactions = src->interactions;
  dst->extent_interactions = src->extent_interactions;
  dst->debug_current_reduction_depth = src->debug_current_reduction_depth;
}

void VW::copy_example_data_with_label(example* dst, const example* src)
{
  copy_example_data(dst, src);
  dst->l = src->l;
}

VW::primitive_feature_space* VW::export_example(VW::workspace& all, VW::example* ec, size_t& len)
{
  len = ec->indices.size();
  auto* fs_ptr = new primitive_feature_space[len];

  size_t fs_count = 0;

  for (size_t idx = 0; idx < len; ++idx)
  {
    namespace_index i = ec->indices[idx];
    fs_ptr[fs_count].name = i;
    fs_ptr[fs_count].len = ec->feature_space[i].size();
    fs_ptr[fs_count].fs = new feature[fs_ptr[fs_count].len];

    uint32_t stride_shift = all.weights.stride_shift();

    auto& f = ec->feature_space[i];
    for (size_t f_count = 0; f_count < fs_ptr[fs_count].len; f_count++)
    {
      feature t = {f.values[f_count], f.indices[f_count]};
      t.weight_index >>= stride_shift;
      fs_ptr[fs_count].fs[f_count] = t;
    }
    fs_count++;
  }
  return fs_ptr;
}

void VW::release_feature_space(primitive_feature_space* features, size_t len)
{
  for (size_t i = 0; i < len; i++) { delete[] features[i].fs; }
  delete (features);
}

void VW::save_predictor(VW::workspace& all, const std::string& reg_name)
{
  VW::details::dump_regressor(all, reg_name, false);
}

void VW::save_predictor(VW::workspace& all, io_buf& buf) { VW::details::dump_regressor(all, buf, false); }
