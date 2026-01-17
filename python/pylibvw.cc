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
      .def("learn", &my_learn, "given a pyvw example, learn (and predict) on that example")
      .def("predict", &my_predict, "given a pyvw example, predict on that example")
      .def("_finish_example", &my_finish_example, "tell VW that you're done with a given example")
      .def("num_weights", &VW::num_weights, "how many weights are we learning?")
      .def("get_weight", &VW::get_weight, "get the weight for a particular index")
      .def("set_weight", &VW::set_weight, "set the weight for a particular index")
      .def("get_sum_loss", &get_sum_loss, "return the total cumulative loss suffered so far")
      
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
      .def("get_simplelabel_label", &ex_get_simplelabel_label,
          "Assuming a simple_label label type, return the corresponding label")
      .def("get_simplelabel_weight", &ex_get_simplelabel_weight,
          "Assuming a simple_label label type, return the importance weight");
}
