#include <float.h>
#include "reductions.h"

using namespace std;
using namespace VW::config;

ostream* vw_log = &cout;

template <bool is_learn>
void predict_or_learn(char&, LEARNER::single_learner& base, example& ec)
{
  if (is_learn)
  {
    *vw_log << "binary: before-base.learn() " << simple_label_to_string(ec) << features_to_string(ec) << endl;
    base.learn(ec);
    *vw_log << "binary: after-base.learn() " << simple_label_to_string(ec) << features_to_string(ec) << endl;
  }
  else
  {
    *vw_log << "binary: before-base.predict() " << scalar_pred_to_string(ec) << features_to_string(ec) << endl;
    base.predict(ec);
    *vw_log << "binary: after-base.predict() " << scalar_pred_to_string(ec) << features_to_string(ec) << endl;
  }

  if (ec.pred.scalar > 0)
    ec.pred.scalar = 1;
  else
    ec.pred.scalar = -1;

  *vw_log << "binary: final-pred " << scalar_pred_to_string(ec) << features_to_string(ec) << endl;

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

LEARNER::base_learner* binary_setup(options_i& options, vw& all)
{
  bool binary = false;
  option_group_definition new_options("Binary loss");
  new_options.add(make_option("binary", binary).keep().help("report loss as binary classification on -1,1"));
  options.add_and_parse(new_options);

  if (!binary)
    return nullptr;

  LEARNER::learner<char, example>& ret =
      LEARNER::init_learner(as_singleline(setup_base(options, all)), predict_or_learn<true>, predict_or_learn<false>);
  return make_base(ret);
}
