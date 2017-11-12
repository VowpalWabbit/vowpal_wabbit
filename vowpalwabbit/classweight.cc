#include <float.h>
#include <unordered_map>
#include <vector>
#include "reductions.h"
using namespace std;


struct classweights
{
  std::unordered_map<int, double> weights;

  classweights(std::string const& source)
  {
    load_string(source);
  }

  void load_string(std::string const& source)
  {
    std::stringstream ss(source);
    std::string item;
    while (std::getline(ss, item, ',')) {
      std::stringstream inner_ss(item);
      std::string klass;
      std::string weight;
      std::getline(inner_ss, klass, ':');
      std::getline(inner_ss, weight, ':');

      if (!klass.size() || !weight.size()) {
        THROW("error: while parsing --classweight " << item);
      }

      int klass_int = std::stoi(klass);
      double weight_double = std::stod(weight);

      weights[klass_int] = weight_double;
    }
  }

  double get_class_weight(int klass) {
    auto got = weights.find(klass);
    if ( got == weights.end() )
      return 1.0;
    else
      return got->second;
  }

  size_t size() {
    return weights.size();
  }

};

template <bool is_learn, int pred_type>
static void predict_or_learn(classweights& cweights, LEARNER::base_learner& base, example& ec)
{
  switch (pred_type) {
    case prediction_type::scalar:
      ec.weight *= cweights.get_class_weight(ec.l.simple.label);
      break;
    case prediction_type::multiclass:
      ec.weight *= cweights.get_class_weight(ec.l.multi.label);
      break;
    default:
      // suppress the warning
      break;
  }

  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);
}

LEARNER::base_learner* classweight_setup(vw& all)
{
  string classweight;
  new_options(all)
   ("classweight", po::value<std::string>(&classweight))
  ;
  add_options(all);

  classweights* cweights;
  po::variables_map& vm = all.vm;

  if (vm.count("classweight"))
  { *all.file_options << " --classweight " << classweight << ' ';
    cweights = new classweights(classweight);
    if (!all.quiet)
      all.trace_message << "parsed " << cweights->size() << " class weights" << endl;
  } else
    return nullptr;

  LEARNER::base_learner* base = setup_base(all);

  if (base->pred_type == prediction_type::scalar)
    return make_base(LEARNER::init_learner<classweights>(cweights, base, predict_or_learn<true,prediction_type::scalar>, predict_or_learn<false,prediction_type::scalar>));
  else if (base->pred_type == prediction_type::multiclass)
    return make_base(LEARNER::init_learner<classweights>(cweights, base, predict_or_learn<true,prediction_type::multiclass>, predict_or_learn<false,prediction_type::multiclass>));
  else
    THROW("--classweight not implemented for this type of prediction");
}
