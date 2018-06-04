#pragma once

#include <vector>
#include "vw.h"

namespace reinforcement_learning {

  class safe_vw {
    // we need to keep a reference to the master around, so it's still valid even if the factory is deleted
    std::shared_ptr<safe_vw> _master;
    vw* _vw;
    std::vector<example*> _example_pool;

    example* get_or_create_example();
    static example& get_or_create_example_f(void* vw);

  public:
    safe_vw(const std::shared_ptr<safe_vw>& master);
    safe_vw(const char* model_data, size_t len);

    ~safe_vw();

    std::vector<float> rank(const char* context);

    friend class safe_vw_factory;
  };

  class safe_vw_factory {
    std::shared_ptr<safe_vw> _master;

  public:
    safe_vw_factory(const std::shared_ptr<safe_vw>& master);

    safe_vw* operator()();
  };
}
