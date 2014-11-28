/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>
#include <math.h>
#include <sstream>
#include <numeric>
#include <vector>

#include "reductions.h"
#include "vw.h"
#include "simple_label.h"
#include "rand48.h"
#include "bs.h"

using namespace std;
using namespace LEARNER;

namespace BS {

  struct bs{
    uint32_t B; //number of bootstrap rounds
    size_t bs_type;
    float lb;
    float ub;
    vector<double> pred_vec;
    vw* all;
  };

  void bs_predict_mean(vw& all, example& ec, vector<double> &pred_vec)
  {
    ec.pred.scalar = (float)accumulate(pred_vec.begin(), pred_vec.end(), 0.0)/pred_vec.size();
    ec.loss = all.loss->getLoss(all.sd, ec.pred.scalar, ec.l.simple.label) * ec.l.simple.weight;    
  }

  void bs_predict_vote(vw& all, example& ec, vector<double> &pred_vec)
  { //majority vote in linear time
    unsigned int counter = 0;
    int current_label = 1, init_label = 1;
    // float sum_labels = 0; // uncomment for: "avg on votes" and getLoss()
    bool majority_found = false;
    bool multivote_detected = false; // distinct(votes)>2: used to skip part of the algorithm
    int* pred_vec_int = new int[pred_vec.size()];

    for(unsigned int i=0; i<pred_vec.size(); i++)
    {
      pred_vec_int[i] = (int)floor(pred_vec[i]+0.5); // could be added: link(), min_label/max_label, cutoff between true/false for binary

      if(multivote_detected == false) { // distinct(votes)>2 detection bloc
        if(i == 0) {
          init_label = pred_vec_int[i];
          current_label = pred_vec_int[i];
        }
        else if(init_label != current_label && pred_vec_int[i] != current_label
                 && pred_vec_int[i] != init_label)
             multivote_detected = true; // more than 2 distinct votes detected
      }

      if (counter == 0) {
        counter = 1;
        current_label = pred_vec_int[i];
      }
      else {
        if(pred_vec_int[i] == current_label)
          counter++;
        else {
          counter--;
        }
      }
    }

    if(counter > 0 && multivote_detected) { // remove this condition for: "avg on votes" and getLoss()
      counter = 0;
      for(unsigned int i=0; i<pred_vec.size(); i++)
        if(pred_vec_int[i] == current_label) {
          counter++;
          // sum_labels += pred_vec[i]; // uncomment for: "avg on votes" and getLoss()
      }
      if(counter*2 > pred_vec.size())
        majority_found = true;
    }

    if(multivote_detected && majority_found == false) { // then find most frequent element - if tie: smallest tie label
        std::sort(pred_vec_int, pred_vec_int+pred_vec.size());
        int tmp_label = pred_vec_int[0];
        counter = 1;
        for(unsigned int i=1, temp_count=1; i<pred_vec.size(); i++)
        {
          if(tmp_label == pred_vec_int[i])
            temp_count++;
          else {
            if(temp_count > counter) {
              current_label = tmp_label;
              counter = temp_count;
            }
            tmp_label = pred_vec_int[i];
            temp_count = 1;
          }
        }
        /* uncomment for: "avg on votes" and getLoss()
        sum_labels = 0;
        for(unsigned int i=0; i<pred_vec.size(); i++)
          if(pred_vec_int[i] == current_label)
            sum_labels += pred_vec[i]; */
    }

    // ld.prediction = sum_labels/(float)counter; //replace line below for: "avg on votes" and getLoss()
    ec.pred.scalar = (float)current_label;

    // ec.loss = all.loss->getLoss(all.sd, ld.prediction, ld.label) * ld.weight; //replace line below for: "avg on votes" and getLoss()
    ec.loss = ((ec.pred.scalar == ec.l.simple.label) ? 0.f : 1.f) * ec.l.simple.weight;
  }

  void print_result(int f, float res, float weight, v_array<char> tag, float lb, float ub)
  {
    if (f >= 0)
    {
      char temp[30];
      sprintf(temp, "%f", res);
      std::stringstream ss;
      ss << temp;
      print_tag(ss, tag);
      ss << ' ';
      sprintf(temp, "%f", lb);
      ss << temp;
      ss << ' ';
      sprintf(temp, "%f", ub);
      ss << temp;
      ss << '\n';
      ssize_t len = ss.str().size();
      ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
      if (t != len)
        cerr << "write error" << endl;
    }    
  }

  void output_example(vw& all, bs& d, example& ec)
  {
    label_data& ld = ec.l.simple;
    
    if(ec.test_only)
      {
	all.sd->weighted_holdout_examples += ld.weight;//test weight seen
	all.sd->weighted_holdout_examples_since_last_dump += ld.weight;
	all.sd->weighted_holdout_examples_since_last_pass += ld.weight;
	all.sd->holdout_sum_loss += ec.loss;
	all.sd->holdout_sum_loss_since_last_dump += ec.loss;
	all.sd->holdout_sum_loss_since_last_pass += ec.loss;//since last pass
      }
    else
      {
    if (ld.label != FLT_MAX)
        all.sd->weighted_labels += ld.label * ld.weight;
	all.sd->weighted_examples += ld.weight;
	all.sd->sum_loss += ec.loss;
	all.sd->sum_loss_since_last_dump += ec.loss;
	all.sd->total_features += ec.num_features;
	all.sd->example_number++;
      }
    
    if(all.final_prediction_sink.size() != 0)//get confidence interval only when printing out predictions
    {
      d.lb = FLT_MAX;
      d.ub = -FLT_MAX;
      for (unsigned i = 0; i < d.pred_vec.size(); i++)
      {
        if(d.pred_vec[i] > d.ub)
          d.ub = (float)d.pred_vec[i];
        if(d.pred_vec[i] < d.lb)
          d.lb = (float)d.pred_vec[i];
      }
    }

    for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
      BS::print_result(*sink, ec.pred.scalar, 0, ec.tag, d.lb, d.ub);
  
    print_update(all, ec);
  }

  template <bool is_learn>
  void predict_or_learn(bs& d, learner& base, example& ec)
  {
    vw* all = d.all;
    bool shouldOutput = all->raw_prediction > 0;

    float weight_temp = ec.l.simple.weight;
  
    string outputString;
    stringstream outputStringStream(outputString);
    d.pred_vec.clear();

    for (size_t i = 1; i <= d.B; i++)
      {
        ec.l.simple.weight = weight_temp * (float) weight_gen();

	if (is_learn)
	  base.learn(ec, i-1);
	else
	  base.predict(ec, i-1);
	
        d.pred_vec.push_back(ec.pred.scalar);

        if (shouldOutput) {
          if (i > 1) outputStringStream << ' ';
          outputStringStream << i << ':' << ec.partial_prediction;
        }
      }	
    
    ec.l.simple.weight = weight_temp;
    
    switch(d.bs_type)
    {
      case BS_TYPE_MEAN:
        bs_predict_mean(*all, ec, d.pred_vec);
        break;
      case BS_TYPE_VOTE:
        bs_predict_vote(*all, ec, d.pred_vec);
        break;
      default:
        std::cerr << "Unknown bs_type specified: " << d.bs_type << ". Exiting." << endl;
        throw exception();
    }

    if (shouldOutput) 
      all->print_text(all->raw_prediction, outputStringStream.str(), ec.tag);

  }

  void finish_example(vw& all, bs& d, example& ec)
  {
    BS::output_example(all, d, ec);
    VW::finish_example(all, &ec);
  }

  void finish(bs& d)
  {
    d.pred_vec.~vector();
  }

  learner* setup(vw& all, po::variables_map& vm)
  {
    bs* data = (bs*)calloc_or_die(1, sizeof(bs));
    data->ub = FLT_MAX;
    data->lb = -FLT_MAX;

    po::options_description bs_options("Bootstrap options");
    bs_options.add_options()
      ("bs_type", po::value<string>(), "prediction type {mean,vote}");
    
    vm = add_options(all, bs_options);

    data->B = (uint32_t)vm["bootstrap"].as<size_t>();

    //append bs with number of samples to options_from_file so it is saved to regressor later
    std::stringstream ss;
    ss << " --bootstrap " << data->B;
    all.file_options.append(ss.str());

    std::string type_string("mean");

    if (vm.count("bs_type"))
    {
      type_string = vm["bs_type"].as<std::string>();
      
      if (type_string.compare("mean") == 0) { 
        data->bs_type = BS_TYPE_MEAN;
      }
      else if (type_string.compare("vote") == 0) {
        data->bs_type = BS_TYPE_VOTE;
      }
      else {
        std::cerr << "warning: bs_type must be in {'mean','vote'}; resetting to mean." << std::endl;
        data->bs_type = BS_TYPE_MEAN;
      }
    }
    else //by default use mean
      data->bs_type = BS_TYPE_MEAN;
    all.file_options.append(" --bs_type ");
    all.file_options.append(type_string);

    data->pred_vec.reserve(data->B);
    data->all = &all;

    learner* l = new learner(data, all.l, data->B);
    l->set_learn<bs, predict_or_learn<true> >();
    l->set_predict<bs, predict_or_learn<false> >();
    l->set_finish_example<bs,finish_example>();
    l->set_finish<bs,finish>();

    return l;
  }
}
