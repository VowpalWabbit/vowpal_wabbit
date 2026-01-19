// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/future_compat.h"
#include "vw/common/text_utils.h"
#include "vw/config/cli_options_serializer.h"
#include "vw/config/option.h"
#include "vw/config/options_cli.h"
#include "vw/core/cb.h"
#include "vw/core/cost_sensitive.h"
#include "vw/core/global_data.h"
#include "vw/core/kskip_ngram_transformer.h"
#include "vw/core/learner.h"
#include "vw/core/memory.h"
#include "vw/core/merge.h"
#include "vw/core/multiclass.h"
#include "vw/core/multilabel.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/reductions/search/search.h"
#include "vw/core/reductions/search/search_hooktask.h"
#include "vw/core/shared_data.h"
#include "vw/core/simple_label_parser.h"
#include "vw/core/slates_label.h"
#include "vw/core/vw.h"
#include "vw/text_parser/parse_example_text.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <memory>
#include <fstream>

namespace py = pybind11;

class py_log_wrapper;

typedef std::shared_ptr<VW::workspace> vw_ptr;
typedef std::shared_ptr<example> example_ptr;
typedef std::shared_ptr<Search::search> search_ptr;
typedef std::shared_ptr<Search::predictor> predictor_ptr;
typedef std::shared_ptr<py_log_wrapper> py_log_wrapper_ptr;

// Search type aliases
using action = uint32_t;
using ptag = uint32_t;

// Label type constants
const size_t lDEFAULT = 0;
const size_t lBINARY = 1;
const size_t lSIMPLE = 1;
const size_t lMULTICLASS = 2;
const size_t lCOST_SENSITIVE = 3;
const size_t lCONTEXTUAL_BANDIT = 4;
const size_t lMAX = 5;
const size_t lCONDITIONAL_CONTEXTUAL_BANDIT = 6;
const size_t lSLATES = 7;
const size_t lCONTINUOUS = 8;
const size_t lCONTEXTUAL_BANDIT_EVAL = 9;
const size_t lMULTILABEL = 10;

// Prediction type constants
const size_t pSCALAR = 0;
const size_t pSCALARS = 1;
const size_t pACTION_SCORES = 2;
const size_t pACTION_PROBS = 3;
const size_t pMULTICLASS = 4;
const size_t pMULTILABELS = 5;
const size_t pPROB = 6;
const size_t pMULTICLASSPROBS = 7;
const size_t pDECISION_SCORES = 8;
const size_t pACTION_PDF_VALUE = 9;
const size_t pPDF = 10;
const size_t pACTIVE_MULTICLASS = 11;
const size_t pNOPRED = 12;

// Type constants for CCB/Slates
const size_t tSHARED = 0;
const size_t tACTION = 1;
const size_t tSLOT = 2;
const size_t tUNSET = 3;

void dont_delete_me(void* arg) {}

// Note: No custom deleter needed - pybind11 manages Python object lifetimes automatically

// OptionManager class for get_options support (backward compatibility)
class OptionManager : VW::config::typed_option_visitor
{
  std::map<std::string, std::vector<VW::config::option_group_definition>> m_option_group_dic;
  py::object m_py_opt_class;
  VW::config::options_i& m_opt;
  std::vector<std::string>& m_enabled_learners;
  std::string default_group_name;
  py::object* m_visitor_output_var;

public:
  OptionManager(VW::config::options_i& options, std::vector<std::string>& enabled_learners, py::object py_class)
      : m_opt(options)
      , m_enabled_learners(enabled_learners)
      , m_option_group_dic(options.get_collection_of_options())
      , m_py_opt_class(py_class)
  {
    default_group_name = options.DEFAULT_TINT;
    m_visitor_output_var = nullptr;
  }

  py::object* value_to_pyobject(VW::config::typed_option<bool>& opt)
  {
    if (m_opt.was_supplied(opt.m_name))
    {
      if (opt.default_value_supplied())
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, opt.value(), true, opt.default_value(), true, opt.m_experimental));
      else
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, opt.value(), true, false, true, opt.m_experimental));
    }
    else
    {
      if (opt.default_value_supplied())
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, opt.default_value(), false, opt.default_value(), true, opt.m_experimental));
      else
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, false, false, false, true, opt.m_experimental));
    }
  }

  template <typename T>
  py::object* value_to_pyobject(VW::config::typed_option<T>& opt)
  {
    T not_supplied{};

    if (m_opt.was_supplied(opt.m_name))
    {
      if (opt.default_value_supplied())
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, opt.value(), true, opt.default_value(), true, opt.m_experimental));
      else
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, opt.value(), true, not_supplied, false, opt.m_experimental));
    }
    else
    {
      if (opt.default_value_supplied())
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, opt.default_value(), false, opt.default_value(), true, opt.m_experimental));
      else
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, py::none(), false, not_supplied, false, opt.m_experimental));
    }
  }

  template <typename T>
  py::object* value_to_pyobject(VW::config::typed_option<std::vector<T>>& opt)
  {
    py::list values;

    if (m_opt.was_supplied(opt.m_name))
    {
      auto vec = opt.value();
      if (vec.size() > 0)
      {
        for (auto const& item : vec) { values.append(py::cast(item)); }
      }
    }

    return new py::object(
        m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary, opt.m_allow_override,
            values, m_opt.was_supplied(opt.m_name), py::list(), opt.default_value_supplied(), opt.m_experimental));
  }

  template <typename T>
  py::object* transform_if_t(VW::config::base_option& base_option)
  {
    if (base_option.m_type_hash == typeid(T).hash_code())
    {
      auto typed = dynamic_cast<VW::config::typed_option<T>&>(base_option);
      return value_to_pyobject(typed);
    }

    return nullptr;
  }

  py::object base_option_to_pyobject(VW::config::base_option& option)
  {
    option.accept(*this);
    if (m_visitor_output_var != nullptr)
    {
      auto repack = py::object(*m_visitor_output_var);
      delete m_visitor_output_var;
      m_visitor_output_var = nullptr;
      return repack;
    }

    return {};
  }

  py::object get_vw_option_pyobjects(bool enabled_only)
  {
    py::dict dres;
    auto it = m_option_group_dic.begin();

    while (it != m_option_group_dic.end())
    {
      auto reduction_enabled =
          std::find(m_enabled_learners.begin(), m_enabled_learners.end(), it->first) != m_enabled_learners.end();

      if (((it->first).compare(default_group_name) != 0) && enabled_only && !reduction_enabled)
      {
        it++;
        continue;
      }

      py::list option_groups;

      for (auto& options_group : it->second)
      {
        py::list options;
        for (auto& opt : options_group.m_options)
        {
          auto temp = base_option_to_pyobject(*opt);
          options.append(temp);
        }

        option_groups.append(py::make_tuple(options_group.m_name, options));
      }

      dres[py::str(it->first)] = option_groups;

      it++;
    }
    return dres;
  }

  void visit(VW::config::typed_option<uint32_t>& opt) override { m_visitor_output_var = value_to_pyobject(opt); };
  void visit(VW::config::typed_option<uint64_t>& opt) override { m_visitor_output_var = value_to_pyobject(opt); };
  void visit(VW::config::typed_option<int64_t>& opt) override { m_visitor_output_var = value_to_pyobject(opt); };
  void visit(VW::config::typed_option<int32_t>& opt) override { m_visitor_output_var = value_to_pyobject(opt); };
  void visit(VW::config::typed_option<bool>& opt) override { m_visitor_output_var = value_to_pyobject(opt); };
  void visit(VW::config::typed_option<float>& opt) override { m_visitor_output_var = value_to_pyobject(opt); };
  void visit(VW::config::typed_option<std::string>& opt) override { m_visitor_output_var = value_to_pyobject(opt); };
  void visit(VW::config::typed_option<std::vector<std::string>>& opt) override
  {
    m_visitor_output_var = value_to_pyobject(opt);
  };
};

// Minimal py_log_wrapper class
class py_log_wrapper
{
public:
  py::object py_log;
  py_log_wrapper(py::object py_log) : py_log(py_log) {}

  static void trace_listener_py(void* wrapper, const std::string& message)
  {
    try
    {
      auto inst = static_cast<py_log_wrapper*>(wrapper);
      inst->py_log.attr("log")(message);
    }
    catch (...)
    {
      PyErr_Print();
      PyErr_Clear();
      std::cerr << "error using python logging. ignoring." << std::endl;
    }
  }
};

// Initialization function with logging
vw_ptr my_initialize_with_log(py::list args, py_log_wrapper_ptr py_log)
{
  std::vector<std::string> args_vec;
  for (auto item : args) 
  { 
    args_vec.push_back(item.cast<std::string>()); 
  }

  if (std::find(args_vec.begin(), args_vec.end(), "--no_stdin") == args_vec.end()) 
  { 
    args_vec.push_back("--no_stdin"); 
  }

  VW::driver_output_func_t trace_listener = nullptr;
  void* trace_context = nullptr;
  std::unique_ptr<VW::io::logger> logger_ptr = nullptr;

  if (py_log)
  {
    trace_listener = py_log_wrapper::trace_listener_py;
    trace_context = py_log.get();

    const auto log_function = [](void* context, VW::io::log_level level, const std::string& message)
    {
      _UNUSED(level);
      try
      {
        auto inst = static_cast<py_log_wrapper*>(context);
        inst->py_log.attr("log")(message);
      }
      catch (...)
      {
        PyErr_Print();
        PyErr_Clear();
        std::cerr << "error using python logging. ignoring." << std::endl;
      }
    };

    logger_ptr = VW::make_unique<VW::io::logger>(VW::io::create_custom_sink_logger(py_log.get(), log_function));
  }

  auto options = VW::make_unique<VW::config::options_cli>(args_vec);
  auto foo = VW::initialize(std::move(options), nullptr, trace_listener, trace_context, logger_ptr.get());
  return std::shared_ptr<VW::workspace>(foo.release());
}

// Initialization function without logging
vw_ptr my_initialize(py::list args)
{
  return my_initialize_with_log(args, nullptr);
}

vw_ptr merge_workspaces(vw_ptr base_workspace, py::list workspaces)
{
  std::vector<const VW::workspace*> const_workspaces;
  for (auto item : workspaces)
  {
    const_workspaces.push_back(item.cast<vw_ptr>().get());
  }
  return std::shared_ptr<VW::workspace>(VW::merge_models(base_workspace.get(), const_workspaces));
}

void my_run_parser(vw_ptr all)
{
  VW::start_parser(*all);
  VW::LEARNER::generic_driver(*all);
  VW::end_parser(*all);
}

struct python_dict_writer : VW::metric_sink_visitor
{
  python_dict_writer(py::dict& dest_dict) : _dest_dict(dest_dict) {}
  void int_metric(const std::string& key, uint64_t value) override { _dest_dict[key.c_str()] = value; }
  void float_metric(const std::string& key, float value) override { _dest_dict[key.c_str()] = value; }
  void string_metric(const std::string& key, const std::string& value) override { _dest_dict[key.c_str()] = value; }
  void bool_metric(const std::string& key, bool value) override { _dest_dict[key.c_str()] = value; }
  void sink_metric(const std::string& key, const VW::metric_sink& value) override
  {
    py::dict nested;
    auto nested_py = python_dict_writer(nested);
    value.visit(nested_py);
    _dest_dict[key.c_str()] = nested;
  }

private:
  py::dict& _dest_dict;
};

py::dict get_learner_metrics(vw_ptr all)
{
  py::dict dictionary;

  if (all->output_runtime.global_metrics.are_metrics_enabled())
  {
    auto metrics = all->output_runtime.global_metrics.collect_metrics(all->l.get());

    python_dict_writer writer(dictionary);
    metrics.visit(writer);
  }

  return dictionary;
}

// Basic workspace functions
void my_finish(vw_ptr all)
{
  all->finish();
}

void my_save(vw_ptr all, std::string name)
{
  VW::save_predictor(*all, name);
}

search_ptr get_search_ptr(vw_ptr all)
{
  auto* search_data = (Search::search*)all->l->get_internal_type_erased_data_pointer_test_use_only();
  return std::shared_ptr<Search::search>(search_data, [](Search::search*){});
}

py::object get_options(vw_ptr all, py::object py_class, bool enabled_only)
{
  std::vector<std::string> enabled_learners;
  if (all->l) all->l->get_enabled_learners(enabled_learners);
  auto opt_manager = OptionManager(*all->options, enabled_learners, py_class);
  return opt_manager.get_vw_option_pyobjects(enabled_only);
}

void my_audit_example(vw_ptr all, example_ptr ec)
{
  VW::details::print_audit_features(*all, *ec);
}

const char* get_model_id(vw_ptr all)
{
  return all->id.c_str();
}

std::string get_arguments(vw_ptr all)
{
  VW::config::cli_options_serializer serializer;
  for (auto const& option : all->options->get_all_options())
  {
    if (all->options->was_supplied(option->m_name)) serializer.add(*option);
  }
  return serializer.str();
}

py::list get_enabled_learners(vw_ptr all)
{
  py::list py_enabled_learners;
  std::vector<std::string> enabled_learners;
  if (all->l) all->l->get_enabled_learners(enabled_learners);
  for (auto ex : enabled_learners)
  {
    py_enabled_learners.append(ex);
  }
  return py_enabled_learners;
}

predictor_ptr get_predictor(search_ptr _sch, ptag my_tag)
{
  HookTask::task_data* d = _sch->get_task_data<HookTask::task_data>();
  Search::predictor* P = new Search::predictor(*_sch, my_tag);
  return std::shared_ptr<Search::predictor>(P);
}

VW::label_parser* get_label_parser(VW::workspace* all, size_t labelType)
{
  switch (labelType)
  {
    case lDEFAULT:
      return all ? &all->parser_runtime.example_parser->lbl_parser : NULL;
    case lBINARY:  // or #lSIMPLE
      return &VW::simple_label_parser_global;
    case lMULTICLASS:
      return &VW::multiclass_label_parser_global;
    case lCOST_SENSITIVE:
      return &VW::cs_label_parser_global;
    case lCONTEXTUAL_BANDIT:
      return &VW::cb_label_parser_global;
    case lCONDITIONAL_CONTEXTUAL_BANDIT:
      return &VW::ccb_label_parser_global;
    case lSLATES:
      return &VW::slates::slates_label_parser;
    case lCONTINUOUS:
      return &VW::cb_continuous::the_label_parser;
    case lCONTEXTUAL_BANDIT_EVAL:
      return &VW::cb_eval_label_parser_global;
    case lMULTILABEL:
      return &VW::multilabel_label_parser_global;
    default:
      THROW("get_label_parser called on invalid label type");
  }
}

size_t my_get_label_type(VW::workspace* all)
{
  VW::label_parser* lp = &all->parser_runtime.example_parser->lbl_parser;
  if (lp->parse_label == VW::simple_label_parser_global.parse_label) { return lSIMPLE; }
  else if (lp->parse_label == VW::multiclass_label_parser_global.parse_label) { return lMULTICLASS; }
  else if (lp->parse_label == VW::cs_label_parser_global.parse_label) { return lCOST_SENSITIVE; }
  else if (lp->parse_label == VW::cb_label_parser_global.parse_label) { return lCONTEXTUAL_BANDIT; }
  else if (lp->parse_label == VW::cb_eval_label_parser_global.parse_label) { return lCONTEXTUAL_BANDIT_EVAL; }
  else if (lp->parse_label == VW::ccb_label_parser_global.parse_label) { return lCONDITIONAL_CONTEXTUAL_BANDIT; }
  else if (lp->parse_label == VW::slates::slates_label_parser.parse_label) { return lSLATES; }
  else if (lp->parse_label == VW::cb_continuous::the_label_parser.parse_label) { return lCONTINUOUS; }
  else if (lp->parse_label == VW::multilabel_label_parser_global.parse_label) { return lMULTILABEL; }
  else { THROW("unsupported label parser used"); }
}

size_t my_get_prediction_type(vw_ptr all)
{
  VW::prediction_type_t pred_type = all->l->get_output_prediction_type();
  if (pred_type == VW::prediction_type_t::SCALAR) return pSCALAR;
  else if (pred_type == VW::prediction_type_t::SCALARS) return pSCALARS;
  else if (pred_type == VW::prediction_type_t::ACTION_SCORES) return pACTION_SCORES;
  else if (pred_type == VW::prediction_type_t::ACTION_PROBS) return pACTION_PROBS;
  else if (pred_type == VW::prediction_type_t::MULTICLASS) return pMULTICLASS;
  else if (pred_type == VW::prediction_type_t::MULTILABELS) return pMULTILABELS;
  else if (pred_type == VW::prediction_type_t::PROB) return pPROB;
  else if (pred_type == VW::prediction_type_t::MULTICLASS_PROBS) return pMULTICLASSPROBS;
  else if (pred_type == VW::prediction_type_t::DECISION_PROBS) return pDECISION_SCORES;
  else if (pred_type == VW::prediction_type_t::ACTION_PDF_VALUE) return pACTION_PDF_VALUE;
  else if (pred_type == VW::prediction_type_t::PDF) return pPDF;
  else if (pred_type == VW::prediction_type_t::ACTIVE_MULTICLASS) return pACTIVE_MULTICLASS;
  else if (pred_type == VW::prediction_type_t::NOPRED) return pNOPRED;
  else THROW("Unknown prediction type");
}

void my_delete_example(void* voidec)
{
  VW::example* ec = (VW::example*)voidec;
  delete ec;
}

VW::example* my_empty_example0(vw_ptr vw, size_t labelType)
{
  VW::label_parser* lp = get_label_parser(&*vw, labelType);
  VW::example* ec = new VW::example;
  lp->default_label(ec->l);
  ec->interactions = &vw->feature_tweaks_config.interactions;
  ec->extent_interactions = &vw->feature_tweaks_config.extent_interactions;
  return ec;
}

example_ptr my_empty_example(vw_ptr all, size_t label_type)
{
  VW::example* ec = my_empty_example0(all, label_type);
  return std::shared_ptr<VW::example>(ec, my_delete_example);
}

// Example parsing functions
example_ptr my_read_example(vw_ptr all, size_t labelType, std::string str)
{
  VW::example* ec = my_empty_example0(all, labelType);
  VW::parsers::text::read_line(*all, ec, str);
  VW::setup_example(*all, ec);
  return std::shared_ptr<VW::example>(ec, my_delete_example);
}

example_ptr my_existing_example(vw_ptr all, size_t labelType, example_ptr existing_example)
{
  return existing_example;
}

multi_ex unwrap_example_list(py::list& ec_list)
{
  multi_ex ex_coll;
  for (auto item : ec_list)
  {
    ex_coll.push_back(item.cast<example_ptr>().get());
  }
  return ex_coll;
}

void my_finish_example(vw_ptr all, example_ptr ec)
{
  all->finish_example(*ec);
}

void my_finish_multi_ex(vw_ptr& all, py::list& ec_list)
{
  multi_ex ex_coll = unwrap_example_list(ec_list);
  all->finish_example(ex_coll);
}

void my_learn(vw_ptr all, example_ptr ec)
{
  all->learn(*ec);
}

std::string my_json_weights(vw_ptr all)
{
  return all->dump_weights_to_json_experimental();
}

void my_predict(vw_ptr all, example_ptr ec)
{
  // Use the learner's predict directly instead of workspace::predict
  // because workspace::predict sets ec->test_only = true, which would
  // cause subsequent learn calls on the same example to skip learning.
  VW::LEARNER::require_singleline(all->l)->predict(*ec);
}

bool my_is_multiline(vw_ptr all)
{
  return all->l->is_multiline();
}

template<bool learn>
void predict_or_learn(vw_ptr& all, py::list& ec_list)
{
  multi_ex ex_coll = unwrap_example_list(ec_list);
  if (learn) { all->learn(ex_coll); }
  else {
    // Use the learner's predict directly instead of workspace::predict
    // because workspace::predict sets test_only = true for all examples,
    // which would cause subsequent learn calls to skip learning.
    VW::LEARNER::require_multiline(all->l)->predict(ex_coll);
  }
}

py::list my_parse(vw_ptr& all, std::string str)
{
  std::vector<example_ptr> ex_ptrs;

  if (my_is_multiline(all))
  {
    // For multiline learners, split by newlines and parse each line
    std::vector<VW::string_view> lines;
    VW::tokenize('\n', VW::string_view(str), lines);

    for (const auto& line : lines)
    {
      // Skip empty lines
      if (line.empty()) continue;

      // Get example from the pool (not new allocation)
      VW::example* ae = &VW::get_unused_example(all.get());
      VW::parsers::text::read_line(*all, ae, line);
      VW::setup_example(*all, ae);
      // Use dont_delete_me because examples are returned to pool via finish_example()
      ex_ptrs.push_back(std::shared_ptr<VW::example>(ae, dont_delete_me));
    }
  }
  else
  {
    // Get example from the pool (not new allocation)
    VW::example* ae = &VW::get_unused_example(all.get());
    VW::parsers::text::read_line(*all, ae, str);
    VW::setup_example(*all, ae);
    // Use dont_delete_me because examples are returned to pool via finish_example()
    ex_ptrs.push_back(std::shared_ptr<VW::example>(ae, dont_delete_me));
  }

  py::list result;
  for (auto& ex : ex_ptrs)
  {
    result.append(ex);
  }
  return result;
}

void my_learn_multi_ex(vw_ptr& all, py::list& ec_list)
{
  predict_or_learn<true>(all, ec_list);
}

void my_predict_multi_ex(vw_ptr& all, py::list& ec_list)
{
  predict_or_learn<false>(all, ec_list);
}

std::string varray_char_to_string(VW::v_array<char>& a)
{
  std::string ret = "";
  for (auto c : a) ret += c;
  return ret;
}

// Helper to convert v_array to py::list
template <class T>
py::list varray_to_pylist(const VW::v_array<T>& a)
{
  py::list list;
  for (const auto& elem : a) { list.append(elem); }
  return list;
}

// Minimal example accessor functions
std::string my_get_tag(example_ptr ec)
{
  return varray_char_to_string(ec->tag);
}

// Example namespace and feature methods
uint32_t ex_num_namespaces(example_ptr ec) 
{ 
  return (uint32_t)ec->indices.size(); 
}

unsigned char ex_namespace(example_ptr ec, uint32_t i) 
{ 
  return ec->indices[i]; 
}

uint32_t ex_num_features(example_ptr ec, unsigned char ns) 
{ 
  return (uint32_t)ec->feature_space[ns].size(); 
}

uint64_t ex_feature(example_ptr ec, unsigned char ns, uint32_t i) 
{ 
  return ec->feature_space[ns].indices[i]; 
}

float ex_feature_weight(example_ptr ec, unsigned char ns, uint32_t i) 
{ 
  return ec->feature_space[ns].values[i]; 
}

float ex_sum_feat_sq(example_ptr ec, unsigned char ns) 
{ 
  return ec->feature_space[ns].sum_feat_sq; 
}

void ex_push_feature(example_ptr ec, unsigned char ns, uint64_t fid, float v)
{
  // Note: push_back already updates sum_feat_sq internally, so we don't need
  // to add it here. The old Boost.Python code had this duplication bug.
  ec->feature_space[ns].push_back(v, fid);
}


void ex_push_feature_list(example_ptr ec, vw_ptr vw, unsigned char ns, uint64_t ns_hash, py::list& a)
{
  for (auto item_handle : a)
  {
    py::object item = py::reinterpret_borrow<py::object>(item_handle);
    float fval = 1.0f;  // Default value for bare strings/ints (Boost.Python compatibility)
    py::object feature_id = item;

    // If it's a tuple, extract (feature_id, value)
    if (py::isinstance<py::tuple>(item))
    {
      auto t = item.cast<py::tuple>();
      if (t.size() != 2) THROW("feature tuples must be (int/str, float)");
      feature_id = t[0];
      fval = t[1].cast<float>();
    }

    // Skip zero-valued features
    if (fval == 0.0f) continue;

    // Process feature_id (either from tuple or bare value)
    if (py::isinstance<py::str>(feature_id))
    {
      std::string fname = feature_id.cast<std::string>();
      uint64_t fhash = VW::hash_feature(*vw, fname, ns_hash);
      ex_push_feature(ec, ns, fhash, fval);
    }
    else if (py::isinstance<py::int_>(feature_id))
    {
      uint64_t fhash = feature_id.cast<uint64_t>();
      ex_push_feature(ec, ns, fhash, fval);
    }
    else
    {
      THROW("features must be strings, ints, or tuples of (str/int, float)");
    }
  }
}

void ex_push_feature_dict(example_ptr ec, vw_ptr vw, unsigned char ns, py::dict& d)
{
  uint64_t ns_hash = VW::hash_space(*vw, std::string(1, (char)ns));
  for (auto item : d)
  {
    float fval = item.second.cast<float>();
    if (fval == 0.0f) continue;

    if (py::isinstance<py::str>(item.first))
    {
      std::string fname = item.first.cast<std::string>();
      uint64_t fhash = VW::hash_feature(*vw, fname, ns_hash);
      ex_push_feature(ec, ns, fhash, fval);
    }
    else if (py::isinstance<py::int_>(item.first))
    {
      uint64_t fhash = item.first.cast<uint64_t>();
      ex_push_feature(ec, ns, fhash, fval);
    }
    else
    {
      THROW("feature id must be int or str");
    }
  }
}


void ex_push_namespace(example_ptr ec, unsigned char ns) 
{ 
  ec->indices.push_back(ns); 
}


void ex_ensure_namespace_exists(example_ptr ec, unsigned char ns)
{
  for (auto n : ec->indices)
  {
    if (n == ns) return;
  }
  ex_push_namespace(ec, ns);
}


void ex_push_dictionary(example_ptr ec, vw_ptr vw, py::dict dict)
{
  for (auto item : dict)
  {
    unsigned char ns = 0;
    uint64_t ns_hash = 0;
    
    if (py::isinstance<py::str>(item.first))
    {
      std::string ns_str = item.first.cast<std::string>();
      if (ns_str.length() == 0) THROW("namespace must be non-empty");
      ns = (unsigned char)ns_str[0];
      ns_hash = VW::hash_space(*vw, ns_str);
    }
    else if (py::isinstance<py::int_>(item.first))
    {
      ns = item.first.cast<unsigned char>();
      ns_hash = VW::hash_space(*vw, std::string(1, (char)ns));
    }
    else
    {
      THROW("namespace must be int or str");
    }
    
    ex_ensure_namespace_exists(ec, ns);

    if (py::isinstance<py::dict>(item.second))
    {
      py::dict fdict = item.second.cast<py::dict>();
      ex_push_feature_dict(ec, vw, ns, fdict);
    }
    else if (py::isinstance<py::list>(item.second))
    {
      py::list flist = item.second.cast<py::list>();
      ex_push_feature_list(ec, vw, ns, ns_hash, flist);
    }
    else
    {
      THROW("namespace value must be a dict or list of (feature, value) tuples");
    }
  }
}


bool ex_pop_feature(example_ptr ec, unsigned char ns)
{
  auto& fs = ec->feature_space[ns];
  if (fs.empty()) return false;
  float val = fs.values.back();
  fs.sum_feat_sq -= val * val;
  fs.indices.pop_back();
  fs.values.pop_back();
  return true;
}


void ex_erase_namespace(example_ptr ec, unsigned char ns)
{
  ec->feature_space[ns].clear();
  ec->feature_space[ns].sum_feat_sq = 0;
}


bool ex_pop_namespace(example_ptr ec)
{
  if (ec->indices.empty()) return false;
  ec->indices.pop_back();
  return true;
}

// Multi-example type (VW expects raw pointers)
typedef std::vector<VW::example*> multi_ex;

// Additional workspace functions
void my_setup_example(vw_ptr vw, example_ptr ec) 
{ 
  VW::setup_example(*vw, ec.get()); 
}

void unsetup_example(vw_ptr vwP, example_ptr ae)
{
  VW::workspace& all = *vwP;
  ae->partial_prediction = 0.;
  ae->num_features = 0;
  ae->reset_total_sum_feat_sq();
  ae->loss = 0.;

  if (all.feature_tweaks_config.ignore_some) { THROW("Cannot unsetup example when some namespaces are ignored"); }

  if (all.feature_tweaks_config.skip_gram_transformer != nullptr &&
      !all.feature_tweaks_config.skip_gram_transformer->get_initial_ngram_definitions().empty())
  {
    THROW("Cannot unsetup example when ngrams are in use");
  }

  if (all.feature_tweaks_config.add_constant)
  {
    ae->feature_space[VW::details::CONSTANT_NAMESPACE].clear();
    int hit_constant = -1;
    size_t N = ae->indices.size();
    for (size_t i = 0; i < N; i++)
    {
      int j = (int)(N - 1 - i);
      if (ae->indices[j] == VW::details::CONSTANT_NAMESPACE)
      {
        if (hit_constant >= 0) { THROW("Constant namespace was found twice. It can only exist 1 or 0 times."); }
        hit_constant = j;
        break;
      }
    }
    if (hit_constant >= 0)
    {
      for (size_t i = hit_constant; i < N - 1; i++) ae->indices[i] = ae->indices[i + 1];
      ae->indices.pop_back();
    }
  }

  uint32_t multiplier = all.reduction_state.total_feature_width << all.weights.stride_shift();
  if (multiplier != 1)  // make room for per-feature information.
    for (auto ns : ae->indices)
      for (auto& idx : ae->feature_space[ns].indices) idx /= multiplier;
}


void ex_set_label_string(example_ptr ec, vw_ptr vw, std::string label, size_t labelType)
{
  VW::label_parser& old_lp = vw->parser_runtime.example_parser->lbl_parser;
  vw->parser_runtime.example_parser->lbl_parser = *get_label_parser(&*vw, labelType);
  VW::parse_example_label(*vw, *ec, label);
  vw->parser_runtime.example_parser->lbl_parser = old_lp;
}

// Simple label accessors
float ex_get_simplelabel_label(example_ptr ec)
{
  return ec->l.simple.label;
}

float ex_get_simplelabel_weight(example_ptr ec)
{
  return ec->weight;
}

float ex_get_simplelabel_initial(example_ptr ec)
{
  return ec->ex_reduction_features.template get<VW::simple_label_reduction_features>().initial;
}

float ex_get_simplelabel_prediction(example_ptr ec)
{
  return ec->pred.scalar;
}

// Prediction type accessors
float ex_get_prob(example_ptr ec)
{
  return ec->pred.prob;
}

uint32_t ex_get_multiclass_label(example_ptr ec)
{
  return ec->l.multi.label;
}

float ex_get_multiclass_weight(example_ptr ec)
{
  return ec->l.multi.weight;
}

uint32_t ex_get_multiclass_prediction(example_ptr ec)
{
  return ec->pred.multiclass;
}

py::list ex_get_scalars(example_ptr ec)
{
  py::list values;
  const auto& scalars = ec->pred.scalars;
  for (float s : scalars) { values.append(s); }
  return values;
}

py::list ex_get_action_scores(example_ptr ec)
{
  py::list values;
  auto const& scores = ec->pred.a_s;
  std::vector<float> ordered_scores(scores.size());
  for (auto const& action_score : scores) { ordered_scores[action_score.action] = action_score.score; }
  for (auto action_score : ordered_scores) { values.append(action_score); }
  return values;
}

py::list ex_get_decision_scores(example_ptr ec)
{
  py::list values;
  for (auto const& scores : ec->pred.decision_scores)
  {
    py::list inner_list;
    for (auto action_score : scores) { inner_list.append(py::make_tuple(action_score.action, action_score.score)); }
    values.append(inner_list);
  }
  return values;
}

py::tuple ex_get_action_pdf_value(example_ptr ec)
{
  return py::make_tuple(ec->pred.pdf_value.action, ec->pred.pdf_value.pdf_value);
}

py::list ex_get_pdf(example_ptr ec)
{
  py::list values;
  for (auto const& segment : ec->pred.pdf)
  {
    values.append(py::make_tuple(segment.left, segment.right, segment.pdf_value));
  }
  return values;
}

py::tuple ex_get_active_multiclass(example_ptr ec)
{
  py::list values;
  for (auto const& query_needed_class : ec->pred.active_multiclass.more_info_required_for_classes)
  {
    values.append(query_needed_class);
  }
  return py::make_tuple(ec->pred.active_multiclass.predicted_class, values);
}

py::list ex_get_multilabel_predictions(example_ptr ec)
{
  py::list values;
  const auto& labels = ec->pred.multilabels;
  for (uint32_t l : labels.label_v) { values.append(l); }
  return values;
}

// Cost-sensitive label accessors
uint32_t ex_get_costsensitive_prediction(example_ptr ec)
{
  return ec->pred.multiclass;
}

uint32_t ex_get_costsensitive_num_costs(example_ptr ec)
{
  return (uint32_t)ec->l.cs.costs.size();
}

float ex_get_costsensitive_cost(example_ptr ec, uint32_t i)
{
  return ec->l.cs.costs[i].x;
}

uint32_t ex_get_costsensitive_class(example_ptr ec, uint32_t i)
{
  return ec->l.cs.costs[i].class_index;
}

float ex_get_costsensitive_partial_prediction(example_ptr ec, uint32_t i)
{
  return ec->l.cs.costs[i].partial_prediction;
}

float ex_get_costsensitive_wap_value(example_ptr ec, uint32_t i)
{
  return ec->l.cs.costs[i].wap_value;
}

// Contextual bandits label accessors
uint32_t ex_get_cbandits_prediction(example_ptr ec)
{
  return ec->pred.multiclass;
}

uint32_t ex_get_cbandits_weight(example_ptr ec)
{
  return ec->l.cb.weight;
}

uint32_t ex_get_cbandits_num_costs(example_ptr ec)
{
  return (uint32_t)ec->l.cb.costs.size();
}

float ex_get_cbandits_cost(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_cbandits_num_costs(ec)) { THROW("Cost index out of bounds"); }
  return ec->l.cb.costs[i].cost;
}

uint32_t ex_get_cbandits_class(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_cbandits_num_costs(ec)) { THROW("Class index out of bounds"); }
  return ec->l.cb.costs[i].action;
}

float ex_get_cbandits_probability(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_cbandits_num_costs(ec)) { THROW("Probability index out of bounds"); }
  return ec->l.cb.costs[i].probability;
}

float ex_get_cbandits_partial_prediction(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_cbandits_num_costs(ec)) { THROW("Partial prediction index out of bounds"); }
  return ec->l.cb.costs[i].partial_prediction;
}

// CB eval label accessors
uint32_t ex_get_cb_eval_action(example_ptr ec)
{
  return ec->l.cb_eval.action;
}

uint32_t ex_get_cb_eval_weight(example_ptr ec)
{
  return ec->l.cb_eval.event.weight;
}

uint32_t ex_get_cb_eval_num_costs(example_ptr ec)
{
  return (uint32_t)ec->l.cb_eval.event.costs.size();
}

float ex_get_cb_eval_cost(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_cb_eval_num_costs(ec)) { THROW("Cost index out of bounds"); }
  return ec->l.cb_eval.event.costs[i].cost;
}

uint32_t ex_get_cb_eval_class(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_cb_eval_num_costs(ec)) { THROW("Class index out of bounds"); }
  return ec->l.cb_eval.event.costs[i].action;
}

float ex_get_cb_eval_probability(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_cb_eval_num_costs(ec)) { THROW("Probability index out of bounds"); }
  return ec->l.cb_eval.event.costs[i].probability;
}

float ex_get_cb_eval_partial_prediction(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_cb_eval_num_costs(ec)) { THROW("Partial prediction index out of bounds"); }
  return ec->l.cb_eval.event.costs[i].partial_prediction;
}

// CB continuous label accessors
uint32_t ex_get_cb_continuous_num_costs(example_ptr ec)
{
  return (uint32_t)ec->l.cb_cont.costs.size();
}

float ex_get_cb_continuous_cost(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_cb_continuous_num_costs(ec)) { THROW("Cost index out of bounds"); }
  return ec->l.cb_cont.costs[i].cost;
}

uint32_t ex_get_cb_continuous_class(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_cb_continuous_num_costs(ec)) { THROW("Class index out of bounds"); }
  return ec->l.cb_cont.costs[i].action;
}

float ex_get_cb_continuous_pdf_value(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_cb_continuous_num_costs(ec)) { THROW("Pdf_value index out of bounds"); }
  return ec->l.cb_cont.costs[i].pdf_value;
}

// Slates label accessors
size_t ex_get_slates_type(example_ptr ec)
{
  switch (ec->l.slates.type)
  {
    case VW::slates::example_type::SHARED:
      return tSHARED;
    case VW::slates::example_type::ACTION:
      return tACTION;
    case VW::slates::example_type::SLOT:
      return tSLOT;
    default:
      return tUNSET;
  }
}

float ex_get_slates_weight(example_ptr ec)
{
  return ec->l.slates.weight;
}

bool ex_get_slates_labeled(example_ptr ec)
{
  return ec->l.slates.labeled;
}

float ex_get_slates_cost(example_ptr ec)
{
  return ec->l.slates.cost;
}

uint32_t ex_get_slates_slot_id(example_ptr ec)
{
  return ec->l.slates.slot_id;
}

size_t ex_get_slates_num_probabilities(example_ptr ec)
{
  return ec->l.slates.probabilities.size();
}

uint32_t ex_get_slates_action(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_slates_num_probabilities(ec)) { THROW("Action index out of bounds"); }
  return ec->l.slates.probabilities[i].action;
}

float ex_get_slates_probability(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_slates_num_probabilities(ec)) { THROW("Probability index out of bounds"); }
  return ec->l.slates.probabilities[i].score;
}

// CCB label accessors
size_t ex_get_ccb_type(example_ptr ec)
{
  switch (ec->l.conditional_contextual_bandit.type)
  {
    case VW::ccb_example_type::SHARED:
      return tSHARED;
    case VW::ccb_example_type::ACTION:
      return tACTION;
    case VW::ccb_example_type::SLOT:
      return tSLOT;
    default:
      return tUNSET;
  }
}

bool ex_get_ccb_has_outcome(example_ptr ec)
{
  return ec->l.conditional_contextual_bandit.outcome != nullptr;
}

float ex_get_ccb_outcome_cost(example_ptr ec)
{
  if (!ex_get_ccb_has_outcome(ec)) { THROW("This label has no outcome"); }
  return ec->l.conditional_contextual_bandit.outcome->cost;
}

size_t ex_get_ccb_num_probabilities(example_ptr ec)
{
  if (!ex_get_ccb_has_outcome(ec)) { THROW("This label has no outcome"); }
  return ec->l.conditional_contextual_bandit.outcome->probabilities.size();
}

size_t ex_get_ccb_num_explicitly_included_actions(example_ptr ec)
{
  const auto& label = ec->l.conditional_contextual_bandit;
  return label.explicit_included_actions.size();
}

uint32_t ex_get_ccb_action(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_ccb_num_probabilities(ec)) { THROW("Action index out of bounds"); }
  if (!ex_get_ccb_has_outcome(ec)) { THROW("This label has no outcome"); }
  const auto* outcome_ptr = ec->l.conditional_contextual_bandit.outcome;
  return outcome_ptr->probabilities[i].action;
}

float ex_get_ccb_probability(example_ptr ec, uint32_t i)
{
  if (i >= ex_get_ccb_num_probabilities(ec)) { THROW("Probability index out of bounds"); }
  if (!ex_get_ccb_has_outcome(ec)) { THROW("This label has no outcome"); }
  const auto* outcome_ptr = ec->l.conditional_contextual_bandit.outcome;
  return outcome_ptr->probabilities[i].score;
}

float ex_get_ccb_weight(example_ptr ec)
{
  return ec->l.conditional_contextual_bandit.weight;
}

py::list ex_get_ccb_explicitly_included_actions(example_ptr ec)
{
  const auto& label = ec->l.conditional_contextual_bandit;
  return varray_to_pylist(label.explicit_included_actions);
}

py::list ex_get_multilabel_labels(example_ptr ec)
{
  py::list l;
  for (const auto& v : ec->l.multilabels.label_v) { l.append(v); }
  return l;
}

// Additional example property accessors
float ex_get_topic_prediction(VW::workspace& all, example_ptr ec, size_t i)
{
  return VW::get_topic_prediction(ec.get(), i);
}

size_t get_example_counter(example_ptr ec) 
{ 
  return ec->example_counter; 
}

uint32_t get_ft_offset(example_ptr ec)
{
  return ec->ft_offset;
}

size_t ex_get_feature_number(example_ptr ec)
{
  return ec->get_num_features();
}

float get_partial_prediction(example_ptr ec)
{
  return ec->partial_prediction;
}

float ex_get_updated_prediction(example_ptr ec)
{
  return ec->updated_prediction;
}

float get_loss(example_ptr ec) 
{ 
  return ec->loss; 
}

float get_total_sum_feat_sq(example_ptr ec)
{
  return ec->total_sum_feat_sq;
}

// Workspace accessor functions
double get_sum_loss(vw_ptr vw)
{
  return vw->sd->sum_loss;
}

double get_holdout_sum_loss(vw_ptr vw)
{
  return vw->sd->holdout_sum_loss;
}

double get_weighted_examples(vw_ptr vw)
{
  return vw->sd->weighted_examples();
}

bool search_should_output(search_ptr _sch)
{
  return _sch->output().good();
}

void search_output(search_ptr _sch, std::string s)
{
  _sch->output() << s;
}

// Search helper functions
void verify_search_set_properly(search_ptr _sch)
{
  if (!_sch) { THROW("search object not initialized properly"); }
}

uint32_t search_get_num_actions(search_ptr _sch)
{
  verify_search_set_properly(_sch);
  HookTask::task_data* d = _sch->get_task_data<HookTask::task_data>();
  return (uint32_t)d->num_actions;
}

void search_run_fn(Search::search& _sch)
{
  try
  {
    HookTask::task_data* d = _sch.get_task_data<HookTask::task_data>();
    py::object run = *static_cast<py::object*>(d->run_object.get());
    run.attr("__call__")();
  }
  catch (const py::error_already_set&)
  {
    throw;
  }
  catch (...)
  {
    THROW("Exception in 'search_run_fn'");
  }
}

void search_setup_fn(Search::search& _sch)
{
  try
  {
    HookTask::task_data* d = _sch.get_task_data<HookTask::task_data>();
    py::object run = *static_cast<py::object*>(d->setup_object.get());
    run.attr("__call__")();
  }
  catch (const py::error_already_set&)
  {
    throw;
  }
  catch (...)
  {
    THROW("Exception in 'search_setup_fn'");
  }
}

void search_takedown_fn(Search::search& _sch)
{
  try
  {
    HookTask::task_data* d = _sch.get_task_data<HookTask::task_data>();
    py::object run = *static_cast<py::object*>(d->takedown_object.get());
    run.attr("__call__")();
  }
  catch (const py::error_already_set&)
  {
    throw;
  }
  catch (...)
  {
    THROW("Exception in 'search_takedown_fn'");
  }
}

void my_set_test_only(example_ptr ec, bool test_only)
{
  ec->test_only = test_only;
}

void set_force_oracle(search_ptr _sch, bool useOracle)
{
  verify_search_set_properly(_sch);
  _sch->set_force_oracle(useOracle);
}

void set_structured_predict_hook(
    search_ptr _sch, py::object run_object, py::object setup_object, py::object takedown_object)
{
  verify_search_set_properly(_sch);
  HookTask::task_data* d = _sch->get_task_data<HookTask::task_data>();
  d->run_object = nullptr;
  d->setup_object = nullptr;
  d->takedown_object = nullptr;
  _sch->set_force_oracle(false);

  d->run_f = &search_run_fn;
  d->run_object = std::make_shared<py::object>(run_object);
  if (!setup_object.is_none())
  {
    d->setup_object = std::make_shared<py::object>(setup_object);
    d->run_setup_f = &search_setup_fn;
  }
  if (!takedown_object.is_none())
  {
    d->takedown_object = std::make_shared<py::object>(takedown_object);
    d->run_takedown_f = &search_takedown_fn;
  }
}

bool po_exists(search_ptr _sch, std::string arg)
{
  HookTask::task_data* d = _sch->get_task_data<HookTask::task_data>();
  return d->arg->was_supplied(arg);
}

std::string po_get_string(search_ptr _sch, std::string arg)
{
  HookTask::task_data* d = _sch->get_task_data<HookTask::task_data>();
  return d->arg->get_typed_option<std::string>(arg).value();
}

int32_t po_get_int(search_ptr _sch, std::string arg)
{
  HookTask::task_data* d = _sch->get_task_data<HookTask::task_data>();
  try { return d->arg->get_typed_option<int32_t>(arg).value(); }
  catch (...) {}
  try { return static_cast<int32_t>(d->arg->get_typed_option<int64_t>(arg).value()); }
  catch (...) {}
  try { return (int32_t)d->arg->get_typed_option<uint32_t>(arg).value(); }
  catch (...) {}
  try { return (int32_t)d->arg->get_typed_option<uint64_t>(arg).value(); }
  catch (...) {}
  return d->arg->get_typed_option<int32_t>(arg).value();
}

py::object po_get(search_ptr _sch, std::string arg)
{
  try { return py::cast(po_get_string(_sch, arg)); }
  catch (...) {}
  try { return py::cast(po_get_int(_sch, arg)); }
  catch (...) {}
  return py::none();
}

// Predictor helper functions
void my_set_input(predictor_ptr P, example_ptr ec) { P->set_input(*ec); }
void my_set_input_at(predictor_ptr P, size_t posn, example_ptr ec) { P->set_input_at(posn, *ec); }

void my_add_oracle(predictor_ptr P, action a) { P->add_oracle(a); }
void my_add_oracles(predictor_ptr P, py::list& a)
{
  for (auto item : a) { P->add_oracle(item.cast<action>()); }
}

void my_add_allowed(predictor_ptr P, action a) { P->add_allowed(a); }
void my_add_alloweds(predictor_ptr P, py::list& a)
{
  for (auto item : a) { P->add_allowed(item.cast<action>()); }
}

void my_add_condition(predictor_ptr P, ptag t, char c) { P->add_condition(t, c); }
void my_add_condition_range(predictor_ptr P, ptag hi, ptag count, char name0)
{
  P->add_condition_range(hi, count, name0);
}

void my_set_oracle(predictor_ptr P, action a) { P->set_oracle(a); }
void my_set_oracles(predictor_ptr P, py::list& a)
{
  if (py::len(a) > 0)
    P->set_oracle(a[0].cast<action>());
  else
    P->erase_oracles();
  for (size_t i = 1; i < py::len(a); i++) P->add_oracle(a[i].cast<action>());
}

void my_set_allowed(predictor_ptr P, action a) { P->set_allowed(a); }
void my_set_alloweds(predictor_ptr P, py::list& a)
{
  if (py::len(a) > 0)
    P->set_allowed(a[0].cast<action>());
  else
    P->erase_alloweds();
  for (size_t i = 1; i < py::len(a); i++) P->add_allowed(a[i].cast<action>());
}

void my_set_condition(predictor_ptr P, ptag t, char c) { P->set_condition(t, c); }
void my_set_condition_range(predictor_ptr P, ptag hi, ptag count, char name0)
{
  P->set_condition_range(hi, count, name0);
}

void my_set_learner_id(predictor_ptr P, size_t id) { P->set_learner_id(id); }
void my_set_tag(predictor_ptr P, ptag t) { P->set_tag(t); }

PYBIND11_MODULE(pylibvw, m)
{
  m.doc() = "Vowpal Wabbit Python bindings (pybind11 version)";

  // VW::workspace class (bound as "vw")
  py::class_<VW::workspace, vw_ptr>(m, "vw",
      "the basic VW object that holds weight vector, parser, etc.")
      .def(py::init(&my_initialize))
      .def(py::init(&my_initialize_with_log))
      .def("run_parser", &my_run_parser, "parse external data file")
      .def("get_learner_metrics", &get_learner_metrics,
          "get current learner stack metrics. returns empty dict if --extra_metrics was not supplied.")
      .def("finish", &my_finish, "stop VW by calling finish (and, eg, write weights to disk)")
      .def("save", &my_save, "save model to filename")
      .def("learn", &my_learn, "given a pyvw example, learn (and predict) on that example")
      .def("json_weights", &my_json_weights, "get json string of current weights")
      .def("predict", &my_predict, "given a pyvw example, predict on that example")
      .def("hash_space", &VW::hash_space, "given a namespace (as a string), compute the hash of that namespace")
      .def("hash_feature", &VW::hash_feature,
          "given a feature string (arg2) and a hashed namespace (arg3), hash that feature")
      .def("_finish_example", &my_finish_example, "tell VW that you're done with a given example")
      .def("_finish_example_multi_ex", &my_finish_multi_ex, "tell VW that you're done with the given examples")
      .def("setup_example", &my_setup_example,
          "given an example that you've created by hand, prepare it for learning (eg, compute quadratic feature)")
      .def("unsetup_example", &unsetup_example,
          "reverse the process of setup, so that you can go back and modify this example")
      .def("num_weights", &VW::num_weights, "how many weights are we learning?")
      .def("get_weight", &VW::get_weight, "get the weight for a particular index")
      .def("set_weight", &VW::set_weight, "set the weight for a particular index")
      .def("get_stride", &VW::get_stride, "return the internal stride")
      .def("_get_label_type", &my_get_label_type, "return parse label type")
      .def("_get_prediction_type", &my_get_prediction_type, "return prediction type")
      .def("get_sum_loss", &get_sum_loss, "return the total cumulative loss suffered so far")
      .def("get_holdout_sum_loss", &get_holdout_sum_loss, "return the total cumulative holdout loss suffered so far")
      .def("get_weighted_examples", &get_weighted_examples, "return the total weight of examples so far")
      .def("get_search_ptr", &get_search_ptr, "return a pointer to the search data structure")
      .def("get_options", &get_options, "get available vw options")
      .def("audit_example", &my_audit_example, "print example audit information")
      .def("get_id", &get_model_id, "return the model id")
      .def("get_arguments", &get_arguments, "return the arguments after resolving all dependencies")
      .def("get_enabled_learners", &get_enabled_learners, "return the list of names of the enabled learners")
      .def("get_enabled_reductions", &get_enabled_learners, "return the list of names of the enabled learners")
      .def("learn_multi", &my_learn_multi_ex, "given a list pyvw examples, learn (and predict) on those examples")
      .def("predict_multi", &my_predict_multi_ex, "given a list of pyvw examples, predict on that example")
      .def("_parse", &my_parse, "Parse a string into a collection of VW examples")
      .def("_is_multiline", &my_is_multiline, "true if the base reduction is multiline")

      .def_readonly_static("lDefault", &lDEFAULT)
      .def_readonly_static("lBinary", &lBINARY)
      .def_readonly_static("lSimple", &lSIMPLE)
      .def_readonly_static("lMulticlass", &lMULTICLASS)
      .def_readonly_static("lCostSensitive", &lCOST_SENSITIVE)
      .def_readonly_static("lContextualBandit", &lCONTEXTUAL_BANDIT)
      .def_readonly_static("lMax", &lMAX)
      .def_readonly_static("lConditionalContextualBandit", &lCONDITIONAL_CONTEXTUAL_BANDIT)
      .def_readonly_static("lSlates", &lSLATES)
      .def_readonly_static("lContinuous", &lCONTINUOUS)
      .def_readonly_static("lContextualBanditEval", &lCONTEXTUAL_BANDIT_EVAL)
      .def_readonly_static("lMultilabel", &lMULTILABEL)

      .def_readonly_static("pSCALAR", &pSCALAR)
      .def_readonly_static("pSCALARS", &pSCALARS)
      .def_readonly_static("pACTION_SCORES", &pACTION_SCORES)
      .def_readonly_static("pACTION_PROBS", &pACTION_PROBS)
      .def_readonly_static("pMULTICLASS", &pMULTICLASS)
      .def_readonly_static("pMULTILABELS", &pMULTILABELS)
      .def_readonly_static("pPROB", &pPROB)
      .def_readonly_static("pMULTICLASSPROBS", &pMULTICLASSPROBS)
      .def_readonly_static("pDECISION_SCORES", &pDECISION_SCORES)
      .def_readonly_static("pACTION_PDF_VALUE", &pACTION_PDF_VALUE)
      .def_readonly_static("pPDF", &pPDF)
      .def_readonly_static("pACTIVE_MULTICLASS", &pACTIVE_MULTICLASS)
      .def_readonly_static("pNOPRED", &pNOPRED)

      .def_readonly_static("tUNSET", &tUNSET)
      .def_readonly_static("tSHARED", &tSHARED)
      .def_readonly_static("tACTION", &tACTION)
      .def_readonly_static("tSLOT", &tSLOT);

  // example class
  py::class_<example, example_ptr>(m, "example")
      .def(py::init(&my_read_example),
          "Given a string as an argument parse that into a VW example")
      .def(py::init(&my_empty_example),
          "Construct an empty example; you must provide a label type")
      .def(py::init(&my_existing_example),
          "Create a new example object pointing to an existing object")
      .def("set_test_only", &my_set_test_only, "Change the test-only bit on an example")

      .def("get_tag", &my_get_tag, "Returns the tag associated with this example")
      .def("get_topic_prediction", &ex_get_topic_prediction,
          "For LDA models, returns the topic prediction for the topic id given")
      .def("get_feature_number", &ex_get_feature_number, "Returns the total number of features for this example")

      .def("get_example_counter", &get_example_counter,
          "Returns the counter of total number of examples seen up to and including this one")
      .def("get_ft_offset", &get_ft_offset,
          "Returns the feature offset for this example (used, eg, by multiclass classification to bulk offset all features)")
      .def("get_partial_prediction", &get_partial_prediction,
          "Returns the partial prediction associated with this example")
      .def("get_updated_prediction", &ex_get_updated_prediction,
          "Returns the partial prediction as if we had updated it after learning")
      .def("get_loss", &get_loss, "Returns the loss associated with this example")
      .def("get_total_sum_feat_sq", &get_total_sum_feat_sq, "The total sum of feature-value squared for this example")

      // Namespace and feature methods
      .def("num_namespaces", &ex_num_namespaces, "The total number of namespaces associated with this example")
      .def("namespace", &ex_namespace,
          "Get the namespace id for namespace i (for i = 0.. num_namespaces)")
      .def("sum_feat_sq", &ex_sum_feat_sq,
          "Get the sum of feature-values squared for a given namespace id (id=character-ord)")
      .def("num_features_in", &ex_num_features, "Get the number of features in a given namespace id")
      .def("feature", &ex_feature, "Get the feature id for the ith feature in a given namespace id")
      .def("feature_weight", &ex_feature_weight, "The the feature value (weight) per .feature(...)")
      .def("push_hashed_feature", &ex_push_feature, "Add a hashed feature to a given namespace")
      .def("push_feature_list", &ex_push_feature_list, "Add a (Python) list of features to a given namespace")
      .def("push_feature_dict", &ex_push_dictionary, "Add a (Python) dictionary of namespace/feature-list pairs")
      .def("pop_feature", &ex_pop_feature,
          "Remove the top feature from a given namespace; returns True iff the list was non-empty")
      .def("push_namespace", &ex_push_namespace, "Add a new namespace")
      .def("ensure_namespace_exists", &ex_ensure_namespace_exists, "Add a new namespace if it doesn't already exist")
      .def("pop_namespace", &ex_pop_namespace, "Remove the top namespace off; returns True iff the list was non-empty")
      .def("erase_namespace", &ex_erase_namespace, "Remove all the features from a given namespace")
      .def("set_label_string", &ex_set_label_string, "(Re)assign the label of this example to this string")
      
      // Simple label accessors
      .def("get_simplelabel_label", &ex_get_simplelabel_label,
          "Assuming a simple_label label type, return the corresponding label")
      .def("get_simplelabel_weight", &ex_get_simplelabel_weight,
          "Assuming a simple_label label type, return the importance weight")
      .def("get_simplelabel_initial", &ex_get_simplelabel_initial,
          "Assuming a simple_label label type, return the initial value")
      .def("get_simplelabel_prediction", &ex_get_simplelabel_prediction,
          "Assuming a simple_label label type, return the prediction")

      // Multiclass label accessors
      .def("get_multiclass_label", &ex_get_multiclass_label,
          "Assuming a multiclass label type, return the corresponding label")
      .def("get_multiclass_weight", &ex_get_multiclass_weight,
          "Assuming a multiclass label type, return the importance weight")
      .def("get_multiclass_prediction", &ex_get_multiclass_prediction,
          "Assuming a multiclass label type, return the prediction")

      // Prediction type accessors
      .def("get_prob", &ex_get_prob, "Get probability from example prediction")
      .def("get_scalars", &ex_get_scalars, "Get scalar values from example prediction")
      .def("get_action_scores", &ex_get_action_scores, "Get action scores from example prediction")
      .def("get_decision_scores", &ex_get_decision_scores, "Get decision scores from example prediction")
      .def("get_action_pdf_value", &ex_get_action_pdf_value, "Get action and pdf value from example prediction")
      .def("get_pdf", &ex_get_pdf, "Get pdf from example prediction")
      .def("get_active_multiclass", &ex_get_active_multiclass, "Get active multiclass from example prediction")
      .def("get_multilabel_predictions", &ex_get_multilabel_predictions,
          "Get multilabel predictions from example prediction")
      .def("get_multilabel_labels", &ex_get_multilabel_labels,
          "Get multilabel labels from example label")

      // Cost-sensitive label accessors
      .def("get_costsensitive_prediction", &ex_get_costsensitive_prediction,
          "Assuming a cost_sensitive label type, return the prediction")
      .def("get_costsensitive_num_costs", &ex_get_costsensitive_num_costs,
          "Assuming a cost_sensitive label type, return the number of costs")
      .def("get_costsensitive_cost", &ex_get_costsensitive_cost,
          "Assuming a cost_sensitive label type, return the cost for the ith class")
      .def("get_costsensitive_class", &ex_get_costsensitive_class,
          "Assuming a cost_sensitive label type, return the ith class")
      .def("get_costsensitive_partial_prediction", &ex_get_costsensitive_partial_prediction,
          "Assuming a cost_sensitive label type, return the ith partial prediction")
      .def("get_costsensitive_wap_value", &ex_get_costsensitive_wap_value,
          "Assuming a cost_sensitive label type, return the ith wap value")

      // Contextual bandits label accessors
      .def("get_cbandits_prediction", &ex_get_cbandits_prediction,
          "Assuming a contextual_bandits label type, return the prediction")
      .def("get_cbandits_weight", &ex_get_cbandits_weight,
          "Assuming a contextual_bandits label type, get the weight")
      .def("get_cbandits_num_costs", &ex_get_cbandits_num_costs,
          "Assuming a contextual_bandits label type, return the number of costs")
      .def("get_cbandits_cost", &ex_get_cbandits_cost,
          "Assuming a contextual_bandits label type, return the cost for the ith action")
      .def("get_cbandits_class", &ex_get_cbandits_class,
          "Assuming a contextual_bandits label type, return the ith class")
      .def("get_cbandits_probability", &ex_get_cbandits_probability,
          "Assuming a contextual_bandits label type, return the ith probability")
      .def("get_cbandits_partial_prediction", &ex_get_cbandits_partial_prediction,
          "Assuming a contextual_bandits label type, return the ith partial prediction")

      // CB eval label accessors
      .def("get_cb_eval_action", &ex_get_cb_eval_action, "Assuming a cb_eval label type, get action")
      .def("get_cb_eval_weight", &ex_get_cb_eval_weight, "Assuming a cb_eval label type, get weight")
      .def("get_cb_eval_num_costs", &ex_get_cb_eval_num_costs,
          "Assuming a cb_eval label type, return the number of costs")
      .def("get_cb_eval_cost", &ex_get_cb_eval_cost,
          "Assuming a cb_eval label type, return the cost for the ith action")
      .def("get_cb_eval_class", &ex_get_cb_eval_class,
          "Assuming a cb_eval label type, return the ith class")
      .def("get_cb_eval_probability", &ex_get_cb_eval_probability,
          "Assuming a cb_eval label type, return the ith probability")
      .def("get_cb_eval_partial_prediction", &ex_get_cb_eval_partial_prediction,
          "Assuming a cb_eval label type, return the ith partial prediction")

      // CCB label accessors
      .def("get_ccb_type", &ex_get_ccb_type, "Assuming a CCB label type, return the example type")
      .def("get_ccb_has_outcome", &ex_get_ccb_has_outcome, "Assuming a CCB label type, check if has outcome")
      .def("get_ccb_cost", &ex_get_ccb_outcome_cost, "Assuming a CCB label type, get the cost")
      .def("get_ccb_num_probabilities", &ex_get_ccb_num_probabilities,
          "Assuming a CCB label type, return the number of probabilities")
      .def("get_ccb_num_explicitly_included_actions", &ex_get_ccb_num_explicitly_included_actions,
          "Assuming a CCB label type, return the number of explicitly included actions")
      .def("get_ccb_action", &ex_get_ccb_action, "Assuming a CCB label type, return the ith action")
      .def("get_ccb_probability", &ex_get_ccb_probability, "Assuming a CCB label type, return the ith probability")
      .def("get_ccb_weight", &ex_get_ccb_weight, "Assuming a CCB label type, get the weight")
      .def("get_ccb_explicitly_included_actions", &ex_get_ccb_explicitly_included_actions,
          "Assuming a CCB label type, return the list of explicitly included actions")

      // CB continuous label accessors
      .def("get_cb_continuous_num_costs", &ex_get_cb_continuous_num_costs,
          "Assuming a cb_continuous label type, return the number of costs")
      .def("get_cb_continuous_cost", &ex_get_cb_continuous_cost,
          "Assuming a cb_continuous label type, return the cost for the ith action")
      .def("get_cb_continuous_class", &ex_get_cb_continuous_class,
          "Assuming a cb_continuous label type, return the ith class")
      .def("get_cb_continuous_pdf_value", &ex_get_cb_continuous_pdf_value,
          "Assuming a cb_continuous label type, return the ith pdf_value")

      // Slates label accessors
      .def("get_slates_type", &ex_get_slates_type, "Assuming a slates label type, return the example type")
      .def("get_slates_weight", &ex_get_slates_weight, "Assuming a slates label type, get the weight")
      .def("get_slates_labeled", &ex_get_slates_labeled, "Assuming a slates label type, check if labeled")
      .def("get_slates_cost", &ex_get_slates_cost, "Assuming a slates label type, get the cost")
      .def("get_slates_slot_id", &ex_get_slates_slot_id, "Assuming a slates label type, get the slot id")
      .def("get_slates_num_probabilities", &ex_get_slates_num_probabilities,
          "Assuming a slates label type, return the number of probabilities")
      .def("get_slates_action", &ex_get_slates_action,
          "Assuming a slates label type, return the ith action")
      .def("get_slates_probability", &ex_get_slates_probability,
          "Assuming a slates label type, return the ith probability");

  // Search::predictor class
  py::class_<Search::predictor, predictor_ptr>(m, "predictor")
      .def("set_input", &my_set_input, "set the input (an example) for this predictor (non-LDF mode only)")
      .def("set_input_length", &Search::predictor::set_input_length, "declare the length of an LDF-sequence of examples")
      .def("set_input_at", &my_set_input_at,
          "put a given example at position in the LDF sequence (call after set_input_length)")
      .def("add_oracle", &my_add_oracle, "add an action to the current list of oracle actions")
      .def("add_oracles", &my_add_oracles, "add a list of actions to the current list of oracle actions")
      .def("add_allowed", &my_add_allowed, "add an action to the current list of allowed actions")
      .def("add_alloweds", &my_add_alloweds, "add a list of actions to the current list of allowed actions")
      .def("add_condition", &my_add_condition, "add a (tag,char) pair to the list of variables on which to condition")
      .def("add_condition_range", &my_add_condition_range,
          "given (tag,len,char), add (tag,char), (tag-1,char+1), ..., (tag-len,char+len) to the list of conditionings")
      .def("set_oracle", &my_set_oracle, "set an action as the current list of oracle actions")
      .def("set_oracles", &my_set_oracles, "set a list of actions as the current list of oracle actions")
      .def("set_allowed", &my_set_allowed, "set an action as the current list of allowed actions")
      .def("set_alloweds", &my_set_alloweds, "set a list of actions as the current list of allowed actions")
      .def("set_condition", &my_set_condition, "set a (tag,char) pair as the list of variables on which to condition")
      .def("set_condition_range", &my_set_condition_range,
          "given (tag,len,char), set (tag,char), (tag-1,char+1), ..., (tag-len,char+len) as the list of conditionings")
      .def("set_learner_id", &my_set_learner_id, "select the learner with which to make this prediction")
      .def("set_tag", &my_set_tag, "change the tag of this prediction")
      .def("predict", &Search::predictor::predict, "make a prediction");

  // py_log_wrapper class
  py::class_<py_log_wrapper, py_log_wrapper_ptr>(m, "vw_log",
      "do not use, see pyvw.Workspace.init(enable_logging..)")
      .def(py::init<py::object>());

  // Search::search class
  py::class_<Search::search, search_ptr>(m, "search", py::dynamic_attr())
      .def("set_options", &Search::search::set_options, "Set global search options (auto conditioning, etc.)")
      .def("get_history_length", &Search::search::get_history_length,
          "Get the value specified by --search_history_length")
      .def("loss", &Search::search::loss, "Declare a (possibly incremental) loss")
      .def("should_output", &search_should_output,
          "Check whether search wants us to output (only happens if you have -p running)")
      .def("predict_needs_example", &Search::search::predictNeedsExample,
          "Check whether a subsequent call to predict is actually going to use the example you pass---i.e., can you "
          "skip feature computation?")
      .def("output", &search_output, "Add a string to the coutput (should only do if should_output returns True)")
      .def("get_num_actions", &search_get_num_actions, "Return the total number of actions search was initialized with")
      .def("set_structured_predict_hook", &set_structured_predict_hook,
          "Set the hook (function pointer) that search should use for structured prediction (This is rarely called by "
          "the end user. It is used internally.)")
      .def("set_force_oracle", &set_force_oracle, "For oracle decoding when .predict is run")
      .def("is_ldf", &Search::search::is_ldf, "check whether this search task is running in LDF mode")
      .def("po_exists", &po_exists,
          "For program (cmd line) options, check to see if a given option was specified; eg _sch.po_exists(\"search\") "
          "should be True")
      .def("po_get", &po_get,
          "For program (cmd line) options, if an option was specified, get its value; eg _sch.po_get(\"search\") "
          "should "
          "return the # of actions (returns either int or string)")
      .def("po_get_str", &po_get_string, "Same as po_get, but specialized for string return values.")
      .def("po_get_int", &po_get_int, "Same as po_get, but specialized for integer return values.")
      .def("get_predictor", &get_predictor,
          "Get a predictor object that can be used for making predictions; requires a tag argument to tag the "
          "prediction.")
      .def_readonly_static("AUTO_CONDITION_FEATURES", &Search::AUTO_CONDITION_FEATURES,
          "Tell search to automatically add features based on conditioned-on variables")
      .def_readonly_static("AUTO_HAMMING_LOSS", &Search::AUTO_HAMMING_LOSS,
          "Tell search to automatically compute hamming loss over predictions")
      .def_readonly_static("EXAMPLES_DONT_CHANGE", &Search::EXAMPLES_DONT_CHANGE,
          "Tell search that on a single structured 'run', you don't change the examples you pass to predict")
      .def_readonly_static("IS_LDF", &Search::IS_LDF, "Tell search that this is an LDF task");

  m.def("_merge_models_impl", &merge_workspaces, "Merge several Workspaces into one. Experimental.");
}
