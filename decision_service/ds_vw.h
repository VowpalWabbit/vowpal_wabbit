#pragma once

#include "ds_internal.h"
#include <vector>

struct vw;
struct example;

namespace Microsoft {
  namespace DecisionService {
    class VowpalWabbitModel {
      vw* _vw;
    public:
      VowpalWabbitModel(vw* vw);

      ~VowpalWabbitModel();

      inline vw* model();
    };

    class VowpalWabbit {
      std::shared_ptr<VowpalWabbitModel> _model;
      vw* _vw;
      std::vector<example*> _example_pool;
      example* _empty_example;

    public:
      VowpalWabbit(std::shared_ptr<VowpalWabbitModel> model, vw* vw);
      ~VowpalWabbit();

      std::vector<ActionProbability> rank(const char* context);

      example* get_or_create_example();
    };

    class VowpalWabbitFactory {
      std::shared_ptr<VowpalWabbitModel> _vw_model;

    public:
      VowpalWabbitFactory(std::shared_ptr<VowpalWabbitModel> vw_model);

      VowpalWabbit* operator()();
    };

    class VowpalWabbitThreadSafe : public IRanker {
      ObjectPool<VowpalWabbit, VowpalWabbitFactory> pool;
    public:
      VowpalWabbitThreadSafe();
      virtual ~VowpalWabbitThreadSafe();

      virtual std::vector<ActionProbability> rank(const char* context);
    };
  }
}
