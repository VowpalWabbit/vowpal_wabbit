/*
 Copyright (c) by respective owners including Yahoo!, Microsoft, and
 individual contributors. All rights reserved.  Released under a BSD (revised)
 license as described in the file LICENSE.
 */

/*
 * Implementation of online boosting algorithms in the following two papers: 
 * 1) Beygelzimer, Kale, Luo: Optimal and adaptive algorithms for online 
 *    boosting, ICML-2015. 
 * 2) Chen, Lin, and Lu: An online boosting algorithm with theoretical 
 *    justifications, ICML-2012.
 */

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>

#include "reductions.h"
#include "vw.h"
#include "rand48.h"

using namespace std;
using namespace LEARNER;

inline float sign(float w) { if (w <= 0.) return -1.; else  return 1.;}

/* 
 * Module to compute projections on the positive simplex or the L1-ball
 * translated from Adrien Gaidon's Python implementation:
 * https://gist.github.com/daien/1272551
 *
 * Based on:
 * Duchi, Shalev-Shwartz, Singer, and Chandra: Efficient Projections 
 * onto the l1-Ball for Learning in High Dimensions, ICML 2008.
 */
void euclidean_proj_simplex(std::vector<float> &v) {
    float sum = 0;

    // get the array of cumulative sums of a sorted (decreasing) copy of v
    std::vector<float> u(v);
    std::sort(u.begin(), u.end(), std::greater<float>());

    std::vector<float> cssv(u);
    for(size_t i = 1; i<cssv.size(); i++) cssv[i] += cssv[i-1];
    
    // get the number of > 0 components of the optimal solution     
    float rho = 0, theta = 0;
    for(size_t i = 0; i<cssv.size(); i++) {
	if (u[i]*(i+1) > (cssv[i]-1)) rho++;
    }
    // compute the Lagrange multiplier associated to the simplex constraint
    if (rho > 0)
        theta = (cssv[rho-1] - 1)/rho;

    // compute the projection by thresholding v using theta
    for(size_t i = 0; i<cssv.size(); i++) {
	v[i] -= theta;
        if (v[i]<0) v[i]=0;
    }
}

void euclidean_proj_l1ball(std::vector<float> &v) {
    std::vector<float> u(v);
    for(size_t i = 0; i<v.size(); i++) {
	if (u[i]<0) u[i] = -u[i];
    }
    euclidean_proj_simplex(u);
    for(size_t i = 0; i<v.size(); i++) {
	v[i] = u[i]*sign(v[i]);
    }
}

long long choose(long long n, long long k) {
    if (k > n) return 0;
    if (k<0) return 0;
    if (k==n) return 1;
    if (k==0 && n!=0) return 1;
    long long r = 1;
    for (long long d = 1; d <= k; ++d) {
      r *= n--;
      r /= d;
    }
    return r;
}
  
struct boosting {
    int N;
    float gamma;
    string* alg;
    vw* all;
    std::vector<std::vector<long long> > C;
    std::vector<float> alpha;
    std::vector<float> v;
    int t;
    bool discrete;
};
  
//---------------------------------------------------
// Online Boost-by-Majority (BBM)
// --------------------------------------------------
template <bool is_learn>
  void predict_or_learn(boosting& o, LEARNER::base_learner& base, example& ec) {
    label_data& ld = ec.l.simple;
    
    float final_prediction = 0;
    
    float s = 0;
    float u = ld.weight;

    if (is_learn) o.t++;
      
    for (int i = 0; i < o.N; i++)
    {
      if (is_learn) {
        
        float k = floorf((float)(o.N-i-s)/2);
        long long c;
        if (o.N-(i+1)<0) c=0;
        else if (k > o.N-(i+1)) c=0;
        else if (k < 0) c = 0;
        else if (o.C[o.N-(i+1)][(long long)k] != -1) 
	    c=o.C[o.N-(i+1)][(long long)k];
        else { c = choose(o.N-(i+1),k); o.C[o.N-(i+1)][(long long)k] = c; }
        
        float w = c * pow((double)(0.5+o.gamma),
	    (double)k) * pow((double)0.5-o.gamma,(double)(o.N-(i+1)-k));
          
        // update ld.weight, weight for learner i (starting from 0)
	ld.weight = u * w;
        
        base.predict(ec, i);

	// ec.pred.scalar is now ith learner prediction on this example
	if (o.discrete) s += ld.label * sign(ec.pred.scalar);
	else s += ld.label * ec.pred.scalar;

	if (o.discrete) final_prediction += sign(ec.pred.scalar);
	else final_prediction += ec.pred.scalar;

        base.learn(ec, i);
      }
      else {
          base.predict(ec, i);
	  if (o.discrete) final_prediction += sign(ec.pred.scalar);
          else final_prediction += ec.pred.scalar;
      }
    }	
    
    ld.weight = u;
    ec.pred.scalar = sign(final_prediction);

    if (ld.label == ec.pred.scalar)
      ec.loss = 0.;
    else
      ec.loss = ld.weight;
}

//-----------------------------------------------------------------
// Logistic boost
//-----------------------------------------------------------------
template <bool is_learn>
  void predict_or_learn_logistic(boosting& o, LEARNER::base_learner& base, example& ec) {
    label_data& ld = ec.l.simple;

    float final_prediction = 0;

    float s = 0;
    float u = ld.weight;

    if (is_learn) o.t++;
    float eta = 4 / sqrt(o.t);

    for (int i = 0; i < o.N; i++) {

      if (is_learn) {
        float w = 1 / (1 + exp(s));

        ld.weight = u * w;

        base.predict(ec, i);
	float z;
	if (o.discrete) z = ld.label * sign(ec.pred.scalar);
	else z = ld.label * ec.pred.scalar;

        s += z * o.alpha[i];

	// if ld.label * ec.pred.scalar < 0, learner i made a mistake

	if (o.discrete) {
	    final_prediction += sign(ec.pred.scalar) * o.alpha[i];
	}
	else 
	    final_prediction += ec.pred.scalar * o.alpha[i];

	// update alpha
        o.alpha[i] += eta * z / (1 + exp(s));
	if (o.alpha[i] > 2.) o.alpha[i] = 2;
	if (o.alpha[i] < -2.) o.alpha[i] = -2;

        base.learn(ec, i);

      }
      else {
        base.predict(ec, i);
	if (o.discrete)
            final_prediction += sign(ec.pred.scalar) * o.alpha[i];
	else 
	    final_prediction += ec.pred.scalar * o.alpha[i];
      }
    }

    ld.weight = u;
    ec.pred.scalar = sign(final_prediction);

    if (ld.label == ec.pred.scalar)
      ec.loss = 0.;
    else
      ec.loss = ld.weight;
}

//-----------------------------------------------------------------
// Smooth boost: Second algorithm in Chen, Lin, and Lu
// based on learning with expert advice
//-----------------------------------------------------------------
template <bool is_learn>
  void predict_or_learn_smooth(boosting& o, LEARNER::base_learner& base, example& ec) {
    label_data& ld = ec.l.simple;

    float final_prediction = 0;

    float s = 0;
    float u = ld.weight;

    if (is_learn) o.t++;

    for (int i = 0; i < o.N; i++)
    {
      if (is_learn) {
        float w;

        float theta = o.gamma / (2 + o.gamma);
        float ss = s - i * theta;
        if (ss > 0)
            w = pow((double)(1-o.gamma), (double)(ss/2));
        else
            w = 1;

        ld.weight = u * w;

        base.predict(ec, i);

	if (o.discrete) s += ld.label * sign(ec.pred.scalar);
        else s += ld.label * ec.pred.scalar;

	if (o.discrete) final_prediction += sign(ec.pred.scalar);
	else final_prediction += ec.pred.scalar;

        base.learn(ec, i);
      }
      else {
        base.predict(ec, i);

	if (o.discrete) final_prediction += sign(ec.pred.scalar);
	else final_prediction += ec.pred.scalar;
      }
    }

    ld.weight = u;
    ec.pred.scalar = sign(final_prediction);

    if (ld.label == ec.pred.scalar)
      ec.loss = 0.;
    else
      ec.loss = ld.weight;
}


//-----------------------------------------------------------------
// Algorithm 1 in Chen, Lin, and Lu
//-----------------------------------------------------------------
template <bool is_learn>
  void predict_or_learn_OCP(boosting& o, LEARNER::base_learner& base, example& ec) {
    label_data& ld = ec.l.simple;

    float final_prediction = 0;
    float u = ld.weight;

    // compute the weighted prediction
    for (int i = 0; i < o.N; i++) {
        base.predict(ec, i);
	if (o.discrete) 
	    final_prediction += o.alpha[i] * sign(ec.pred.scalar);
        else 
	    final_prediction += o.alpha[i] * ec.pred.scalar;
    }

    if (is_learn) {
	float theta = o.gamma / (2 + o.gamma);
	float z = 0, w = 1.0;

	o.t++;
	float eta = pow(o.N*o.t,-0.5);

	bool alpha_change = false;

        for (int i = 0; i < o.N; i++) {

	    base.predict(ec, i);

	    if (ld.label * final_prediction < theta) {
		alpha_change = true;
		if (o.discrete) 
		    o.alpha[i] += eta * ld.label * sign(ec.pred.scalar);
		else 
		    o.alpha[i] += eta * ld.label * ec.pred.scalar;
	    }

	    ld.weight = u * w;
            base.learn(ec, i);

	    if (o.discrete) 
		z += z + ld.label * sign(ec.pred.scalar) - theta;
	    else 
		z += z + ld.label * ec.pred.scalar - theta;

	    w = pow((double)(1-o.gamma),(double) z/2);
	    if (w>1) w=1;
        }
        if (alpha_change) euclidean_proj_simplex(o.alpha);
    }

    ld.weight = u;
    ec.pred.scalar = sign(final_prediction);

    if (ld.label == ec.pred.scalar)
      ec.loss = 0.;
    else
      ec.loss = ld.weight;
}

/*-------------------------------------------------------*/
template <bool is_learn>
  void predict_or_learn_adaptive(boosting& o, LEARNER::base_learner& base, example& ec) {
    label_data& ld = ec.l.simple;

#ifdef DEBUG
    for(int i = 0; i < o.N; i++) {
	cerr << o.v[i] << " ";
    }
    cerr << endl;
#endif

    float final_prediction = 0, partial_prediction = 0;

    float s = 0;
    float v_normalization = 0, v_partial_sum = 0;
    float u = ld.weight;

    if (is_learn) o.t++;
    float eta = 4 / sqrt(o.t);

    float stopping_point = frand48();

    for (int i = 0; i < o.N; i++) {

      if (is_learn) {
        float w = 1 / (1 + exp(s));

        ld.weight = u * w;

        base.predict(ec, i);
	float z;

	if (o.discrete) z = ld.label * sign(ec.pred.scalar);
	else z = ld.label * ec.pred.scalar;

        s += z * o.alpha[i];

        if (v_partial_sum <= stopping_point) {
	    if (o.discrete)
	        final_prediction += sign(ec.pred.scalar) * o.alpha[i];
	    else
	        final_prediction += ec.pred.scalar * o.alpha[i];
	}

	if (o.discrete) partial_prediction += sign(ec.pred.scalar) * o.alpha[i];
	else partial_prediction += ec.pred.scalar * o.alpha[i];

	v_partial_sum += o.v[i];

	// update v, exp(-1) = 0.36788
	if (ld.label * partial_prediction < 0) {
	    o.v[i] *= 0.36788;
	    // cerr << "Updating v[" << i << "]=" << o.v[i] << endl;
	}
	v_normalization += o.v[i];

	// update alpha
        o.alpha[i] += eta * z / (1 + exp(s));
	if (o.alpha[i] > 2.) o.alpha[i] = 2;
	if (o.alpha[i] < -2.) o.alpha[i] = -2;

        base.learn(ec, i);

      }
      else {
        base.predict(ec, i);
        if (v_partial_sum <= stopping_point) {
	    if (o.discrete)
                final_prediction += sign(ec.pred.scalar) * o.alpha[i];
	    else 
                final_prediction += ec.pred.scalar * o.alpha[i];
	}
	else { 
	    // stopping at learner i
	    break;
	}
	v_partial_sum += o.v[i];
      }
    }

    // normalize v vector in training
    if (is_learn) {
        for(int i = 0; i < o.N; i++) {
	    if (v_normalization)
	        o.v[i] /= v_normalization;
        }
    }

    ld.weight = u;
    ec.pred.scalar = sign(final_prediction);

    if (ld.label == ec.pred.scalar)
      ec.loss = 0.;
    else
      ec.loss = ld.weight;
}


void save_load_sampling(boosting &o, io_buf &model_file, bool read, bool text) 
{
  if (model_file.files.size() == 0)
    return;
  stringstream os;
  os << "boosts " << o.N << endl;
  const char* buff = os.str().c_str();
  bin_text_read_write_fixed(model_file, (char *) &(o.N),  sizeof(o.N), "", read, buff, strlen(buff), text);
  
  if (read) {
    o.alpha.resize(o.N);
    o.v.resize(o.N);
  }
  
  for (int i = 0; i < o.N; i++)
    if (read)
      {
	float f;
	bin_read_fixed(model_file, (char *) &f,  sizeof(f), "");
	o.alpha[i] = f;
      }
    else
      {
	stringstream os2;
	os2 << "alpha " << o.alpha[i] << endl;
	const char* buff2 = os.str().c_str();
	bin_text_write_fixed(model_file, (char *) &(o.alpha[i]),  sizeof(o.alpha[i]), buff2, strlen(buff2), text);		  
      }

   for (int i = 0; i < o.N; i++)
    if (read)
      {
	float f;
	bin_read_fixed(model_file, (char *) &f,  sizeof(f), "");
	o.v[i] = f;
      }
    else
      {
	stringstream os2;
	os2 << "v " << o.v[i] << endl;
	const char* buff2 = os.str().c_str();
	bin_text_write_fixed(model_file, (char *) &(o.v[i]),  sizeof(o.v[i]), buff2, strlen(buff2), text);		  
      }

   
   if (read) {
	cerr << "Loading alpha and v: " << endl;
   }
   else {
	cerr << "Saving alpha and v, current weighted_examples = " << o.all->sd->weighted_examples << endl;
   }
   for (int i = 0; i < o.N; i++) {
	cerr << o.alpha[i] << " " << o.v[i] << endl;
   }
   cerr << endl;
}

/*-------------------------------------------------------*/
  
void finish(boosting& o) {
    delete o.alg;
    o.C.~vector();
    o.alpha.~vector();
}
  
void return_example(vw& all, boosting& a, example& ec) {
    output_and_account_example(all, ec);
    VW::finish_example(all,&ec);
}

void save_load(boosting &o, io_buf &model_file, bool read, bool text) 
{
  if (model_file.files.size() == 0)
    return;
  stringstream os;
  os << "boosts " << o.N << endl;
  const char* buff = os.str().c_str();
  bin_text_read_write_fixed(model_file, (char *) &(o.N),  sizeof(o.N), "", read, buff, strlen(buff), text);
  
  if (read) {
    o.alpha.resize(o.N);
  }
  
  for (int i = 0; i < o.N; i++)
    if (read)
      {
	float f;
	bin_read_fixed(model_file, (char *) &f,  sizeof(f), "");
	o.alpha[i] = f;
      }
    else
      {
	stringstream os2;
	os2 << "alpha " << o.alpha[i] << endl;
	const char* buff2 = os.str().c_str();
	bin_text_write_fixed(model_file, (char *) &(o.alpha[i]),  sizeof(o.alpha[i]), buff2, strlen(buff2), text);		  
      }

   if (read) {
	cerr << "Loading alpha: " << endl;
   }
   else {
	cerr << "Saving alpha, current weighted_examples = " << o.all->sd->weighted_examples << endl;
   }
   for (int i = 0; i < o.N; i++) {
	cerr << o.alpha[i] << " " << endl;
   }
   cerr << endl;
}

LEARNER::base_learner* boosting_setup(vw& all)
{
    if (missing_option<size_t,true>(all,"boosting",
	"Online boosting with <N> weak learners"))
        return NULL;
    new_options(all, "Boosting Options")
      ("gamma", po::value<float>()->default_value(0.1), 
	 "weak learner's edge (=0.1)")
      ("discrete", "use hard predictions to update s")
      ("alg", po::value<string>()->default_value("BBM"),
	 "specify the boosting algorithm: BBM (default), logistic, smooth, OCP, adaptive");
    // Description of options:
    // "BBM" implements online BBM (Algorithm 1 in BLK'15)
    // "logistic" implements AdaBoost.OL.W (importance weighted version 
    // 	    of Algorithm 2 in BLK'15)
    // "adaptive" implements AdaBoost.OL (Algorithm 2 in BLK'15, 
    // 	    using sampling rather than importance weighting)
    // "smooth" implements Smooth boost, the second algorithm 
    //     in CLL'12 based on learning with expert advice
    // "OCP" implements Algorithm 1 in CLL'12
    add_options(all);
    
    boosting& data = calloc_or_die<boosting>();
    data.N = (uint32_t)all.vm["boosting"].as<size_t>();
    cerr << "Number of weak learners = " << data.N << endl;
    data.gamma = all.vm["gamma"].as<float>();
    cerr << "Gamma = " << data.gamma << endl;
    data.discrete = all.vm.count("discrete");
    string* temp = new string;
    *temp = all.vm["alg"].as<string>();
    data.alg = temp;

    data.C = std::vector<std::vector<long long> >(data.N, 
	std::vector<long long>(data.N,-1));

    data.t = 0;
    
    data.all = &all;
    data.alpha = std::vector<float>(data.N,0);
    data.v = std::vector<float>(data.N,1);


    cerr << "Discrete: " << data.discrete << endl;

    learner<boosting>* l;
    if (*data.alg == "BBM") {

        l = &init_learner<boosting>(&data, setup_base(all), 
	    predict_or_learn<true>, 
	    predict_or_learn<false>, data.N);
    } 
    else if (*data.alg == "OCP") {
        for(int i=0;i<data.N;i++) data.alpha[i]=pow(data.N,-1);

        l = &init_learner<boosting>(&data, setup_base(all), 
	    predict_or_learn_OCP<true>, 
	    predict_or_learn_OCP<false>, data.N);
	l->set_save_load(save_load);
    } 
    else if (*data.alg == "logistic") {

        l = &init_learner<boosting>(&data, setup_base(all), 
	    predict_or_learn_logistic<true>, 
	    predict_or_learn_logistic<false>, data.N);
	l->set_save_load(save_load);
    } 
    else if (*data.alg == "smooth") {
        l = &init_learner<boosting>(&data, setup_base(all), 
	    predict_or_learn_smooth<true>, 
	    predict_or_learn_smooth<false>, data.N);
    } 
    else if (*data.alg == "adaptive") {
        l = &init_learner<boosting>(&data, setup_base(all), 
	    predict_or_learn_adaptive<true>, 
	    predict_or_learn_adaptive<false>, data.N);
	l->set_save_load(save_load_sampling);
    }
    else {
	cout << "Unrecognized boosting algorithm: \'" << *data.alg <<
            "\' Bailing!" << endl;
        throw exception();
    }

    l->set_finish(finish);
    l->set_finish_example(return_example);

    return make_base(*l);
}
