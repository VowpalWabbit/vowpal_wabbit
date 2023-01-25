#pragma once

#include "../base.h"

namespace pseudo_vw
{
  template <class T, class E>
  class learner;

  namespace VW
  {
    namespace LEARNER
    {
      template <class T, class E>
      using learner = pseudo_vw::learner<T, E>;

      using base_learner = learner<char, char>;
      using single_learner = learner<char, example>;
      using multi_learner = learner<char, multi_ex>;
    }
  }

  class learn_data
  {
  public:
    using fn = void (*)(void* data, VW::LEARNER::base_learner& base, void* ex);
    using multi_fn = void (*)(void* data, VW::LEARNER::base_learner& base, void* ex, size_t count, size_t step, polyprediction* pred,
        bool finalize_predictions);

    void* data = nullptr;
    VW::LEARNER::base_learner* base = nullptr;
    fn learn_f = nullptr;
    fn predict_f = nullptr;
    fn update_f = nullptr;
    multi_fn multipredict_f = nullptr;
  };

class sensitivity_data
{
public:
  using fn = float (*)(void* data, VW::LEARNER::base_learner& base, example& ex);
  void* data = nullptr;
  fn sensitivity_f = nullptr;
};

// struct io_buf{};
// class save_load_data
// {
// public:
//   using fn = void (*)(void*, io_buf&, bool read, bool text);
//   void* data = nullptr;
//   VW::LEARNER::base_learner* base = nullptr;
//   fn save_load_f = nullptr;
// };

// struct metric_sink{};

// class save_metric_data
// {
// public:
//   using fn = void (*)(void*, metric_sink& metrics);
//   void* data = nullptr;
//   VW::LEARNER::base_learner* base = nullptr;
//   fn save_metric_f = nullptr;
// };

// class workspace;

// class finish_example_data
// {
// public:
//   using fn = void (*)(workspace&, void* data, void* ex);
//   void* data = nullptr;
//   VW::LEARNER::base_learner* base = nullptr;
//   fn finish_example_f = nullptr;
//   fn print_example_f = nullptr;
// };

}