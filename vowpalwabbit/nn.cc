/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <sstream>

#include "constant.h"
#include "oaa.h"
#include "simple_label.h"
#include "cache.h"
#include "v_hashmap.h"
#include "rand48.h"
#include "vw.h"

using namespace std;

namespace NN {
  const float hidden_min_activation = -3;
  const float hidden_max_activation = 3;
  const int nn_constant = 533357803;
  
  struct nn {
    uint32_t k;
    loss_function* squared_loss;
    example output_layer;
    size_t increment;
    bool dropout;
    uint64_t xsubi;
    uint64_t save_xsubi;
    bool inpass;
    bool finished_setup;

    vw* all;
  };

#define cast_uint32_t static_cast<uint32_t>

  static inline float
  fastpow2 (float p)
  {
    float offset = (p < 0) ? 1.0f : 0.0f;
    float clipp = (p < -126) ? -126.0f : p;
    int w = (int)clipp;
    float z = clipp - w + offset;
    union { uint32_t i; float f; } v = { cast_uint32_t ( (1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z) ) };

    return v.f;
  }

  static inline float
  fastexp (float p)
  {
    return fastpow2 (1.442695040f * p);
  }

  static inline float
  fasttanh (float p)
  {
    return -1.0f + 2.0f / (1.0f + fastexp (-2.0f * p));
  }

  void finish_setup (nn& n, vw& all)
  {
    bool initialize = true;

    // TODO: output_layer audit

    memset (&n.output_layer, 0, sizeof (n.output_layer));
    n.output_layer.indices.push_back(nn_output_namespace);
    feature output = {1., nn_constant*all.reg.stride};

    for (unsigned int i = 0; i < n.k; ++i)
      {
        n.output_layer.atomics[nn_output_namespace].push_back(output);
        initialize &= (all.reg.weight_vector[output.weight_index & all.reg.weight_mask] == 0);
        ++n.output_layer.num_features;
        output.weight_index += (uint32_t)n.increment;
      }

    if (! n.inpass) 
      {
        n.output_layer.atomics[nn_output_namespace].push_back(output);
        initialize &= (all.reg.weight_vector[output.weight_index & all.reg.weight_mask] == 0);
        ++n.output_layer.num_features;
      }

    n.output_layer.in_use = true;

    if (initialize) {
      // output weights

      float sqrtk = sqrt ((float)n.k);
      for (feature* x = n.output_layer.atomics[nn_output_namespace].begin; 
           x != n.output_layer.atomics[nn_output_namespace].end; 
           ++x)
        {
          weight* w = &all.reg.weight_vector[x->weight_index & all.reg.weight_mask];

          w[0] = (float) (frand48 () - 0.5) / sqrtk;

          // prevent divide by zero error
          if (n.dropout && all.normalized_updates)
            w[all.normalized_idx] = 1e-4f;
        }

      // hidden biases

      unsigned int weight_index = constant * all.reg.stride;

      for (unsigned int i = 0; i < n.k; ++i)
        {
          weight_index += (uint32_t)n.increment;
          all.reg.weight_vector[weight_index & all.reg.weight_mask] = (float) (frand48 () - 0.5);
        }
    }

    n.finished_setup = true;
  }

  void end_pass(void* d)
  {
    nn* n = (nn*)d;
    
    if (n->all->bfgs)
      n->xsubi = n->save_xsubi;
  }

  void learn(void* d, learner& base, example* ec)
  {
    nn* n = (nn*)d;
    bool shouldOutput = n->all->raw_prediction > 0;

    if (! n->finished_setup)
      finish_setup (*n, *(n->all));

    label_data* ld = (label_data*)ec->ld;
    float save_label = ld->label;
    void (*save_set_minmax) (shared_data*, float) = n->all->set_minmax;
    float save_min_label;
    float save_max_label;
    float dropscale = n->dropout ? 2.0f : 1.0f;
    loss_function* save_loss = n->all->loss;

    float* hidden_units = (float*) alloca (n->k * sizeof (float));
    bool* dropped_out = (bool*) alloca (n->k * sizeof (bool));
  
    string outputString;
    stringstream outputStringStream(outputString);

    n->all->set_minmax = noop_mm;
    n->all->loss = n->squared_loss;
    save_min_label = n->all->sd->min_label;
    n->all->sd->min_label = hidden_min_activation;
    save_max_label = n->all->sd->max_label;
    n->all->sd->max_label = hidden_max_activation;
    ld->label = FLT_MAX;
    for (unsigned int i = 0; i < n->k; ++i)
      {
        base.learn(ec, i);
        hidden_units[i] = ec->final_prediction;

        dropped_out[i] = (n->dropout && merand48 (n->xsubi) < 0.5);

        if (shouldOutput) {
          if (i > 0) outputStringStream << ' ';
          outputStringStream << i << ':' << ec->partial_prediction << ',' << fasttanh (hidden_units[i]);
        }
      }
    ld->label = save_label;
    n->all->loss = save_loss;
    n->all->set_minmax = save_set_minmax;
    n->all->sd->min_label = save_min_label;
    n->all->sd->max_label = save_max_label;

    bool converse = false;
    float save_partial_prediction = 0;
    float save_final_prediction = 0;
    float save_ec_loss = 0;

CONVERSE: // That's right, I'm using goto.  So sue me.

    n->output_layer.total_sum_feat_sq = 1;
    n->output_layer.sum_feat_sq[nn_output_namespace] = 1;

    for (unsigned int i = 0; i < n->k; ++i)
      {
        float sigmah = 
          (dropped_out[i]) ? 0.0f : dropscale * fasttanh (hidden_units[i]);
        n->output_layer.atomics[nn_output_namespace][i].x = sigmah;

        n->output_layer.total_sum_feat_sq += sigmah * sigmah;
        n->output_layer.sum_feat_sq[nn_output_namespace] += sigmah * sigmah;
      }

    if (n->inpass) {
      // TODO: this is not correct if there is something in the 
      // nn_output_namespace but at least it will not leak memory
      // in that case

      ec->indices.push_back (nn_output_namespace);
      v_array<feature> save_nn_output_namespace = ec->atomics[nn_output_namespace];
      ec->atomics[nn_output_namespace] = n->output_layer.atomics[nn_output_namespace];
      ec->sum_feat_sq[nn_output_namespace] = n->output_layer.sum_feat_sq[nn_output_namespace];
      ec->total_sum_feat_sq += n->output_layer.sum_feat_sq[nn_output_namespace];
      base.learn(ec, n->k);
      n->output_layer.partial_prediction = ec->partial_prediction;
      n->output_layer.loss = ec->loss;
      ec->total_sum_feat_sq -= n->output_layer.sum_feat_sq[nn_output_namespace];
      ec->sum_feat_sq[nn_output_namespace] = 0;
      ec->atomics[nn_output_namespace] = save_nn_output_namespace;
      ec->indices.pop ();
    }
    else {
      n->output_layer.ld = ec->ld;
      n->output_layer.partial_prediction = 0;
      n->output_layer.eta_round = ec->eta_round;
      n->output_layer.eta_global = ec->eta_global;
      n->output_layer.global_weight = ec->global_weight;
      n->output_layer.example_t = ec->example_t;
      base.learn(&n->output_layer, n->k);
      n->output_layer.ld = 0;
    }

    n->output_layer.final_prediction = GD::finalize_prediction (*(n->all), n->output_layer.partial_prediction);

    if (shouldOutput) {
      outputStringStream << ' ' << n->output_layer.partial_prediction;
      n->all->print_text(n->all->raw_prediction, outputStringStream.str(), ec->tag);
    }

    if (n->all->training && ld->label != FLT_MAX) {
      float gradient = n->all->loss->first_derivative(n->all->sd, 
                                                  n->output_layer.final_prediction,
                                                  ld->label);

      if (fabs (gradient) > 0) {
        n->all->loss = n->squared_loss;
        n->all->set_minmax = noop_mm;
        save_min_label = n->all->sd->min_label;
        n->all->sd->min_label = hidden_min_activation;
        save_max_label = n->all->sd->max_label;
        n->all->sd->max_label = hidden_max_activation;

        for (unsigned int i = 0; i < n->k; ++i) {
          if (! dropped_out[i]) {
            float sigmah = 
              n->output_layer.atomics[nn_output_namespace][i].x / dropscale;
            float sigmahprime = dropscale * (1.0f - sigmah * sigmah);
            float nu = n->all->reg.weight_vector[n->output_layer.atomics[nn_output_namespace][i].weight_index & n->all->reg.weight_mask];
            float gradhw = 0.5f * nu * gradient * sigmahprime;

            ld->label = GD::finalize_prediction (*(n->all), hidden_units[i] - gradhw);
            if (ld->label != hidden_units[i]) 
              base.learn(ec, i);
          }
        }

        n->all->loss = save_loss;
        n->all->set_minmax = save_set_minmax;
        n->all->sd->min_label = save_min_label;
        n->all->sd->max_label = save_max_label;
      }
    }

    ld->label = save_label;

    if (! converse) {
      save_partial_prediction = n->output_layer.partial_prediction;
      save_final_prediction = n->output_layer.final_prediction;
      save_ec_loss = n->output_layer.loss;
    }

    if (n->dropout && ! converse)
      {
        for (unsigned int i = 0; i < n->k; ++i)
          {
            dropped_out[i] = ! dropped_out[i];
          }

        converse = true;
        goto CONVERSE;
      }

    ec->partial_prediction = save_partial_prediction;
    ec->final_prediction = save_final_prediction;
    ec->loss = save_ec_loss;
  }

  void finish_example(vw& all, void*, example* ec)
  {
    int save_raw_prediction = all.raw_prediction;
    all.raw_prediction = -1;
    return_simple_example(all, NULL, ec);
    all.raw_prediction = save_raw_prediction;
  }

  void finish(void* d)
  {
    nn* n =(nn*)d;
    delete n->squared_loss;
    free (n->output_layer.indices.begin);
    free (n->output_layer.atomics[nn_output_namespace].begin);
  }

  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    nn* n = (nn*)calloc(1,sizeof(nn));
    n->all = &all;

    po::options_description desc("NN options");
    desc.add_options()
      ("inpass", "Train or test sigmoidal feedforward network with input passthrough.")
      ("dropout", "Train or test sigmoidal feedforward network using dropout.")
      ("meanfield", "Train or test sigmoidal feedforward network using mean field.");

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

    //first parse for number of hidden units
    n->k = 0;
    if( vm_file.count("nn") ) {
      n->k = (uint32_t)vm_file["nn"].as<size_t>();
      if( vm.count("nn") && (uint32_t)vm["nn"].as<size_t>() != n->k )
        std::cerr << "warning: you specified a different number of hidden units through --nn than the one loaded from predictor. Pursuing with loaded value of: " << n->k << endl;
    }
    else {
      n->k = (uint32_t)vm["nn"].as<size_t>();

      std::stringstream ss;
      ss << " --nn " << n->k;
      all.options_from_file.append(ss.str());
    }

    if( vm_file.count("dropout") ) {
      n->dropout = all.training || vm.count("dropout");

      if (! n->dropout && ! vm.count("meanfield") && ! all.quiet) 
        std::cerr << "using mean field for testing, specify --dropout explicitly to override" << std::endl;
    }
    else if ( vm.count("dropout") ) {
      n->dropout = true;

      std::stringstream ss;
      ss << " --dropout ";
      all.options_from_file.append(ss.str());
    }

    if ( vm.count("meanfield") ) {
      n->dropout = false;
      if (! all.quiet) 
        std::cerr << "using mean field for neural network " 
                  << (all.training ? "training" : "testing") 
                  << std::endl;
    }

    if (n->dropout) 
      if (! all.quiet)
        std::cerr << "using dropout for neural network "
                  << (all.training ? "training" : "testing") 
                  << std::endl;

    if( vm_file.count("inpass") ) {
      n->inpass = true;
    }
    else if (vm.count ("inpass")) {
      n->inpass = true;

      std::stringstream ss;
      ss << " --inpass";
      all.options_from_file.append(ss.str());
    }

    if (n->inpass && ! all.quiet)
      std::cerr << "using input passthrough for neural network "
                << (all.training ? "training" : "testing") 
                << std::endl;

    n->finished_setup = false;
    n->squared_loss = getLossFunction (0, "squared", 0);

    n->xsubi = 0;

    if (vm.count("random_seed"))
      n->xsubi = vm["random_seed"].as<size_t>();

    n->save_xsubi = n->xsubi;
    learner* l = new learner(n, learn, all.l, n->k+1);
    l->set_finish(finish);
    l->set_finish_example(finish_example);
    l->set_end_pass(end_pass);
    n->increment = l->increment;//Indexing of output layer is odd.

    return l;
  }
}
