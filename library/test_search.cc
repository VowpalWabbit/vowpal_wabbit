#include "libsearch.h"
#include "vw/config/options_cli.h"
#include "vw/core/memory.h"
#include "vw/core/reductions/search/search_sequencetask.h"
#include "vw/core/vw.h"

#include <cstdio>
#include <cstdlib>  // for system
#include <utility>

using std::cerr;
using std::endl;

struct wt
{
  std::string word;
  uint32_t tag;
  wt(std::string w, uint32_t t) : word(std::move(w)), tag(t) {}
};

class SequenceLabelerTask : public SearchTask<std::vector<wt>, std::vector<uint32_t> >  // NOLINT
{
public:
  SequenceLabelerTask(VW::workspace& vw_obj)
      : SearchTask<std::vector<wt>, std::vector<uint32_t> >(vw_obj)  // must run parent constructor!
  {
    sch.set_options(Search::AUTO_HAMMING_LOSS | Search::AUTO_CONDITION_FEATURES);
    HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    cerr << "num_actions = " << d->num_actions << endl;
  }

  // using vanilla vw interface
  void _run(Search::search& sch, std::vector<wt>& input_example, std::vector<uint32_t>& output)
  {
    output.clear();
    // ptag currently uint32_t
    for (ptag i = 0; i < input_example.size(); i++)
    {
      VW::example* ex = VW::read_example(vw_obj, std::string("1 |w ") + input_example[i].word);
      action p =
          Search::predictor(sch, i + 1).set_input(*ex).set_oracle(input_example[i].tag).set_condition(i, 'p').predict();
      VW::finish_example(vw_obj, *ex);
      output.push_back(p);
    }
  }

  void _run2(Search::search& sch, std::vector<wt>& input_example, std::vector<uint32_t>& output)  // NOLINT
  {
    auto& vw_obj = sch.get_vw_pointer_unsafe();
    output.clear();
    // ptag currently uint32_t
    for (ptag i = 0; i < input_example.size(); i++)
    {
      VW::example ex;
      auto ns_hash_w = VW::hash_space(vw_obj, "w");
      auto& fs_w = ex.feature_space['w'];
      ex.indices.push_back('w');
      fs_w.push_back(1.f, VW::hash_feature(vw_obj, input_example[i].word, ns_hash_w));
      VW::setup_example(vw_obj, &ex);
      action p =
          Search::predictor(sch, i + 1).set_input(ex).set_oracle(input_example[i].tag).set_condition(i, 'p').predict();
      output.push_back(p);
      VW::finish_example(vw_obj, ex);
    }
  }
};

void run(VW::workspace& vw_obj)
{
  // we put this in its own scope so that its destructor on
  // SequenceLabelerTask gets called *before* VW::finish gets called;
  // otherwise we'll get a segfault :(. i'm not sure what to do about
  // this :(.
  SequenceLabelerTask task(vw_obj);
  std::vector<wt> data;
  std::vector<uint32_t> output;
  static constexpr const uint32_t DET = 1;
  static constexpr const uint32_t NOUN = 2;
  static constexpr const uint32_t VERB = 3;
  static constexpr const uint32_t ADJ = 4;
  data.push_back(wt("the", DET));
  data.push_back(wt("monster", NOUN));
  data.push_back(wt("ate", VERB));
  data.push_back(wt("a", DET));
  data.push_back(wt("big", ADJ));
  data.push_back(wt("sandwich", NOUN));
  task.learn(data, output);
  task.learn(data, output);
  task.learn(data, output);
  task.predict(data, output);
  cerr << "output = [";
  for (size_t i = 0; i < output.size(); i++) { cerr << " " << output[i]; }
  cerr << " ]" << endl;
  cerr << "should have printed: 1 2 3 1 4 2" << endl;
}

void train()
{
  // initialize VW as usual, but use 'hook' as the search_task
  cerr << endl << endl << "##### train() #####" << endl << endl;
  auto vw_obj = VW::initialize(VW::make_unique<VW::config::options_cli>(std::vector<std::string>{
      "--search", "4", "--quiet", "--search_task", "hook", "--example_queue_limit", "1024", "-f", "my_model"}));
  run(*vw_obj);
  vw_obj->finish();
}

void predict()
{
  cerr << endl << endl << "##### predict() #####" << endl << endl;
  auto vw_obj = VW::initialize(VW::make_unique<VW::config::options_cli>(
      std::vector<std::string>{"--quiet", "-t", "--example_queue_limit", "1024", "-i", "my_model"}));
  run(*vw_obj);
  vw_obj->finish();
}

void test_buildin_task()
{
  cerr << endl << endl << "##### run commandline vw #####" << endl << endl;
  // train a model on the command line
  int ret = system(
      "../vowpalwabbit/vw -c -k --holdout_off --passes 20 --search 4 --search_task sequence -d "
      "../../test/train-sets/sequence_data -f "
      "sequence.model");
  if (ret != 0) { cerr << "../vowpalwabbit/vw failed" << endl; }

  // now, load that model using the BuiltInTask library
  cerr << endl << endl << "##### test BuiltInTask #####" << endl << endl;

  auto vw_obj =
      VW::initialize(VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"-t", "--search_task", "hook"}));
  {  // create a new scope for the task object
    BuiltInTask task(*vw_obj, &SequenceTask::task);
    VW::multi_ex mult_ex;
    mult_ex.push_back(VW::read_example(*vw_obj, (char*)"1 | a"));
    mult_ex.push_back(VW::read_example(*vw_obj, (char*)"1 | a"));
    mult_ex.push_back(VW::read_example(*vw_obj, (char*)"1 | a"));
    mult_ex.push_back(VW::read_example(*vw_obj, (char*)"1 | a"));
    mult_ex.push_back(VW::read_example(*vw_obj, (char*)"1 | a"));
    std::vector<action> out;
    task.predict(mult_ex, out);
    cerr << "out (should be 1 2 3 4 3) =";
    for (size_t i = 0; i < out.size(); i++) { cerr << " " << out[i]; }
    cerr << endl;
    for (size_t i = 0; i < mult_ex.size(); i++) { VW::finish_example(*vw_obj, *mult_ex[i]); }
  }

  vw_obj->finish();
}

int main(int argc, char* argv[])
{
  train();
  predict();
  test_buildin_task();
}
