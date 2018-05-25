#include "safe_vw.h"
#include "example.h"
#include "parse_example_json.h"

namespace reinforcement_learning {

  class in_memory_buf : public io_buf
  {
  private:
    const char* _model_data;
    const char* _model_data_end;
    const char* _current;

  public:
    in_memory_buf(const char* model_data, size_t len)
      : _model_data(model_data), _model_data_end(model_data + len), _current(model_data)
    {
      files.push_back(0);
    }

    virtual int open_file(const char* name, bool stdin_off, int flag = READ)
    {
      _current = _model_data;
      return 0;
    }

    virtual void reset_file(int f)
    {
      _current = _model_data;
    }

    virtual ssize_t read_file(int f, void* buf, size_t nbytes)
    {
      size_t left_over = min(nbytes, (size_t)(_model_data_end - _current));

      if (left_over == 0)
        return 0;

#ifdef WIN32
      memcpy_s(buf, nbytes, &*_current, left_over);
#else
      memcpy(buf, &*_current, left_over);
#endif

      _current += left_over;

      return left_over;
    }

    virtual size_t num_files() { return 1; }

    virtual ssize_t write_file(int file, const void* buf, size_t nbytes) { return -1; }

    virtual bool compressed() { return false; }

    virtual bool close_file() { return true; }
  };


  safe_vw::safe_vw(vw* vw) : _vw(vw)
  { }

  safe_vw::safe_vw(const char* model_data, size_t len)
  {
    in_memory_buf buf(model_data, len);

    _vw = VW::initialize("--quiet --json", &buf, false, nullptr, nullptr);
  }

  safe_vw::~safe_vw()
  {
    // cleanup examples
    for (auto&& ex : _example_pool) {
      VW::dealloc_example(_vw->p->lp.delete_label, *ex);
      ::free_it(ex);
    }

    // cleanup VW instance
    reset_source(*_vw, _vw->num_bits);
    release_parser_datastructures(*_vw);

    VW::finish(*_vw);
  }

  example* safe_vw::get_or_create_example()
  {
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

  example& safe_vw::get_or_create_example_f(void* vw) { return *(((safe_vw*)vw)->get_or_create_example()); }

  std::vector<float> safe_vw::rank(const char* context)
  {
    v_array<example*> examples = v_init<example*>();
    examples.push_back(get_or_create_example());

    std::vector<char> line_vec(context, context + strlen(context) + 1);

    VW::read_line_json<false>(*_vw, examples, &line_vec[0], get_or_create_example_f, this);

    // finalize example
    VW::setup_examples(*_vw, examples);

    // TODO: refactor setup_examples/read_line_json to take in multi_ex
    multi_ex examples2(examples.begin(), examples.end());

    _vw->predict(examples2);

    // prediction are in the first-example
    std::vector<float> ranking;
    ranking.resize(examples2[0]->pred.a_s.size());
    for (auto&& a_s : examples2[0]->pred.a_s)
      ranking[a_s.action] = a_s.score;

    // push examples back into pool for re-use
    for (auto&& ex : examples)
      _example_pool.push_back(ex);

    // cleanup
    examples.delete_v();

    return ranking;
  }

  safe_vw_factory::safe_vw_factory(safe_vw* master) : _master(master)
  { }

  safe_vw* safe_vw_factory::operator()()
  {
    return new safe_vw(VW::seed_vw_model(_master->_vw, "", nullptr, nullptr));
  }
}
