// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/future_compat.h"
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

namespace py = pybind11;

class py_log_wrapper;

typedef std::shared_ptr<VW::workspace> vw_ptr;
typedef std::shared_ptr<example> example_ptr;
typedef std::shared_ptr<Search::search> search_ptr;
typedef std::shared_ptr<Search::predictor> predictor_ptr;
typedef std::shared_ptr<py_log_wrapper> py_log_wrapper_ptr;

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

void my_delete_example(void* voidec)
{
  VW::example* ec = (VW::example*)voidec;
  delete ec;
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

VW::example* my_empty_example0(vw_ptr vw, size_t labelType)
{
  VW::label_parser* lp = get_label_parser(&*vw, labelType);
  VW::example* ec = new VW::example;
  lp->default_label(ec->l);
  ec->interactions = &vw->feature_tweaks_config.interactions;
  ec->extent_interactions = &vw->feature_tweaks_config.extent_interactions;
  return ec;
}

void my_run_parser(vw_ptr all)
{
  VW::start_parser(*all);
  VW::LEARNER::generic_driver(*all);
  VW::end_parser(*all);
}

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

// Basic workspace functions
void my_finish(vw_ptr all)
{
  all->finish();
}

void my_save(vw_ptr all, std::string name) 
{ 
  VW::save_predictor(*all, name); 
}

// Example parsing functions
example_ptr my_read_example(vw_ptr all, size_t labelType, std::string str)
{
  VW::example* ec = my_empty_example0(all, labelType);
  VW::parsers::text::read_line(*all, ec, (char*)str.c_str());
  VW::setup_example(*all, ec);
  return std::shared_ptr<VW::example>(ec, my_delete_example);
}

example_ptr my_empty_example(vw_ptr all, size_t label_type)
{
  VW::example* ec = my_empty_example0(all, label_type);
  return std::shared_ptr<VW::example>(ec, my_delete_example);
}

example_ptr my_existing_example(vw_ptr all, size_t labelType, example_ptr existing_example)
{
  return existing_example;
}

void my_finish_example(vw_ptr all, example_ptr ec)
{
  all->finish_example(*ec);
}

void my_learn(vw_ptr all, example_ptr ec)
{
  all->learn(*ec);
}

void my_predict(vw_ptr all, example_ptr ec)
{
  all->predict(*ec);
}

float get_sum_loss(vw_ptr all)
{
  return static_cast<float>(all->sd->sum_loss);
}

// Minimal example accessor functions
std::string my_get_tag(example_ptr ec)
{
  return std::string(ec->tag.begin(), ec->tag.end());
}

void my_set_test_only(example_ptr ec, bool test_only)
{
  ec->test_only = test_only;
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

// Multi-example type (VW expects raw pointers)
typedef std::vector<VW::example*> multi_ex;

// Additional workspace functions
void my_setup_example(vw_ptr vw, example_ptr ec) 
{ 
  VW::setup_example(*vw, ec.get()); 
}

void unsetup_example(vw_ptr vwP, example_ptr ae)
{
  if (vwP->output_config.audit || vwP->output_config.hash_inv)
  {
    VW::details::truncate_example_namespaces_from_example(*ae, *ae);
  }
  ae->indices.clear();
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

bool my_is_multiline(vw_ptr all)
{
  return all->l->is_multiline();
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

py::list my_parse(vw_ptr& all, std::string str)
{
  std::vector<example_ptr> ex_ptrs;
  VW::example* ae = VW::make_unique<VW::example>().release();
  char* cstr = (char*)str.c_str();
  VW::parsers::text::read_line(*all, ae, cstr);
  VW::setup_example(*all, ae);
  if (my_is_multiline(all))
  {
    while (!VW::example_is_newline(*ae))
    {
      ex_ptrs.push_back(std::shared_ptr<VW::example>(ae, my_delete_example));
      ae = VW::make_unique<VW::example>().release();
      VW::parsers::text::read_line(*all, ae, cstr);
      VW::setup_example(*all, ae);
    }
    ex_ptrs.push_back(std::shared_ptr<VW::example>(ae, my_delete_example));
  }
  else
  {
    ex_ptrs.push_back(std::shared_ptr<VW::example>(ae, my_delete_example));
  }

  py::list result;
  for (auto& ex : ex_ptrs)
  {
    result.append(ex);
  }
  return result;
}

void my_finish_multi_ex(vw_ptr& all, py::list& ec_list)
{
  multi_ex ex_coll = unwrap_example_list(ec_list);
  all->finish_example(ex_coll);
}

template<bool learn>
void predict_or_learn(vw_ptr& all, py::list& ec_list)
{
  multi_ex ex_coll = unwrap_example_list(ec_list);
  if (learn) { all->learn(ex_coll); }
  else { all->predict(ex_coll); }
}

void my_learn_multi_ex(vw_ptr& all, py::list& ec_list) 
{ 
  predict_or_learn<true>(all, ec_list); 
}

void my_predict_multi_ex(vw_ptr& all, py::list& ec_list) 
{ 
  predict_or_learn<false>(all, ec_list); 
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

void my_audit_example(vw_ptr all, example_ptr ec) 
{ 
  VW::details::print_audit_features(*all, *ec); 
}

const char* get_model_id(vw_ptr all) 
{ 
  return all->id.c_str(); 
}

std::string my_json_weights(vw_ptr all) 
{ 
  return all->dump_weights_to_json_experimental(); 
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
  ec->feature_space[ns].push_back(v, fid);
  ec->feature_space[ns].sum_feat_sq += v * v;
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

bool ex_pop_namespace(example_ptr ec)
{
  if (ec->indices.empty()) return false;
  ec->indices.pop_back();
  return true;
}

void ex_erase_namespace(example_ptr ec, unsigned char ns)
{
  ec->feature_space[ns].clear();
  ec->feature_space[ns].sum_feat_sq = 0;
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

void ex_push_feature_list(example_ptr ec, vw_ptr vw, unsigned char ns, py::list& a)
{
  for (auto item : a)
  {
    if (py::isinstance<py::tuple>(item))
    {
      auto t = item.cast<py::tuple>();
      if (t.size() != 2) THROW("features must be tuples of (int/str, float)");
      float fval = t[1].cast<float>();
      
      if (py::isinstance<py::str>(t[0]))
      {
        std::string fname = t[0].cast<std::string>();
        uint64_t fhash = VW::hash_feature(*vw, fname, VW::hash_space(*vw, std::string(1, (char)ns)));
        ex_push_feature(ec, ns, fhash, fval);
      }
      else if (py::isinstance<py::int_>(t[0]))
      {
        uint64_t fhash = t[0].cast<uint64_t>();
        ex_push_feature(ec, ns, fhash, fval);
      }
      else
      {
        THROW("feature id must be int or str");
      }
    }
    else
    {
      THROW("features must be a list of tuples (int/str, float)");
    }
  }
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
    
    if (py::isinstance<py::list>(item.second))
    {
      py::list flist = item.second.cast<py::list>();
      ex_push_feature_list(ec, vw, ns, flist);
    }
    else
    {
      THROW("namespace value must be a list of (feature, value) tuples");
    }
  }
}

void ex_set_label_string(example_ptr ec, vw_ptr vw, std::string label, size_t labelType)
{
  VW::label_parser& old_lp = vw->parser_runtime.example_parser->lbl_parser;
  vw->parser_runtime.example_parser->lbl_parser = *get_label_parser(&*vw, labelType);
  VW::parse_example_label(*vw, *ec, label);
  vw->parser_runtime.example_parser->lbl_parser = old_lp;
}

// Additional label getters
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

size_t get_example_counter(example_ptr ec) 
{ 
  return ec->example_counter; 
}

uint32_t get_ft_offset(example_ptr ec) 
{ 
  return ec->ft_offset; 
}

float get_partial_prediction(example_ptr ec) 
{ 
  return ec->partial_prediction; 
}

float get_loss(example_ptr ec) 
{ 
  return ec->loss; 
}

float get_total_sum_feat_sq(example_ptr ec) 
{ 
  return ec->total_sum_feat_sq; 
}
PYBIND11_MODULE(pylibvw, m)
{
  m.doc() = "Vowpal Wabbit Python bindings (pybind11 version)";

  // py_log_wrapper class
  py::class_<py_log_wrapper, py_log_wrapper_ptr>(m, "vw_log",
      "do not use, see pyvw.Workspace.init(enable_logging..)")
      .def(py::init<py::object>());

  // VW::workspace class (bound as "vw")
  py::class_<VW::workspace, vw_ptr>(m, "vw", 
      "the basic VW object that holds weight vector, parser, etc.")
      .def(py::init(&my_initialize))
      .def(py::init(&my_initialize_with_log))
      .def("finish", &my_finish, "stop VW by calling finish (and, eg, write weights to disk)")
      .def("save", &my_save, "save model to filename")
      .def("run_parser", &my_run_parser, "parse external data file")
      .def("learn", &my_learn, "given a pyvw example, learn (and predict) on that example")
      .def("predict", &my_predict, "given a pyvw example, predict on that example")
      .def("learn_multi", &my_learn_multi_ex, "given a list pyvw examples, learn (and predict) on those examples")
      .def("predict_multi", &my_predict_multi_ex, "given a list of pyvw examples, predict on that example")
      .def("_finish_example", &my_finish_example, "tell VW that you're done with a given example")
      .def("_finish_example_multi_ex", &my_finish_multi_ex, "tell VW that you're done with the given examples")
      .def("_parse", &my_parse, "Parse a string into a collection of VW examples")
      .def("_is_multiline", &my_is_multiline, "true if the base reduction is multiline")
      .def("setup_example", &my_setup_example,
          "given an example that you've created by hand, prepare it for learning (eg, compute quadratic feature)")
      .def("unsetup_example", &unsetup_example,
          "reverse the process of setup, so that you can go back and modify this example")
      .def("num_weights", &VW::num_weights, "how many weights are we learning?")
      .def("get_weight", &VW::get_weight, "get the weight for a particular index")
      .def("set_weight", &VW::set_weight, "set the weight for a particular index")
      .def("get_stride", &VW::get_stride, "return the internal stride")
      .def("get_sum_loss", &get_sum_loss, "return the total cumulative loss suffered so far")
      .def("hash_space", &VW::hash_space, "given a namespace (as a string), compute the hash of that namespace")
      .def("hash_feature", &VW::hash_feature,
          "given a feature string (arg2) and a hashed namespace (arg3), hash that feature")
      .def("_get_prediction_type", &my_get_prediction_type, "return prediction type")
      .def("get_arguments", &get_arguments, "return the arguments after resolving all dependencies")
      .def("get_enabled_learners", &get_enabled_learners, "return the list of names of the enabled learners")
      .def("get_enabled_reductions", &get_enabled_learners, "return the list of names of the enabled learners")
      .def("audit_example", &my_audit_example, "print example audit information")
      .def("get_id", &get_model_id, "return the model id")
      .def("json_weights", &my_json_weights, "get json string of current weights")
      
      // Label type constants
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
      
      // Prediction type constants
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
      
      // Type constants
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
          
      // Multiclass label accessors
      .def("get_multiclass_label", &ex_get_multiclass_label,
          "Assuming a multiclass label type, return the corresponding label")
      .def("get_multiclass_weight", &ex_get_multiclass_weight,
          "Assuming a multiclass label type, return the importance weight")
      .def("get_multiclass_prediction", &ex_get_multiclass_prediction,
          "Assuming a multiclass label type, return the prediction")
          
      // General example properties
      .def("get_example_counter", &get_example_counter,
          "Returns the counter of total number of examples seen up to and including this one")
      .def("get_ft_offset", &get_ft_offset,
          "Returns the feature offset for this example")
      .def("get_partial_prediction", &get_partial_prediction,
          "Returns the partial prediction associated with this example")
      .def("get_loss", &get_loss, "Returns the loss associated with this example")
      .def("get_total_sum_feat_sq", &get_total_sum_feat_sq, "The total sum of feature-value squared for this example");
}
