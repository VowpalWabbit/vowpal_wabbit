#include "prelude.h"
#include "kernel_svm.h"
#include "typesys.h"
#include "reductions.h"
#include "autofb.h"

#define enum(name) enum name : unsigned int

// TODO: more reasonable type (so we can bind it)
enum(kernel_type_t)
{
  SVM_KER_LINEAR = 0,
  SVM_KER_RBF    = 1,
  SVM_KER_POLY   = 2
};
  // can we somehow automatically generate the
  //   string <=> enum conversion?
  // we would need to get further away from C++ lang

data(svm_params_hyper)
{
  _(std::string, kernel_type);
  _(float, bandwidth);
  _(int, degree);
  _(bool, para_active);
  _(bool, active_pool_greedy);
  _(uint64_t, pool_size); // these are not .keep()ed - why?
  _(uint64_t, subsample);
  _(uint64_t, reprocess);
};

data(svm_params_predict)
{
  _(int32_t, num_support)
  _(std::string, placeholder_support_data)
};

data(svm_params_train)
{
  // this is an empty type?
};

struct svn_params_scratch
{

};

using kernel_svm_persisted = reduction_data<svm_params_hyper, 
                                            svm_params_predict, 
                                            svm_params_train>;

struct svn_params : 
  kernel_svm_persisted, 
  svn_params_scratch
{
};



void kernel_svm_test()
{
  std::cout << "kernel_svm test: Generate schemas for kernel_svm types" << std::endl;

  const std::string schema_dir = "C:\\s\\vw\\vw_proto\\prototype\\autofb\\schemas\\"; 
  const std::string schema_ns = "autofb_proto";

  autofb::print_universe_types(schema_ns);
  autofb::generate_universe_types(schema_ns, schema_dir);
}

template <typename T>
void erased_deleter(void* p)
{
  delete static_cast<T*>(p);
}

template <typename T>
void erased_vdeleter(void* p)
{
  delete [] static_cast<T*>(p);
}

erased_data make_erased_data()
{
  svm_params_hyper* hyper = new svm_params_hyper();
  hyper->kernel_type = "rbf";
  hyper->bandwidth = 0.5f;
  hyper->degree = 3;
  hyper->para_active = false;
  hyper->active_pool_greedy = false;
  hyper->pool_size = 100;
  hyper->subsample = 240;
  hyper->reprocess = 1; 

  return erased_data{activation(std::move(hyper), &erased_deleter<svm_params_hyper>), type<svm_params_hyper>::erase()};
}