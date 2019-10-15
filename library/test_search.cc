#include <stdio.h>
#include <stdlib.h> // for system
#include "../vowpalwabbit/vw.h"
#include "../vowpalwabbit/ezexample.h"
#include "../vowpalwabbit/search_sequencetask.h"
#include "libsearch.h"

using std::cerr;
using std::endl;

struct wt
{ std::string word;
  uint32_t tag;
  wt(std::string w, uint32_t t) : word(w), tag(t) {}
};

class SequenceLabelerTask : public SearchTask<std::vector<wt>, std::vector<uint32_t> >
{
public:
  SequenceLabelerTask(vw& vw_obj)
    : SearchTask<std::vector<wt>, std::vector<uint32_t> >(vw_obj)    // must run parent constructor!
  { sch.set_options( Search::AUTO_HAMMING_LOSS | Search::AUTO_CONDITION_FEATURES );
    HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    cerr << "num_actions = " << d->num_actions << endl;
  }

  // using vanilla vw interface
  void _run(Search::search& sch, std::vector<wt> & input_example, std::vector<uint32_t> & output)
  { output.clear();
    for (size_t i=0; i<input_example.size(); i++)
    { example* ex = VW::read_example(vw_obj, std::string("1 |w ") + input_example[i].word);
      action p  = Search::predictor(sch, i+1).set_input(*ex).set_oracle(input_example[i].tag).set_condition(i, 'p').predict();
      VW::finish_example(vw_obj, *ex);
      output.push_back(p);
    }
  }

  // using ezexample
  void _run2(Search::search& sch, std::vector<wt> & input_example, std::vector<uint32_t> & output)
  { output.clear();
    for (size_t i=0; i<input_example.size(); i++)
    { ezexample ex(&vw_obj);
      ex(vw_namespace('w'))(input_example[i].word);  // add the feature
      action p  = Search::predictor(sch, i+1).set_input(*ex.get()).set_oracle(input_example[i].tag).set_condition(i, 'p').predict();
      output.push_back(p);
    }
  }

};

void run(vw& vw_obj)
{ // we put this in its own scope so that its destructor on
  // SequenceLabelerTask gets called *before* VW::finish gets called;
  // otherwise we'll get a segfault :(. i'm not sure what to do about
  // this :(.
  SequenceLabelerTask task(vw_obj);
  std::vector<wt> data;
  std::vector<uint32_t> output;
  uint32_t DET = 1, NOUN = 2, VERB = 3, ADJ = 4;
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
  cerr << "should have printed: 1 2 3 1 4 2" << endl;
}


void train()
{ // initialize VW as usual, but use 'hook' as the search_task
  cerr << endl << endl << "##### train() #####" << endl << endl;
  vw& vw_obj = *VW::initialize("--search 4 --quiet --search_task hook --ring_size 1024 -f my_model");
  run(vw_obj);
  VW::finish(vw_obj);
}

void predict()
{ cerr << endl << endl << "##### predict() #####" << endl << endl;
  vw& vw_obj = *VW::initialize("--quiet -t --ring_size 1024 -i my_model");
  run(vw_obj);
  VW::finish(vw_obj);
}

void test_buildin_task()
{ cerr << endl << endl << "##### run commandline vw #####" << endl << endl;
  // train a model on the command line
  int ret = system("../vowpalwabbit/vw -k -c --holdout_off --passes 20 --search 4 --search_task sequence -d sequence.data -f sequence.model");
  if (ret != 0) cerr << "../vowpalwabbit/vw failed" << endl;

  // now, load that model using the BuiltInTask library
  cerr << endl << endl << "##### test BuiltInTask #####" << endl << endl;
  vw& vw_obj = *VW::initialize("-t -i sequence.model --search_task hook");
  { // create a new scope for the task object
    BuiltInTask task(vw_obj, &SequenceTask::task);
    multi_ex V;
    V.push_back( VW::read_example(vw_obj, (char*)"1 | a") );
    V.push_back( VW::read_example(vw_obj, (char*)"1 | a") );
    V.push_back( VW::read_example(vw_obj, (char*)"1 | a") );
    V.push_back( VW::read_example(vw_obj, (char*)"1 | a") );
    V.push_back( VW::read_example(vw_obj, (char*)"1 | a") );
    std::vector<action> out;
    task.predict(V, out);
    cerr << "out (should be 1 2 3 4 3) =";
    for (size_t i=0; i<out.size(); i++)
      cerr << " " << out[i];
    cerr << endl;
    for (size_t i=0; i<V.size(); i++)
      VW::finish_example(vw_obj, *V[i]);
  }

  VW::finish(vw_obj);
}

int main(int argc, char *argv[])
{ train();
  predict();
  test_buildin_task();
}
