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
#include <algorithm>

#include "bs.h"
#include "simple_label.h"
#include "cache.h"
#include "v_hashmap.h"
#include "vw.h"
#include "rand48.h"

using namespace std;

namespace BS {

  struct bs{
    uint32_t B; //number of bootstrap rounds
    uint32_t increment;
    uint32_t total_increment;
    float alpha;
    float lb;
    float ub;
    learner base;
    vw* all;
  };

  size_t poisson_gen(double lamda)
  {
    double L = exp((-1)*lamda);
    size_t k = 0;
    double p = 1.;
    double u = 0.;
  
    do
    {
      k++;
      u = frand48();
      p *= u;
    }
    while (p > L);

    return (k-1);
  }


  int print_tag(std::stringstream& ss, v_array<char> tag)
  {
    if (tag.begin != tag.end){
      ss << ' ';
      ss.write(tag.begin, sizeof(char)*tag.size());
    } 
    return tag.begin != tag.end;
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

  void print_update(vw& all, example *ec)
  {
    if (all.sd->weighted_examples > all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
        label_data* ld = (label_data*) ec->ld;
        char label_buf[32];
        if (ld->label == FLT_MAX)
          strcpy(label_buf," unknown");
        else
          sprintf(label_buf,"%8.4f",ld->label);

        fprintf(stderr, "%-10.6f %-10.6f %10ld %11.1f %s %8.4f %8lu\n",
                all.sd->sum_loss/all.sd->weighted_examples,
                all.sd->sum_loss_since_last_dump / (all.sd->weighted_examples - all.sd->old_weighted_examples),
                (long int)all.sd->example_number,
                all.sd->weighted_examples,
                label_buf,
                ec->final_prediction,
                (long unsigned int)ec->num_features);
     
        all.sd->sum_loss_since_last_dump = 0.0;
        all.sd->old_weighted_examples = all.sd->weighted_examples;
        all.sd->dump_interval *= 2;
      }
  }

  void output_example(vw& all, example* ec, float lb, float ub)
  {
    if (command_example(&all,ec))
      return;

    label_data* ld = (label_data*)ec->ld;
    all.sd->weighted_examples += ld->weight;
    all.sd->total_features += ec->num_features;

    all.sd->sum_loss += ec->loss;
    all.sd->sum_loss_since_last_dump += ec->loss;
  
    for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
      BS::print_result(*sink, ec->final_prediction, 0, ec->tag, lb, ub);
  
    all.sd->example_number++;

    print_update(all, ec);
  }

  void learn_with_output(bs* d, example* ec, bool shouldOutput)
  {
    vw* all = d->all;
    if (command_example(all,ec))
      {
	d->base.learn(ec);
	return;
      }

    double weight_temp = ((label_data*)ec->ld)->weight;
  
    string outputString;
    stringstream outputStringStream(outputString);

    float pre_mean = 0.0;

    vector<double> pred_vec;

    for (size_t i = 1; i <= d->B; i++)
      {
        if (i != 1)
          update_example_indicies(all->audit, ec, d->increment);
          
        ((label_data*)ec->ld)->weight = poisson_gen(weight_temp);

        d->base.learn(ec);

        pred_vec.push_back(ec->final_prediction);

        if (shouldOutput) {
          if (i > 1) outputStringStream << ' ';
          outputStringStream << i << ':' << ec->partial_prediction;
        }
      }	

    ((label_data*)ec->ld)->weight = weight_temp;

    update_example_indicies(all->audit, ec, -d->total_increment);
 
    sort(pred_vec.begin(), pred_vec.end());
    
    pre_mean = accumulate(pred_vec.begin(), pred_vec.end(), 0.0)/pred_vec.size();
    ec->final_prediction = pre_mean;
    ec->loss = all->loss->getLoss(all->sd, ec->final_prediction, ((label_data*)ec->ld)->label) * ((label_data*)ec->ld)->weight;

    size_t lb_index = d->B * d->alpha-1 < 0 ? 0 :  d->B * d->alpha-1;
    size_t ub_index = d->B * (1 - d->alpha)-1 > pred_vec.size()-1 ? pred_vec.size()-1 : d->B * (1 - d->alpha)-1;

    d->lb = pred_vec[lb_index];
    d->ub = pred_vec[ub_index];

    //cout<<" "<<ec->final_prediction<<" "<<d->lb<<" "<<d->ub<<endl;

    if (shouldOutput) 
      all->print_text(all->raw_prediction, outputStringStream.str(), ec->tag);

  }

  void learn(void* d, example* ec) {
    learn_with_output((bs*)d, ec, false);
  }

  void drive(vw* all, void* d)
  {
    example* ec = NULL;
    while ( true )
      {
        if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
          {
            learn_with_output((bs*)d, ec, all->raw_prediction > 0);
            if (!command_example(all, ec))
              BS::output_example(*all, ec, ((bs*)d)->lb , ((bs*)d)->ub);
	    VW::finish_example(*all, ec);
          }
        else if (parser_done(all->p))
	  return;
        else 
          ;
      }
  }

  void finish(void* data)
  {    
    bs* o=(bs*)data;
    o->base.finish();
    free(o);
  }

  learner setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    bs* data = (bs*)calloc(1, sizeof(bs));
    data->alpha = 0.;
    data->ub = FLT_MAX;
    data->lb = -FLT_MAX;

    po::options_description desc("BS options");
    desc.add_options()
      ("bs_percentile", po::value<float>(), "percentile for confidence interval");

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


    if (vm.count("bs_percentile") || vm_file.count("bs_percentile"))
    {
      if(vm_file.count("bs_percentile"))
        data->alpha = 1 - vm_file["bs_percentile"].as<float>();
      else {
        data->alpha = 1 - vm["bs_percentile"].as<float>();
        std::stringstream ss;
        ss << " --bs_percentile " << vm["bs_percentile"].as<float>();
        all.options_from_file.append(ss.str());
      }
      if(data->alpha > 1 || data->alpha < 0)
        std::cerr << "warning: bs_percentile should be between 0 and 1 !"<< endl;
    }

    if( vm_file.count("bs") ) {
      data->B = (uint32_t)vm_file["bs"].as<size_t>();
      if( vm.count("bs") && (uint32_t)vm["bs"].as<size_t>() != data->B )
        std::cerr << "warning: you specified a different number of actions through --bs than the one loaded from predictor. Pursuing with loaded value of: " << data->B << endl;
    }
    else {
      data->B = (uint32_t)vm["bs"].as<size_t>();

      //append bs with nb_actions to options_from_file so it is saved to regressor later
      std::stringstream ss;
      ss << " --bs " << data->B;
      all.options_from_file.append(ss.str());
    }

    data->all = &all;
    data->increment = all.reg.stride * all.weights_per_problem;
    all.weights_per_problem *= data->B;
    data->total_increment = data->increment*(data->B-1);
    data->base = all.l;
    learner l(data, drive, learn, finish, all.l.sl);
    return l;
  }
}
