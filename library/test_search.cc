#include <stdio.h>
#include "../vowpalwabbit/vw.h"
#include "libsearn.h"

struct wt {
  string word;
  int tag;
  wt(string w, int t) : word(w), tag(t) {}
};

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
      VW::finish_example(all, ex);
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














