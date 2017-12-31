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
  bool is_missing;

 public:
  po::options_description opts; //All specified options.
  vw_ostream trace_message;//error reporting
  std::stringstream* file_options; // the set of options to store in the model file.
  po::variables_map vm; //A stored map from option to value.
  std::vector<std::string> args;//All arguments
  vw* all;//backdoor that should go away over time.

  //initialization
 arguments(vw& all_in, std::string name_in=""):new_od(name_in), is_missing(false), all(&all_in) {file_options = new std::stringstream;};
 arguments():is_missing(0){};//this should not be used but appears sometimes unavoidable.  Do an in-place allocation with the upper initializer after it is used.

  //reinitialization
  arguments& new_options(std::string name_in="")
  {
    (&new_od)->~options_description();//in place delete
    new (&new_od) po::options_description(name_in);
    is_missing=false;
    return *this;
  }

  //insert arguments
  arguments& operator()(const char* option, const char* description)
    {
      new_od.add_options()(option, description);
      return *this;
    }
  arguments& operator()(bool& exists, const char* option, const char* description)
    {
      exists=false;
      po::options_description temp_od("");
      temp_od.add_options()(option, description);
      auto new_vm = add_options_skip_duplicates(temp_od, false);//do not notify
      if (new_vm.size() > 0)
        exists = true;
      return operator()(option, description);
    }
  template<class T> arguments& operator()(const char* option, T& location, const char* description)
    { return operator()(option, po::value(&location), description); }
  template<class T> arguments& operator()(const char* option, T& location, T def, const char* description)
    { return operator()(option, po::value(&location)->default_value(def), description); }
  arguments& operator()(const char* option, const po::value_semantic* type, const char* description)
    {
      new_od.add_options()(option, type, description);
      std::cout << "options after adding = " << new_od;
      return *this;
    }
  //A keep option is kept in the model.
  template<class T> arguments& keep(const char* option, T& store, const char* description)
    { return keep<T>(option, po::value(&store), description); }
  template<class T> arguments& keep(const char* option, T& store, T def, const char* description)
    { return keep<T>(option, po::value(&store)->default_value(def), description); }
  template<class T> arguments& keep(const char* option, const po::value_semantic* type, const char* description, bool critical=false, const char* error_message=nullptr)
    {
      po::options_description temp_od("");
      temp_od.add_options()(option, type, description);
      auto new_vm = add_options_skip_duplicates(temp_od, false);//do not notify
      if (new_vm.size() > 0)
        *file_options << " --" << option << " " << vm[option].as<T>();
      else if (critical)
        {
        is_missing=true;
        }
      return operator()(option, type, description);
    }
  template<class T> arguments& keep_vector(const char* option, const po::value_semantic* type, const char* description, bool critical=false)
    {
      return operator()(option, type, description);
    }
  arguments& keep(bool& exists, const char* option, const char* description, bool critical=false)
    {
      exists=false;
      po::options_description temp_od("");
      temp_od.add_options()(option, description);
      auto new_vm = add_options_skip_duplicates(temp_od, false);//do not notify
      if (new_vm.size() > 0)
        {
          exists = true;
          *file_options << " --" << option;
        }
      else if (critical)
        is_missing=true;
      return operator()(option, description);
    }
  arguments& keep(const char* option, const char* description, bool critical=false)
    {
      bool temp=false;
      return keep(temp, option, description, critical);
    }

  //A missing critical argument raises the missing flag.  Critical implies keep.
  template<class T> arguments& critical(const char* option, T& store, const char* description)
    { return critical<T>(option, po::value<T>(&store), description); }
  template<class T> arguments& critical(const char* option, T& store, const char* description, const char* error_message)
    { return keep<T>(option, po::value<T>(&store), description, true, error_message); }
  template<class T> arguments& critical(const char* option, T& store, T def, const char* description)
    { return keep<T>(option, po::value(&store)->default_value(def), description, true); }
  template<class T> arguments& critical(const char* option, const po::value_semantic* type, const char* description)
    { return keep<T>(option, type, description, true); }
  template<class T> arguments& critical_vector(const char* option, const po::value_semantic* type, const char* description)
    { return keep_vector<T>(option, type, description, true);}
  template<class T> arguments& critical(const char* option, const char* description)
    { return keep<T>(option, po::value<T>(), description, true); }
  arguments& critical(const char* option, const char* description)
    { return keep(option, description, true); }

  bool missing()  //Return true if key options are missing.
  {
    if(is_missing)
      return true;

    opts.add(new_od);    //compile options
    auto new_vm = add_options_skip_duplicates(new_od, true);//do notify
    for (auto& it : new_vm)
      vm.insert(it);
    return false;
  }
};
