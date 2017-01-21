#include "gd.h"
#include "float.h"
#include "reductions.h"

using namespace std;
struct print { vw* all; }; //regressor, feature loop

void print_feature(vw& all, float value, uint64_t index)
{ cout << index;
  if (value != 1.)
    cout << ":" << value;
  cout << " ";
}

void learn(print& p, LEARNER::base_learner&, example& ec)
{ label_data& ld = ec.l.simple;
  if (ld.label != FLT_MAX)
  { cout << ld.label << " ";
    if (ec.weight != 1 || ld.initial != 0)
    { cout << ec.weight << " ";
      if (ld.initial != 0)
        cout << ld.initial << " ";
    }
  }
  if (ec.tag.size() > 0)
  { cout << '\'';
    cout.write(ec.tag.begin(), ec.tag.size());
  }
  cout << "| ";
  GD::foreach_feature<vw, uint64_t, print_feature>(*(p.all), ec, *p.all);
  cout << endl;
}

LEARNER::base_learner* print_setup(vw& all)
{ if (missing_option(all, true, "print", "print examples")) return nullptr;

  print& p = calloc_or_throw<print>();
  p.all = &all;

  all.weights.stride_shift(0);

  LEARNER::learner<print>& ret = init_learner(&p, learn, 1);
  return make_base(ret);
}
