#pragma once

#include <vector>
#include "vw.h"

namespace reinforcement_learning {

  class safe_vw {
    vw* _vw;
    std::vector<example*> _example_pool;

    example* get_or_create_example();
    static example& get_or_create_example_f(void* vw);

  public:
    safe_vw(vw* vw);
    safe_vw(const char* model_data, size_t len);

    ~safe_vw();

    std::vector<float> rank(const char* context);

    friend class safe_vw_factory;
  };


  class safe_vw_factory {
    std::unique_ptr<safe_vw> _master;

  public:
    safe_vw_factory(safe_vw* master);

    safe_vw* operator()();
  };
}
