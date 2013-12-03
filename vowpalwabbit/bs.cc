/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <sstream>
#include <numeric>
#include <vector>

#include "bs.h"
#include "cache.h"
#include "v_hashmap.h"
#include "vw.h"
#include "rand48.h"

using namespace std;

namespace BS {

  struct bs{
    uint32_t B; //number of bootstrap rounds
    size_t bs_type;
    float lb;
    float ub;
    vector<double> pred_vec;
    vw* all;
  };

  float weight_gen()//sampling from Poisson with rate 1
  { 
    float temp = frand48();
    if(temp<=0.3678794411714423215955) return 0.;
    if(temp<=0.735758882342884643191)  return 1.;
    if(temp<=0.919698602928605803989)  return 2.;
    if(temp<=0.9810118431238461909214) return 3.;
    if(temp<=0.9963401531726562876545) return 4.;
    if(temp<=0.9994058151824183070012) return 5.;
    if(temp<=0.9999167588507119768923) return 6.;
    if(temp<=0.9999897508033253583053) return 7.;
    if(temp<=0.9999988747974020309819) return 8.;
    if(temp<=0.9999998885745216612793) return 9.;
    if(temp<=0.9999999899522336243091) return 10.;
    if(temp<=0.9999999991683892573118) return 11.;
    if(temp<=0.9999999999364022267287) return 12;
    if(temp<=0.999999999995480147453) return 13.;
    if(temp<=0.9999999999996999989333) return 14.;
    if(temp<=0.9999999999999813223654) return 15.;
    if(temp<=0.9999999999999989050799) return 16.;
    if(temp<=0.9999999999999999393572) return 17.;
    if(temp<=0.999999999999999996817)  return 18.;
    if(temp<=0.9999999999999999998412) return 19.;
    return 20.;
  }

  void bs_predict_mean(vw& all, example* ec, vector<double> &pred_vec)
  {
    ec->final_prediction = (float)accumulate(pred_vec.begin(), pred_vec.end(), 0.0)/pred_vec.size();
    ec->loss = all.loss->getLoss(all.sd, ec->final_prediction, ((label_data*)ec->ld)->label) * ((label_data*)ec->ld)->weight;    
  }

  void bs_predict_vote(vw& all, example* ec, vector<double> &pred_vec)
  { //majority vote in linear time
    unsigned int counter = 0;
    float current_label = 1.;
    for(unsigned int i=0; i<pred_vec.size(); i++)
    {
      if(pred_vec[i] == current_label)
        counter++;
      else
      { 
        if (counter == 0)
        {
          counter = 1;
          current_label = (float)pred_vec[i];
        }
        else
          counter--;
      }
       
      if(counter==0)
        current_label = -1;
    }
    if(counter == 0)//no majority exists
    {
      ec->final_prediction = -1;
      ec->loss = 1.;
      return;
    }
    //will output majority if it exists
    ec->final_prediction = current_label;
    if (ec->final_prediction == ((label_data*)ec->ld)->label)
      ec->loss = 0.;
    else
      ec->loss = 1.;
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
#ifdef _WIN32
	  ssize_t t = _write(f, ss.str().c_str(), (unsigned int)len);
#else
	  ssize_t t = write(f, ss.str().c_str(), (unsigned int)len);
#endif
      if (t != len)
        cerr << "write error" << endl;
    }    
  }

  void output_example(vw& all, bs* d, example* ec)
  {
    label_data* ld = (label_data*)ec->ld;
    
    if(ec->test_only)
      {
	all.sd->weighted_holdout_examples += ld->weight;//test weight seen
	all.sd->weighted_holdout_examples_since_last_dump += ld->weight;
	all.sd->weighted_holdout_examples_since_last_pass += ld->weight;
	all.sd->holdout_sum_loss += ec->loss;
	all.sd->holdout_sum_loss_since_last_dump += ec->loss;
	all.sd->holdout_sum_loss_since_last_pass += ec->loss;//since last pass
      }
    else
      {
	all.sd->weighted_examples += ld->weight;
	all.sd->sum_loss += ec->loss;
	all.sd->sum_loss_since_last_dump += ec->loss;
	all.sd->total_features += ec->num_features;
	all.sd->example_number++;
      }
    
    if(all.final_prediction_sink.size() != 0)//get confidence interval only when printing out predictions
    {
      d->lb = FLT_MAX;
      d->ub = -FLT_MAX;
      for (unsigned i = 0; i < d->pred_vec.size(); i++)
      {
        if(d->pred_vec[i] > d->ub)
          d->ub = (float)d->pred_vec[i];
        if(d->pred_vec[i] < d->lb)
          d->lb = (float)d->pred_vec[i];
      }
    }

    for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
      BS::print_result(*sink, ec->final_prediction, 0, ec->tag, d->lb, d->ub);
  
    print_update(all, ec);
  }

  void learn(void* data, learner& base, example* ec)
  {
    bs* d = (bs*)data;
    vw* all = d->all;
    bool shouldOutput = all->raw_prediction > 0;

    float weight_temp = ((label_data*)ec->ld)->weight;
  
    string outputString;
    stringstream outputStringStream(outputString);
    d->pred_vec.clear();

    for (size_t i = 1; i <= d->B; i++)
      {
        ((label_data*)ec->ld)->weight = weight_temp * weight_gen();

        base.learn(ec, i-1);

        d->pred_vec.push_back(ec->final_prediction);

        if (shouldOutput) {
          if (i > 1) outputStringStream << ' ';
          outputStringStream << i << ':' << ec->partial_prediction;
        }
      }	

    ((label_data*)ec->ld)->weight = weight_temp;

    switch(d->bs_type)
    {
      case BS_TYPE_MEAN:
        bs_predict_mean(*all, ec, d->pred_vec);
        break;
      case BS_TYPE_VOTE:
        bs_predict_vote(*all, ec, d->pred_vec);
        break;
      default:
        std::cerr << "Unknown bs_type specified: " << d->bs_type << ". Exiting." << endl;
        throw exception();
    }

    if (shouldOutput) 
      all->print_text(all->raw_prediction, outputStringStream.str(), ec->tag);

  }

  void finish_example(vw& all, void* d, example* ec)
  {
    BS::output_example(all, (bs*)d, ec);
    VW::finish_example(all, ec);
  }

  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    bs* data = (bs*)calloc(1, sizeof(bs));
    data->ub = FLT_MAX;
    data->lb = -FLT_MAX;

    po::options_description desc("BS options");
    desc.add_options()
      ("bs_type", po::value<string>(), "prediction type {mean,vote}");

    po::parsed_options parsed = po::command_line_parser(opts).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc).allow_unregistered().run();
    opts = po::collect_unrecognized(parsed.options, po::include_positional);
    po::store(parsed, vm);
    po::notify(vm);

    po::parsed_options parsed_file = po::command_line_parser(all.options_from_file_argc,all.options_from_file_argv).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc).allow_unregistered().run();
    po::store(parsed_file, vm_file);
    po::notify(vm_file);

    if( vm_file.count("bs") ) {
      data->B = (uint32_t)vm_file["bs"].as<size_t>();
      if( vm.count("bs") && (uint32_t)vm["bs"].as<size_t>() != data->B )
        std::cerr << "warning: you specified a different number of samples through --bs than the one loaded from predictor. Pursuing with loaded value of: " << data->B << endl;
    }
    else {
      data->B = (uint32_t)vm["bs"].as<size_t>();

      //append bs with number of samples to options_from_file so it is saved to regressor later
      std::stringstream ss;
      ss << " --bs " << data->B;
      all.options_from_file.append(ss.str());
    }

    if (vm.count("bs_type") || vm_file.count("bs_type"))
    {
      std::string type_string;

      if(vm_file.count("bs_type")) {
        type_string = vm_file["bs_type"].as<std::string>();
        if( vm.count("bs_type") && type_string.compare(vm["bs_type"].as<string>()) != 0)
          cerr << "You specified a different --bs_type than the one loaded from regressor file. Pursuing with loaded value of: " << type_string << endl;
      }
      else {
        type_string = vm["bs_type"].as<std::string>();

        all.options_from_file.append(" --bs_type ");
        all.options_from_file.append(type_string);
      }

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
    else {
      //by default use mean
      data->bs_type = BS_TYPE_MEAN;
      all.options_from_file.append(" --bs_type mean");
    }

    data->pred_vec.reserve(data->B);
    data->all = &all;

    learner* l = new learner(data, learn, all.l, data->B);
    l->set_finish_example(finish_example);

    return l;
  }
}
