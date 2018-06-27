#pragma once
#include "model_mgmt.h"
namespace reinforcement_learning { namespace model_management {

  class data_callback_fn {
  public:
    using data_fn = void(*)(const model_data& data, void*);
    int report_data(const model_data& data, api_status* status = nullptr);

    data_callback_fn(data_fn, void*);
    ~data_callback_fn() = default;

    data_callback_fn(const data_callback_fn&) = delete;
    data_callback_fn(data_callback_fn&&) = delete;
    data_callback_fn& operator=(const data_callback_fn&) = delete;
    data_callback_fn& operator=(data_callback_fn&&) = delete;

  private:
    data_fn _fn;
    void* _context;
  };
}}
