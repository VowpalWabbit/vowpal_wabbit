#include <float.h>
#include <unordered_map>
#include <vector>
#include "reductions.h"
using namespace std;


struct classweights
{
  std::unordered_map<int, double> weights;
  vw* all;

  classweights(std::string const& source, vw* all)
  {
    load_string(source);
    this->all = all;
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

template <bool is_learn>
static void predict_or_learn(classweights& cweights, LEARNER::base_learner& base, example& ec)
{
  switch (cweights.all->l->pred_type) {
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
    cweights = new classweights(classweight, &all);
    if (!all.quiet)
      all.trace_message << "parsed " << cweights->size() << " class weights" << endl;
  } else
    return nullptr;

  LEARNER::learner<classweights>& ret =
    LEARNER::init_learner<classweights>(cweights, setup_base(all), predict_or_learn<true>, predict_or_learn<false>);
  return make_base(ret);
}
