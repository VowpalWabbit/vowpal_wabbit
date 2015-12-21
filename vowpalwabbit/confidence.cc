#include "reductions.h"
#include "vw.h"

using namespace LEARNER;

struct confidence
{ vw* all;//statistics, loss
};

template <bool is_learn>
void predict_or_learn_with_confidence(confidence& c, base_learner& base, example& ec)
{ if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);
  float threshold = 0.f;

  ec.confidence = fabsf(ec.pred.scalar - threshold) / base.sensitivity(ec);
  cout << "confidence = " << ec.confidence << " pred = " << ec.pred.scalar << " threshold = " << threshold << " sensitivity = " << base.sensitivity(ec) << endl;
}

void confidence_print_result(int f, float res, float confidence, v_array<char> tag)
{ if (f >= 0)
  { std::stringstream ss;
    char temp[30];
    sprintf(temp, "%f %f", res, confidence);
    ss << temp;
    if(!print_tag(ss, tag))
      ss << ' ';
    ss << '\n';
    ssize_t len = ss.str().size();
    ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
    if (t != len)
      cerr << "write error: " << strerror(errno) << endl;
  }
}

void output_and_account_confidence_example(vw& all, example& ec)
{ label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ec.loss, ec.weight, ec.num_features);
  if (ld.label != FLT_MAX && !ec.test_only)
    all.sd->weighted_labels += ld.label * ec.weight;
  all.sd->weighted_unlabeled_examples += ld.label == FLT_MAX ? ec.weight : 0;

  all.print(all.raw_prediction, ec.partial_prediction, -1, ec.tag);
  for (size_t i = 0; i<all.final_prediction_sink.size(); i++)
  { int f = (int)all.final_prediction_sink[i];
    confidence_print_result(f, ec.pred.scalar, ec.confidence, ec.tag);
  }

  print_update(all, ec);
}

void return_confidence_example(vw& all, confidence& c, example& ec)
{ output_and_account_confidence_example(all, ec);
  VW::finish_example(all,&ec);
}

base_learner* confidence_setup(vw& all)
{ //parse and set arguments
  if(missing_option(all, false, "confidence", "Get confidence for binary predictions")) return nullptr;

  confidence& data = calloc_or_throw<confidence>();
  data.all=&all;

  //Create new learner
  learner<confidence>& l = init_learner(&data, setup_base(all), predict_or_learn_with_confidence<true>,
                                        predict_or_learn_with_confidence<false>);
  l.set_finish_example(return_confidence_example);

  return make_base(l);
}
