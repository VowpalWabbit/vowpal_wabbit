#pragma once
#include <boost/program_options.hpp>
#include <type_traits>
#include "vw_exception.h"
#include "error_reporting.h"
#include<iostream>
namespace po = boost::program_options;

class vw;

class arguments {
  po::options_description new_od;//a set of options
  po::variables_map add_options_skip_duplicates(po::options_description& opts, bool do_notify);
  uint32_t critical_count;

 public:
  po::options_description all_opts; //All specified options.
  po::options_description opts; //Critical options and their dependencies.
  vw_ostream trace_message;//error reporting
  std::stringstream* file_options; // the set of options to store in the model file.
  po::variables_map vm; //A stored map from option to value.
  std::vector<std::string> args;//All arguments
  vw* all;//backdoor that should go away over time.

  //initialization
 arguments(vw& all_in, std::string name_in=""):new_od(name_in), critical_count(0), all(&all_in) {file_options = new std::stringstream;};
 arguments():critical_count(0){};//this should not be used but appears sometimes unavoidable.  Do an in-place allocation with the upper initializer after it is used.

  //reinitialization
  arguments& new_options(std::string name_in="")
  {
    (&new_od)->~options_description();//in place delete
    new (&new_od) po::options_description(name_in);
    critical_count=0;
    return *this;
  }

  //insert arguments
  arguments& operator()(const char* option, const char* description)
    {
      new_od.add_options()(option, description);
      return *this;
    }
  arguments& operator()(bool& exists, const char* option, const char* description)
    { return operator()(option, po::bool_switch(&exists), description); }
  template<class T> arguments& operator()(const char* option, T& location, const char* description)
    { return operator()(option, po::value(&location), description); }
  template<class T> arguments& operator()(const char* option, T& location, T def, const char* description)
    { return operator()(option, po::value(&location)->default_value(def), description); }
  arguments& operator()(const char* option, const po::value_semantic* type, const char* description)
    {
      new_od.add_options()(option, type, description);
      return *this;
    }
  //A keep option is kept in the model.
  template<class T> arguments& keep(const char* option, T& store, const char* description)
    { return keep<T>(option, po::value(&store), description); }
  template<class T> arguments& keep(const char* option, T& store, T def, const char* description)
    {
      return operator()(option,
                        po::value(&store)->default_value(def)
                        ->notifier([this, option, def] (T arg)
                                   {
                                     if (arg != def)
                                       *this->file_options << " --" << option << " " << arg;
                                   }),
                        description);
    }
  template<class T> arguments& keep(const char* option, po::typed_value<T>* type, const char* description)
    {
      return operator()(option,
                        type->notifier([this, option] (T arg)
                                       { *this->file_options << " --" << option << " " << arg; }),
                        description);
    }
  template<class T> arguments& keep_vector(const char* option, po::typed_value<std::vector<T>>* type, const char* description)
    {
      return operator()(option,
                        type->notifier([this, option] (std::vector<T> arg)
                                       {
                                         for (auto i : arg)
                                           *this->file_options << " --" << option << " " << i;
                                       }),
                        description);
    }
  arguments& keep(bool& exists, const char* option, const char* description)
    {
      return operator()(option,
                        po::bool_switch(&exists)
                        ->notifier([this, option] (bool v)
                                   { if (v) *this->file_options << " --" << option; }),
                        description);
    }
  arguments& keep(const char* option, const char* description)
    {
      bool temp=false;
      return keep(temp, option, description);
    }

  //A missing critical argument raises the missing flag.  Critical implies keep.
  template<class T> arguments& critical(const char* option, T& store, const char* description)
    { return critical<T>(option, po::value<T>(&store), description); }
  template<class T> arguments& critical(const char* option, po::typed_value<T>* type, const char* description)
    {
      critical_count++;
      operator()(option, type->notifier([this, option] (T arg)
                                        {
                                          critical_count--;
                                          *this->file_options << " --" << option << " " << arg;
                                        }), description);
      if (missing())
        {
          new_options();
          critical_count=1;        
        }
      else
        new_options();
      return *this;
    }
  template<class T> arguments& critical_vector(const char* option, po::typed_value<std::vector<T>>* type, const char* description)
    {
      critical_count++;
      operator()(option, type->notifier([this, option] (std::vector<T> arg)
                                        {
                                          critical_count--;
                                          for (auto i: arg)
                                            *this->file_options << " --" << option << " " << i;
                                        }), description);
      if (missing())
        {
          new_options();
          critical_count=1;        
        }
      else
        new_options();
      return *this;
    }
  template<class T> arguments& critical(const char* option, const char* description)
    { return critical<T>(option, po::value<T>(), description); }
  arguments& critical(const char* option, const char* description)
    {
      operator()(option,
                 po::bool_switch()->notifier([this, option] (bool temp)
                                             {
                                               if (temp)
                                                 *this->file_options << " --" << option;
                                               else
                                                 ++this->critical_count;
                                             }),
                 description);
      if (missing())
        {
          new_options();
          critical_count=1;        
        }
      else
        new_options();
      return *this;
    }

  bool missing()  //Return true if key options are missing.
  {
    all_opts.add(new_od);
    if (critical_count == 0)
      {
        opts.add(new_od);    //compile options
        auto new_vm = add_options_skip_duplicates(new_od, true);//do notify
        for (auto& it : new_vm)
          vm.insert(it);
      }
    return critical_count > 0;
  }
};
