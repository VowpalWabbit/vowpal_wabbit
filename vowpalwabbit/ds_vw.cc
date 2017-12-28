#include "ds_vw.h"
#include "vw.h"
#include "parse_example_json.h"

namespace ds {
  VowpalWabbitModel::VowpalWabbitModel(vw* vw)
    : _vw(vw)
  { }

  VowpalWabbitModel::~VowpalWabbitModel()
  {
    if (_vw) {
      reset_source(*_vw, _vw->num_bits);
      release_parser_datastructures(*_vw);

      VW::finish(*_vw);

      _vw = nullptr;
    }
  }

  vw* VowpalWabbitModel::model() { return _vw; }

  // VowpalWabbitFactory

  VowpalWabbitFactory::VowpalWabbitFactory(std::shared_ptr<VowpalWabbitModel> vw_model)
    : _vw_model(vw_model)
  { }

  VowpalWabbit* VowpalWabbitFactory::operator()() {
    auto child_vw = VW::seed_vw_model(_vw_model->model(), "", nullptr, nullptr);
    return new VowpalWabbit(_vw_model, child_vw);
  }

  // VowpalWabbit

  VowpalWabbit::VowpalWabbit(std::shared_ptr<VowpalWabbitModel> model, vw* vw)
    : _model(model), _vw(vw)
  {
    // create an empty example
    _empty_example = VW::alloc_examples(0, 1);
    _vw->p->lp.default_label(&_empty_example->l);

    char empty = '\0';
    VW::read_line(*_vw, _empty_example, &empty);

    VW::setup_example(*_vw, _empty_example);
  }

  VowpalWabbit::~VowpalWabbit() {
    // cleanup examples
    for (auto&& ex : _example_pool) {
      VW::dealloc_example(_vw->p->lp.delete_label, *ex);
      ::free_it(ex);
    }

    // empty example
    VW::dealloc_example(_vw->p->lp.delete_label, *_empty_example);
    ::free_it(_empty_example);

    // cleanup VW instance
    reset_source(*_vw, _vw->num_bits);
    release_parser_datastructures(*_vw);

    VW::finish(*_vw);
  }

  example* VowpalWabbit::get_or_create_example() {
    // alloc new element if we don't have any left
    if (_example_pool.size() == 0) {
      auto ex = VW::alloc_examples(0, 1);
      _vw->p->lp.default_label(&ex->l);

      return ex;
    }

    // get last element
    example* ex = _example_pool.back();
    _example_pool.pop_back();

    VW::empty_example(*_vw, *ex);
    _vw->p->lp.default_label(&ex->l);

    return ex;
  }

  example& get_example_from_pool(void* v)
  {
    return *((VowpalWabbit*)v)->get_or_create_example();
  }

  std::vector<ActionProbability> VowpalWabbit::rank(const char* context) {
    v_array<example*> examples = v_init<example*>();
    examples.push_back(get_or_create_example());

    // TODO: audit support
    VW::read_line_json<false>(*_vw, examples, (char*)context, get_example_from_pool, this);

    // finalize example
    VW::setup_examples(*_vw, examples);

    example* first_example = nullptr;
    // predict
    for (auto&& ex : examples) {
      // filter out empty line to avoid early termination
      if (!example_is_newline(*ex)) {
        if (!first_example)
          first_example = ex;

        _vw->l->predict(*ex);
        _vw->l->finish_example(*_vw, *ex);
      }
    }

    // send empty/new line example to signal end of multi
    _vw->l->predict(*_empty_example);
    _vw->l->finish_example(*_vw, *_empty_example);

    // prediction are in the first-example
    std::vector<ActionProbability> ranking;
    if (first_example)
      for (auto&& a_s : first_example->pred.a_s)
        ranking.push_back({ (int)a_s.action, a_s.score });

    // push examples back into pool for re-use
    for (auto&& ex : examples)
      _example_pool.push_back(ex);

    // cleanup
    examples.delete_v();

    return ranking;
  }

  VowpalWabbitThreadSafe::VowpalWabbitThreadSafe() {
  }

  VowpalWabbitThreadSafe::~VowpalWabbitThreadSafe() {
  }

  std::vector<ActionProbability> VowpalWabbitThreadSafe::rank(const char* context) {
    PooledObjectGuard<VowpalWabbit, VowpalWabbitFactory> guard(pool, pool.get_or_create());

    return guard.obj()->rank(context);
  }
}
