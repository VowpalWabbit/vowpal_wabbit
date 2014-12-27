/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
// This is the interface for a learning algorithm
#include<iostream>
#include"memory.h"
using namespace std;

struct vw;
void return_simple_example(vw& all, void*, example& ec);  
  
namespace LEARNER
{
  template<class T> struct learner;
  typedef learner<char> base_learner;
  
  struct func_data {
    void* data;
    base_learner* base;
    void (*func)(void* data);
  };
  
  inline func_data tuple_dbf(void* data, base_learner* base, void (*func)(void* data))
  {
    func_data foo;
    foo.data = data;
    foo.base = base;
    foo.func = func;
    return foo;
  }
  
  struct learn_data {
    void* data;
    base_learner* base;
    void (*learn_f)(void* data, base_learner& base, example&);
    void (*predict_f)(void* data, base_learner& base, example&);
    void (*update_f)(void* data, base_learner& base, example&);
  };

  struct save_load_data{
    void* data;
    base_learner* base;
    void (*save_load_f)(void*, io_buf&, bool read, bool text);
  };
  
  struct finish_example_data{
    void* data;
    base_learner* base;
    void (*finish_example_f)(vw&, void* data, example&);
  };
  
  void generic_driver(vw& all);
  
  inline void generic_sl(void*, io_buf&, bool, bool) {}
  inline void generic_learner(void* data, base_learner& base, example&) {}
  inline void generic_func(void* data) {}

  const save_load_data generic_save_load_fd = {NULL, NULL, generic_sl};
  const learn_data generic_learn_fd = {NULL, NULL, generic_learner, generic_learner, NULL};
  const func_data generic_func_fd = {NULL, NULL, generic_func};
  
  typedef void (*tlearn)(void* d, base_learner& base, example& ec);
  typedef void (*tsl)(void* d, io_buf& io, bool read, bool text);
  typedef void (*tfunc)(void*d);
  typedef void (*tend_example)(vw& all, void* d, example& ec);

  template<class T> learner<T>& init_learner();
  template<class T> learner<T>& init_learner(T* dat, size_t params_per_weight);
  template<class T> learner<T>& init_learner(T* dat, base_learner* base, size_t ws = 1);
  
  template<class T>
    struct learner {
    private:
      func_data init_fd;
      learn_data learn_fd;
      finish_example_data finish_example_fd;
      save_load_data save_load_fd;
      func_data end_pass_fd;
      func_data end_examples_fd;
      func_data finisher_fd;
      
    public:
      size_t weights; //this stores the number of "weight vectors" required by the learner.
      size_t increment;
      
      //called once for each example.  Must work under reduction.
      inline void learn(example& ec, size_t i=0) 
      { 
	ec.ft_offset += (uint32_t)(increment*i);
	learn_fd.learn_f(learn_fd.data, *learn_fd.base, ec);
	ec.ft_offset -= (uint32_t)(increment*i);
      }
      inline void set_learn(void (*u)(T& data, base_learner& base, example&))
      {
	learn_fd.learn_f = (tlearn)u;
	learn_fd.update_f = (tlearn)u;
      }
      
      inline void predict(example& ec, size_t i=0) 
      { 
	ec.ft_offset += (uint32_t)(increment*i);
	learn_fd.predict_f(learn_fd.data, *learn_fd.base, ec);
	ec.ft_offset -= (uint32_t)(increment*i);
      }
      inline void set_predict(void (*u)(T& data, base_learner& base, example&))
      { learn_fd.predict_f = (tlearn)u; }
      
      inline void update(example& ec, size_t i=0) 
      { 
	ec.ft_offset += (uint32_t)(increment*i);
	learn_fd.update_f(learn_fd.data, *learn_fd.base, ec);
	ec.ft_offset -= (uint32_t)(increment*i);
      }
      inline void set_update(void (*u)(T& data, base_learner& base, example&))
      { learn_fd.update_f = (tlearn)u; }
      
      //called anytime saving or loading needs to happen. Autorecursive.
      inline void save_load(io_buf& io, bool read, bool text) { save_load_fd.save_load_f(save_load_fd.data, io, read, text); if (save_load_fd.base) save_load_fd.base->save_load(io, read, text); }
      inline void set_save_load(void (*sl)(T&, io_buf&, bool, bool))
      { save_load_fd.save_load_f = (tsl)sl; 
	save_load_fd.data = learn_fd.data; 
	save_load_fd.base = learn_fd.base;}
      
      //called to clean up state.  Autorecursive.
      void set_finish(void (*f)(T&)) { finisher_fd = tuple_dbf(learn_fd.data,learn_fd.base, (tfunc)f); }
      inline void finish() 
      { 
	if (finisher_fd.data) 
	  {finisher_fd.func(finisher_fd.data); free(finisher_fd.data); } 
	if (finisher_fd.base) { 
	  finisher_fd.base->finish();
	  delete finisher_fd.base;
	}
      }
      
      void end_pass(){ 
	end_pass_fd.func(end_pass_fd.data);
	if (end_pass_fd.base) end_pass_fd.base->end_pass(); }//autorecursive
      void set_end_pass(void (*f)(T&)) 
      {end_pass_fd = tuple_dbf(learn_fd.data, learn_fd.base, (tfunc)f);}
      
      //called after parsing of examples is complete.  Autorecursive.
      void end_examples() 
      { end_examples_fd.func(end_examples_fd.data); 
	if (end_examples_fd.base) end_examples_fd.base->end_examples(); }  
      void set_end_examples(void (*f)(T&)) 
      {end_examples_fd = tuple_dbf(learn_fd.data,learn_fd.base, (tfunc)f);}
      
      //Called at the beginning by the driver.  Explicitly not recursive.
      void init_driver() { init_fd.func(init_fd.data);}
      void set_init_driver(void (*f)(T&)) 
      { init_fd = tuple_dbf(learn_fd.data,learn_fd.base, (tfunc)f); }
      
      //called after learn example for each example.  Explicitly not recursive.
      inline void finish_example(vw& all, example& ec) { finish_example_fd.finish_example_f(all, finish_example_fd.data, ec);}
      void set_finish_example(void (*f)(vw& all, T&, example&))
      {finish_example_fd.data = learn_fd.data;
	finish_example_fd.finish_example_f = (tend_example)f;}
      
      friend learner<T>& init_learner<>();
      friend learner<T>& init_learner<>(T* dat, size_t params_per_weight);
      friend learner<T>& init_learner<>(T* dat, base_learner* base, size_t ws);
    };
  
  template<class T> learner<T>& init_learner()
    {
      learner<T>& ret = calloc_or_die<learner<T> >();
      ret.weights = 1;
      ret.increment = 1;
      ret.learn_fd = LEARNER::generic_learn_fd;
      ret.finish_example_fd.data = NULL;
      ret.finish_example_fd.finish_example_f = return_simple_example;
      ret.end_pass_fd = LEARNER::generic_func_fd;
      ret.end_examples_fd = LEARNER::generic_func_fd;
      ret.init_fd = LEARNER::generic_func_fd;
      ret.finisher_fd = LEARNER::generic_func_fd;
      ret.save_load_fd = LEARNER::generic_save_load_fd;
      return ret;
    }
  
  template<class T> learner<T>& init_learner(T* dat, size_t params_per_weight)
    { // the constructor for all learning algorithms.
      learner<T>& ret = init_learner<T>();
      
      ret.learn_fd.data = dat;
      
      ret.finisher_fd.data = dat;
      ret.finisher_fd.base = NULL;
      ret.finisher_fd.func = LEARNER::generic_func;
      
      ret.increment = params_per_weight;
      return ret;
    }
  
  template<class T> learner<T>& init_learner(T* dat, base_learner* base, size_t ws = 1) 
    { //the reduction constructor, with separate learn and predict functions
      learner<T>& ret = calloc_or_die<learner<T> >();
      ret = *(learner<T>*)base;
      
      ret.learn_fd.data = dat;
      ret.learn_fd.base = base;
      
      ret.finisher_fd.data = dat;
      ret.finisher_fd.base = base;
      ret.finisher_fd.func = LEARNER::generic_func;
      
      ret.weights = ws;
      ret.increment = base->increment * ret.weights;
      return ret;
    }
  
  template<class T> base_learner* make_base(learner<T>* base)
    { return (base_learner*)base; }
}
