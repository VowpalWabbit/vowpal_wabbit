#include "oaa.h"
#include "vw.h"

namespace BINARY {

  template <bool is_learn>
  void predict_or_learn(void* d, learner& base, example* ec) {
    if (is_learn)
      base.learn(ec);
    else
      base.predict(ec);

    if ( ec->final_prediction > 0)
      ec->final_prediction = 1;
    else
      ec->final_prediction = -1;

    label_data* ld = (label_data*)ec->ld;//New loss
    if (ld->label == ec->final_prediction)
      ec->loss = 0.;
    else
      ec->loss = 1.;
  }

  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {//parse and set arguments
    if (!vm_file.count("binary"))
      {
	std::stringstream ss;
	ss << " --binary ";
	all.options_from_file.append(ss.str());
      }

    all.sd->binary_label = true;
    //Create new learner
    return new learner(NULL, predict_or_learn<true>, predict_or_learn<false>, all.l);
  }
}
