#include <float.h>
#include "reductions.h"

using namespace std;
template <bool is_learn>
void predict_or_learn(char&, LEARNER::single_learner& base, example& ec)
{
  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  if ( ec.pred.scalar > 0)
    ec.pred.scalar = 1;
  else
    ec.pred.scalar = -1;

  if (ec.l.simple.label != FLT_MAX)
  {
    if (fabs(ec.l.simple.label) != 1.f)
      cout << "You are using label " << ec.l.simple.label << " not -1 or 1 as loss function expects!" << endl;
    else if (ec.l.simple.label == ec.pred.scalar)
      ec.loss = 0.;
    else
      ec.loss = ec.weight;
  }
}

LEARNER::base_learner* binary_setup(arguments& arg)
{
  if (arg.new_options("Binary loss").
      critical("binary", "report loss as binary classification on -1,1").missing())
    return nullptr;

  LEARNER::learner<char,example>& ret =
    LEARNER::init_learner(as_singleline(setup_base(arg)), predict_or_learn<true>, predict_or_learn<false>);
  return make_base(ret);
}
