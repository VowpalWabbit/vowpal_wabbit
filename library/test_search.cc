#include <stdio.h>
#include "../vowpalwabbit/parser.h"
#include "../vowpalwabbit/vw.h"
#include "../vowpalwabbit/searn.h"
#include "../vowpalwabbit/searn_hooktask.h"

using namespace std;

template<class INPUT, class OUTPUT> class SearchTask {
  public:
  SearchTask(vw* all) : all(*all), srn(*(Searn::searn*)all->searnstr) {
    bogus_example = VW::read_example(*all, (char*)"1 | x");
    blank_line    = VW::read_example(*all, (char*)"");
    VW::finish_example(*all, blank_line);
    HookTask::task_data* d = srn.get_task_data<HookTask::task_data>();
    d->run_f = _searn_run_fn;
    d->run_object = this;
    d->var_map = NULL; // TODO
    //d->num_actions = num_actions;  // TODO
    d->extra_data  = NULL;
    d->extra_data2 = NULL;
  }
  ~SearchTask() { VW::finish_example(all, bogus_example); }

  virtual void _run(Searn::searn&srn, INPUT& input_example, OUTPUT& output) {}  // YOU MUST DEFINE THIS FUNCTION!

  void learn(INPUT& input_example, OUTPUT& output) {
    HookTask::task_data* d = srn.template get_task_data<HookTask::task_data> (); // ugly calling convention :(
    bogus_example->test_only = false;
    d->extra_data  = (void*)&input_example;
    d->extra_data2 = (void*)&output;
    all.learn(bogus_example);
    all.learn(blank_line);   // this will cause our searn_run_fn hook to get called
  }

  void predict(INPUT& input_example, OUTPUT& output) {
    HookTask::task_data* d = srn.template get_task_data<HookTask::task_data> (); // ugly calling convention :(
    bogus_example->test_only = true;
    d->extra_data  = (void*)&input_example;
    d->extra_data2 = (void*)&output;
    all.learn(bogus_example);
    all.learn(blank_line);   // this will cause our searn_run_fn hook to get called
  }
  

  protected:
  vw& all;
  Searn::searn& srn;
  
  private:
  example* bogus_example, *blank_line;

  static void _searn_run_fn(Searn::searn&srn) {
    HookTask::task_data* d = srn.get_task_data<HookTask::task_data>();
    if ((d->run_object == NULL) || (d->extra_data == NULL) || (d->extra_data2 == NULL)) {
      cerr << "error: calling _searn_run_fn without setting run object" << endl;
      throw exception();
    }
    ((SearchTask*)d->run_object)->_run(srn, *(INPUT*)d->extra_data, *(OUTPUT*)d->extra_data2);
  }
  
};

struct wt { string word; int tag; wt(string w, int t) : word(w), tag(t) {} };

class SequenceLabelerTask : public SearchTask< vector<wt>, vector<int> > {
  public:
  SequenceLabelerTask(vw*all) : SearchTask< vector<wt>, vector<int> >(all) {  // must run parent constructor!
    srn.set_options( Searn::AUTO_HAMMING_LOSS | Searn::AUTO_HISTORY | Searn::EXAMPLES_DONT_CHANGE );
  }

  void _run(Searn::searn& srn, vector<wt> & input_example, vector<int> & output) {
    output.clear();
    for (size_t i=0; i<input_example.size(); i++) {
      string   s  = "1 | " + input_example[i].word;
      example* ex = VW::read_example(all, (char*)s.c_str());
      uint32_t p  = srn.predict(ex, input_example[i].tag);
      output.push_back(p);
    }
  }
};

int main(int argc, char *argv[]) {
  // initialize VW as usual, but use 'hook' as the search_task
  vw* all = VW::initialize("--search 4 --quiet --search_task hook --search_no_snapshot --ring_size 1024");

  {
    // we put this in its own scope so that its destructor gets called
    // *before* VW::finish gets called; otherwise we'll get a
    // segfault :(. not sure what to do about this :(.
    SequenceLabelerTask task(all);
    vector<wt> data;
    vector<int> output;
    int DET = 1, NOUN = 2, VERB = 3, ADJ = 4;
    data.push_back( wt("the", DET) );
    data.push_back( wt("monster", NOUN) );
    data.push_back( wt("ate", VERB) );
    data.push_back( wt("a", DET) );
    data.push_back( wt("big", ADJ) );
    data.push_back( wt("sandwich", NOUN) );
    task.learn(data, output);
    task.learn(data, output);
    task.learn(data, output);
    task.predict(data, output);
    cerr << "output = [";
    for (size_t i=0; i<output.size(); i++) cerr << " " << output[i];
    cerr << " ]" << endl;
  }
  
  VW::finish(*all);
}














