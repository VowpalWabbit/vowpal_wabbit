
#include "vw.h"
#include "wrapper_reduction.h"
#include "reductions_fwd.h"
#include "reduction_stack.h"
#include "parse_example.h"
#include "options_boost_po.h"
#include "options_serializer_boost_po.h"

#define BOOST_PYTHON_USE_GCC_SYMBOL_VISIBILITY 1
#include <boost/make_shared.hpp>
#include <boost/python.hpp>

// Brings VW_DLL_PUBLIC to help control exports
#define VWDLL_EXPORTS
#include "../vowpalwabbit/vwdll.h"

typedef boost::shared_ptr<example> example_ptr;
typedef std::vector<example_ptr> ex_list;
class PyCppCallback;
typedef boost::shared_ptr<PyCppCallback> py_cpp_callback_ptr;

namespace py = boost::python;

void dont_delete_me(void* arg) {}

class OptionManager
{
  std::map<std::string, std::vector<VW::config::option_group_definition>> m_option_group_dic;
  // see pyvw.py class VWOption
  py::object m_py_opt_class;
  VW::config::options_i& m_opt;
  std::vector<std::string>& m_enabled_reductions;
  std::string default_group_name;

public:
  OptionManager(VW::config::options_i& options, std::vector<std::string>& enabled_reductions, py::object py_class)
      : m_opt(options)
      , m_enabled_reductions(enabled_reductions)
      , m_option_group_dic(options.get_collection_of_options())
      , m_py_opt_class(py_class)
  {
    default_group_name = static_cast<VW::config::options_boost_po*>(&options)->m_default_tint;
  }

  py::object* value_to_pyobject(VW::config::typed_option<bool>& opt)
  {
    if (m_opt.was_supplied(opt.m_name))
    {
      if (opt.default_value_supplied())
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, opt.value(), true, opt.default_value(), true));
      else
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, opt.value(), true, false, true));
    }
    else
    {
      if (opt.default_value_supplied())
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, opt.default_value(), false, opt.default_value(), true));
      else
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, false, false, false, true));
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
            opt.m_allow_override, opt.value(), true, opt.default_value(), true));
      else
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, opt.value(), true, not_supplied, false));
    }
    else
    {
      if (opt.default_value_supplied())
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, opt.default_value(), false, opt.default_value(), true));
      else
        return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
            opt.m_allow_override, py::object(), false, not_supplied, false));
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
        for (auto const& opt : vec) { values.append(py::object(opt)); }
      }
    }

    return new py::object(m_py_opt_class(opt.m_name, opt.m_help, opt.m_short_name, opt.m_keep, opt.m_necessary,
        opt.m_allow_override, values, m_opt.was_supplied(opt.m_name), py::list(), opt.default_value_supplied()));
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

  template <typename TTypes>
  py::object base_option_to_pyobject(VW::config::base_option& options)
  {
    py::object* temp = transform_if_t<typename TTypes::head>(options);
    if (temp != nullptr)
    {
      auto repack = py::object(*temp);
      delete temp;
      return repack;
    }

    return base_option_to_pyobject<typename TTypes::tail>(options);
  }

  py::object get_vw_option_pyobjects(bool enabled_only)
  {
    py::dict dres;
    auto it = m_option_group_dic.begin();

    while (it != m_option_group_dic.end())
    {
      auto reduction_enabled =
          std::find(m_enabled_reductions.begin(), m_enabled_reductions.end(), it->first) != m_enabled_reductions.end();

      if (((it->first).compare(default_group_name) != 0) && enabled_only && !reduction_enabled)
      {
        it++;
        continue;
      }

      py::list option_groups;

      for (auto options_group : it->second)
      {
        py::list options;
        for (auto opt : options_group.m_options)
        {
          auto temp = base_option_to_pyobject<VW::config::supported_options_types>(*opt.get());
          options.append(temp);
        }

        option_groups.append(py::make_tuple(options_group.m_name, options));
      }

      dres[it->first] = option_groups;

      it++;
    }
    return dres;
  }
};

class PyCppCallback
{
private:
  void* base_learner;

  bool isMulti = false;
  multi_ex* examples = nullptr;

  io_buf* model_file = nullptr;

public:
  PyCppCallback(void* base_learner) : base_learner(base_learner) {}
  PyCppCallback(io_buf* model_file) : model_file(model_file) {}
  PyCppCallback(void* base_learner, multi_ex* examples) : base_learner(base_learner), examples(examples)
  {
    isMulti = true;
  }

  void CallBaseLearner(example_ptr ec, bool should_call_learn = true)
  {
    if (!isMulti)
    {
      if (should_call_learn)
        reinterpret_cast<VW::LEARNER::single_learner*>(this->base_learner)->learn(*ec.get());
      else
        reinterpret_cast<VW::LEARNER::single_learner*>(this->base_learner)->predict(*ec.get());
    }
  }

  // 3. keep track of the multi_ex copy of the vector<boost pointer> copy (to avoid another copy)
  //          both lists points to the same elements unless end user adds or removes
  //          if that's the case we need to be able to override and force a copy
  void CallMultiLearner(ex_list& example_list, bool should_call_learn = true)
  {
    if (isMulti)
    {
      if (should_call_learn)
        VW::LEARNER::multiline_learn_or_predict<true>(
            *reinterpret_cast<VW::LEARNER::multi_learner*>(base_learner), *examples, (*examples)[0]->ft_offset);
      else
        VW::LEARNER::multiline_learn_or_predict<false>(
            *reinterpret_cast<VW::LEARNER::multi_learner*>(base_learner), *examples, (*examples)[0]->ft_offset);
    }
  }
};

class PyCppBridge : public WRAPPER::ExternalBinding
{
private:
  py::object* py_reduction_impl;
  void* base_learner;
  bool register_finish_learn = false;
  bool register_save_load = false;

public:
  int random_num = 0;

  PyCppBridge(py::object* py_reduction_impl) : py_reduction_impl(new py::object(*py_reduction_impl))
  {
    this->register_finish_learn = py::extract<bool>(call_py_impl_method("_is_finish_example_implemented"));
    this->register_save_load = py::extract<bool>(call_py_impl_method("_is_save_load_implemented"));
  }

  ~PyCppBridge() {}

  bool ShouldRegisterFinishExample() { return this->register_finish_learn; }

  bool ShouldRegisterSaveLoad() { return this->register_save_load; }

  template <typename... Args>
  py::object call_py_impl_method(char const* method_name, Args&&... args)
  {
    try
    {
      return this->py_reduction_impl->attr(method_name)(std::forward<Args>(args)...);
    }
    catch (...)
    {
      // TODO: Properly translate and return Python exception. #2169
      PyErr_Print();
      PyErr_Clear();
      THROW("Exception when calling into python method: " << method_name);
    }
  }

  // todo review if dont_delete_me is still needed with this refactor
  void ActualLearn(example* ec)
  {
    this->call_py_impl_method("_learn_convenience", example_ptr(ec, dont_delete_me),
        py_cpp_callback_ptr(new PyCppCallback(base_learner), dont_delete_me));
  }

  void ActualPredict(example* ec)
  {
    this->call_py_impl_method("_predict_convenience", example_ptr(ec, dont_delete_me),
        py_cpp_callback_ptr(new PyCppCallback(base_learner), dont_delete_me));
  }

  void ActualFinishExample(example* ec)
  {
    this->call_py_impl_method("_finish_example", example_ptr(ec, dont_delete_me));
  }

  ex_list multi_ex_to_boost(multi_ex* examples)
  {
    ex_list list;
    for (auto ec : *examples) { list.emplace_back(ec, dont_delete_me); }

    return list;
  }

  void ActualLearn(multi_ex* examples)
  {
    ex_list list = multi_ex_to_boost(examples);
    this->call_py_impl_method(
        "_learn_convenience", list, py_cpp_callback_ptr(new PyCppCallback(base_learner, examples), dont_delete_me));
  }

  void ActualPredict(multi_ex* examples)
  {
    ex_list list = multi_ex_to_boost(examples);
    this->call_py_impl_method(
        "_predict_convenience", list, py_cpp_callback_ptr(new PyCppCallback(base_learner, examples), dont_delete_me));
  }

  void ActualFinishExample(multi_ex* examples)
  {
    ex_list list = multi_ex_to_boost(examples);
    this->call_py_impl_method("_finish_example", list);
  }

  void ActualSaveLoad(io_buf* model_file, bool read, bool text)
  {
    this->call_py_impl_method("_save_load_convenience", read, text, py_cpp_callback_ptr(new PyCppCallback(model_file)));
  }

  void SetBaseLearner(void* learner) { this->base_learner = learner; }
};

// specialization needed to compile, this should never be reached since we always use
// VW::config::supported_options_types
template <>
py::object OptionManager::base_option_to_pyobject<VW::config::typelist<>>(VW::config::base_option& options)
{
  return py::object();
}

struct custom_builder_with_binding : VW::default_reduction_stack_setup
{
  std::unique_ptr<WRAPPER::ExternalBinding> instance;

  bool should_call_custom_python_setup = false;
  std::string call_after_executing;

  // decide here if its base, multi or single
  custom_builder_with_binding(std::unique_ptr<WRAPPER::ExternalBinding> _instance)
  {
    instance = std::move(_instance);
    call_after_executing = "extra_metrics";
  }

  VW::LEARNER::base_learner* setup_base_learner() override
  {
    if (instance)
    {
      if (should_call_custom_python_setup)
      {
        assert(instance != nullptr);
        should_call_custom_python_setup = false;
        return wrapper_reduction_setup(*this, std::move(instance));
      }

      if (std::get<0>(reduction_stack.back()).compare(call_after_executing) == 0)
      { should_call_custom_python_setup = true; }
    }

    if (reduction_stack.size() == 1)
    {
      return VW::default_reduction_stack_setup::setup_base_learner();
      // THROW("about to install GD");
    }
    else
    {
      return VW::default_reduction_stack_setup::setup_base_learner();
    }
  }
};