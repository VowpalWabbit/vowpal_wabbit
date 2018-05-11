#pragma once
#include <mutex>

namespace reinforcement_learning {
  class api_status;

  class error_callback_fn
  {
    public:
      using error_fn = void(*)(const api_status&, void*);
      void set(error_fn, void*);
      void report_error(api_status& s);

    public:
      error_callback_fn(error_fn, void*);
      ~error_callback_fn() = default;

    private:
      error_callback_fn(const error_callback_fn&) = default;
      error_callback_fn(error_callback_fn&&) = default;
      error_callback_fn& operator=(const error_callback_fn&) = default;
      error_callback_fn& operator=(error_callback_fn&&) = default;
      
      std::mutex _mutex;
      error_fn _fn;
      void* _context;
  };
}
