#pragma once

#include <vector>
#include <memory>
#include "../../vowpalwabbit/vw.h"
#include "model_mgmt.h"

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

    void rank(const char* context, std::vector<int>& actions, std::vector<float>& scores);
    const char* id() const;

    friend class safe_vw_factory;
  };

  class safe_vw_factory {
    model_management::model_data _master_data;

  public:
    // model_data is copied and stored in the factory object.
    safe_vw_factory(const model_management::model_data& master_data);
    safe_vw_factory(const model_management::model_data&& master_data);

    safe_vw* operator()();
  };
}
