/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.    Released under a BSD (revised)
license as described in the file LICENSE.
*/
#include <string>
#include "gd.h"
#include "vw.h"
#include "rand48.h"
#include <math.h>

using namespace std;
using namespace LEARNER;

#define NORM2 (m+1)

struct update_data {
    struct OjaNewton *ON;
    float g;
    float sketch_cnt;
    float norm2_x;
    float *Zx;
    float *AZx;
    float *delta;
    float bdelta;
    float prediction;
};

struct OjaNewton {
    vw* all;
    int m;
    int epoch_size;
    float alpha;
    int cnt;
    int t;

    float *ev;
    float *b;
    float *D;
    float **A;
	float **K;

    float *zv;
    float *vv;
    float *tmp;

    example **buffer;
    float *weight_buffer;
    struct update_data data;

    float learning_rate_cnt;
    bool normalize;
    bool random_init;

  void initialize_Z(parameters& weights)
  { 
    uint32_t length = 1 << all->num_bits;
    if (normalize) { // initialize normalization part
      for (uint32_t i = 0; i < length; i++)
	(&(weights.strided_index(i)))[NORM2] = 0.1f;
    }
    if(!random_init) {
      // simple initialization
      for (int i = 1; i <= m; i++)
	(&(weights.strided_index(i)))[i] = 1.f;
    }
    else {
      // more complicated initialization: orthgonal basis of a random matrix
      
      const double PI2 = 2.f * 3.1415927f;

      for (uint32_t i = 0; i < length; i++)
	{
	  weight& w = weights.strided_index(i);
	  for (int j = 1; j <= m; j++)
	    {
	      float r1 = merand48(all->random_state);
	      float r2 = merand48(all->random_state);
	      (&w)[j] = sqrt(-2.f * log(r1)) * (float)cos(PI2 * r2);
	    }
	}
    }
    
    // Gram-Schmidt
    for (int j = 1; j <= m; j++) {
      for (int k = 1; k <= j - 1; k++) {
	double tmp = 0;
	
	for (uint32_t i = 0; i < length; i++)
	  tmp += (&(weights.strided_index(i)))[j] * (&(weights.strided_index(i)))[k];
	for (uint32_t i = 0; i < length; i++)
	  (&(weights.strided_index(i)))[j] -= (float)tmp *(&(weights.strided_index(i)))[k];
      }
      double norm = 0;
      for (uint32_t i = 0; i < length; i++)
	norm += (&(weights.strided_index(i)))[j] * (&(weights.strided_index(i)))[j];
      norm = sqrt(norm);
      for (uint32_t i = 0; i < length; i++)
	(&(weights.strided_index(i)))[j] /= (float)norm;
    }
  }
  
  void compute_AZx()
    {
        for (int i = 1; i <= m; i++) {
            data.AZx[i] = 0;
            for (int j = 1; j <= i; j++) {
                data.AZx[i] += A[i][j] * data.Zx[j];
            }
        }
    }

    void update_eigenvalues()
    {
        for (int i = 1; i <= m; i++) {
            float gamma = fmin(learning_rate_cnt / t, 1.f);
            float tmp = data.AZx[i] * data.sketch_cnt;

            if (t == 1) {
                ev[i] = gamma * tmp * tmp;
            }
            else {
                ev[i] = (1 - gamma) * t * ev[i] / (t - 1) + gamma * t * tmp * tmp;
            }
        }
    }

    void compute_delta()
    {
        data.bdelta = 0;
        for (int i = 1; i <= m; i++) {
            float gamma = fmin(learning_rate_cnt / t, 1.f);
            
	    // if different learning rates are used
            /*data.delta[i] = gamma * data.AZx[i] * data.sketch_cnt;
            for (int j = 1; j < i; j++) {
                data.delta[i] -= A[i][j] * data.delta[j];
            }
            data.delta[i] /= A[i][i];*/

	    // if a same learning rate is used
            data.delta[i] = gamma * data.Zx[i] * data.sketch_cnt;

            data.bdelta += data.delta[i] * b[i];
        }
    }

    void update_K()
    {
        float tmp = data.norm2_x * data.sketch_cnt * data.sketch_cnt;
        for (int i = 1; i <= m; i++) {
            for (int j = 1; j <= m; j++) {
                K[i][j] += data.delta[i] * data.Zx[j] * data.sketch_cnt;
                K[i][j] += data.delta[j] * data.Zx[i] * data.sketch_cnt;
                K[i][j] += data.delta[i] * data.delta[j] * tmp;
            }
        }
    }

    void update_A()
    {
        for (int i = 1; i <= m; i++) {

            for (int j = 1; j < i; j++) {
                zv[j] = 0;
                for (int k = 1; k <= i; k++) {
                    zv[j] += A[i][k] * K[k][j];
                }
            }

            for (int j = 1; j < i; j++) {
                vv[j] = 0;
                for (int k = 1; k <= j; k++) {
                    vv[j] += A[j][k] * zv[k];
                }
            }

            for (int j = 1; j < i; j++) {
                for (int k = j; k < i; k++) {
                    A[i][j] -= vv[k] * A[k][j];
                }
            }

            float norm = 0;
            for (int j = 1; j <= i; j++) {
	        float temp = 0;
                for (int k = 1; k <= i; k++) {
                    temp += K[j][k] * A[i][k];
                }
                norm += A[i][j]*temp;
            }
            norm = sqrtf(norm);

            for (int j = 1; j <= i; j++) {
                A[i][j] /= norm;
            }
        }
    }

    void update_b()
    {
        for (int j = 1; j <= m; j++) {
            float tmp = 0;
            for (int i = j; i <= m; i++) {
                tmp += ev[i] * data.AZx[i] * A[i][j] / (alpha * (alpha + ev[i]));
            }
            b[j] += tmp * data.g;
        }
    }

    void update_D()
    {
        for (int j = 1; j <= m; j++) {
            float scale = fabs(A[j][j]);
            for (int i = j+1; i <= m; i++) 
                scale = fmin(fabs(A[i][j]), scale);
            if (scale < 1e-10) continue;
            for (int i = 1; i <= m; i++) {
                A[i][j] /= scale;
                K[j][i] *= scale;
                K[i][j] *= scale;
            }
            b[j] /= scale;
            D[j] *= scale;
            //printf("D[%d] = %f\n", j, D[j]);
        }
    }

	void check()
    {
    	double max_norm = 0;
        for (int i = 1; i <= m; i++)
            for (int j = i; j <= m ;j++)
                max_norm = fmax(max_norm, fabs(K[i][j]));
	//printf("|K| = %f\n", max_norm);
        if (max_norm < 1e7) return;
        
        // implicit -> explicit representation
        // printf("begin conversion: t = %d, norm(K) = %f\n", t, max_norm);
 
        // first step: K <- AKA'
        
        // K <- AK
        for (int j = 1; j <= m; j++) {
            memset(tmp, 0, sizeof(double) * (m+1));
            
            for (int i = 1; i <= m; i++) {
                for (int h = 1; h <= m; h++) {
                    tmp[i] += A[i][h] * K[h][j];
                }
            } 

            for (int i = 1; i <= m; i++)
                K[i][j] = tmp[i];
        }
        // K <- KA'
        for (int i = 1; i <= m; i++) {
            memset(tmp, 0, sizeof(double) * (m+1));
            
            for (int j = 1; j <= m; j++)
                for (int h = 1; h <= m; h++)
                    tmp[j] += K[i][h] * A[j][h];

            for (int j = 1; j <= m; j++) { 
                K[i][j] = tmp[j];
	    }
        }
        
	//second step: w[0] <- w[0] + (DZ)'b, b <- 0.

        uint32_t length = 1 << all->num_bits;
		for (uint32_t i = 0; i < length; i++) {
			weight& w = all->weights.strided_index(i);
			for (int j = 1; j <= m; j++)
				w += (&w)[j] * b[j] * D[j];
		}

        memset(b, 0, sizeof(double) * (m+1));

        //third step: Z <- ADZ, A, D <- Identity

	    //double norm = 0;
        for (uint32_t i = 0; i < length; ++i) {
            memset(tmp, 0, sizeof(float) * (m+1));
			weight& w = all->weights.strided_index(i);
			for (int j = 1; j <= m; j++)
			{ for (int h = 1; h <= m; ++h)
			    tmp[j] += A[j][h] * D[h] * (&w)[h];
			}
            for (int j = 1; j <= m; ++j) {
		      //norm = max(norm, fabs(tmp[j]));
				(&w)[j] = tmp[j];
	        }
        }
        //printf("|Z| = %f\n", norm);

        for (int i = 1; i <= m; i++) {
            memset(A[i], 0, sizeof(double) * (m+1));
            D[i] = 1;
            A[i][i] = 1;
        }
    }
};

void keep_example(vw& all, OjaNewton& ON, example& ec) {
    output_and_account_example(all, ec);
}

void finish(OjaNewton& ON) {
    free(ON.ev);
    free(ON.b);
    free(ON.D);
    free(ON.buffer);
    free(ON.weight_buffer);
    free(ON.zv);
    free(ON.vv);
    free(ON.tmp);
   
    for (int i = 1; i <= ON.m; i++) {
        free(ON.A[i]);
	free(ON.K[i]);
    }
    free(ON.A);
    free(ON.K);
    
    free(ON.data.Zx);
    free(ON.data.AZx);
    free(ON.data.delta);
}

void make_pred(update_data& data, float x, float& wref) {
    int m = data.ON->m;
    float* w = &wref;

    if (data.ON->normalize) {
        x /= sqrt(w[NORM2]);
    }

    data.prediction += w[0] * x;
    for (int i = 1; i <= m; i++) {
        data.prediction += w[i] * x * data.ON->D[i] * data.ON->b[i];
    }
}

void predict(OjaNewton& ON, base_learner&, example& ec) {
    ON.data.prediction = 0;
    GD::foreach_feature<update_data, make_pred>(*ON.all, ec, ON.data);
    ec.partial_prediction = (float)ON.data.prediction;
    ec.pred.scalar = GD::finalize_prediction(ON.all->sd, ec.partial_prediction);
}

void update_Z_and_wbar(update_data& data, float x, float& wref) {   
    float* w = &wref;
    int m = data.ON->m;
    if (data.ON->normalize) x /= sqrt(w[NORM2]);
    float s = data.sketch_cnt * x;

    for (int i = 1; i <= m; i++) {
        w[i] += data.delta[i] * s / data.ON->D[i];
    }
    w[0] -= s * data.bdelta;
}

void compute_Zx_and_norm(update_data& data, float x, float& wref) {
    float* w = &wref;
    int m = data.ON->m;
    if (data.ON->normalize) x /= sqrt(w[NORM2]);

    for (int i = 1; i <= m; i++) {
        data.Zx[i] += w[i] * x * data.ON->D[i];
    }
    data.norm2_x += x * x;
}

void update_wbar_and_Zx(update_data& data, float x, float& wref) {
    float* w = &wref;
    int m = data.ON->m;
    if (data.ON->normalize) x /= sqrt(w[NORM2]);

    float g = data.g * x;

    for (int i = 1; i <= m; i++) {
        data.Zx[i] += w[i] * x * data.ON->D[i];
    }
    w[0] -= g / data.ON->alpha;
}


void update_normalization(update_data& data, float x, float& wref) {
    float* w = &wref;
    int m = data.ON->m;
    
    w[NORM2] += x * x * data.g * data.g;
}

void learn(OjaNewton& ON, base_learner& base, example& ec) {
    assert(ec.in_use);

    // predict
    predict(ON, base, ec);

    update_data& data = ON.data;
    data.g = ON.all->loss->first_derivative(ON.all->sd, ec.pred.scalar, ec.l.simple.label)*ec.l.simple.weight;
    data.g /= 2; // for half square loss
    
    if(ON.normalize) GD::foreach_feature<update_data, update_normalization>(*ON.all, ec, data);

    ON.buffer[ON.cnt] = &ec;
    ON.weight_buffer[ON.cnt++] = data.g / 2;

    if (ON.cnt == ON.epoch_size) {
        for (int k = 0; k < ON.epoch_size; k++, ON.t++) {
            example& ex = *(ON.buffer[k]);
            data.sketch_cnt = ON.weight_buffer[k];

            data.norm2_x = 0;
            memset(data.Zx, 0, sizeof(float)* (ON.m+1));
            GD::foreach_feature<update_data, compute_Zx_and_norm>(*ON.all, ex, data);
            ON.compute_AZx();

            ON.update_eigenvalues();
            ON.compute_delta();

            ON.update_K();

            GD::foreach_feature<update_data, update_Z_and_wbar>(*ON.all, ex, data);
        }

        ON.update_A();
        //ON.update_D();
    }

    memset(data.Zx, 0, sizeof(float)* (ON.m+1));
    GD::foreach_feature<update_data, update_wbar_and_Zx>(*ON.all, ec, data);
    ON.compute_AZx();

    ON.update_b();
    ON.check();

    if (ON.cnt == ON.epoch_size) {
        ON.cnt = 0;
        for (int k = 0; k < ON.epoch_size; k++) {
            VW::finish_example(*ON.all, ON.buffer[k]);
        }
    }
}


void save_load(OjaNewton& ON, io_buf& model_file, bool read, bool text) {
    vw& all = *ON.all;
    if (read) {
        initialize_regressor(all);
        ON.initialize_Z(all.weights);
    }

    if (model_file.files.size() > 0) {
        bool resume = all.save_resume;
	stringstream msg;
	msg << ":"<< resume <<"\n";
        bin_text_read_write_fixed(model_file, (char *)&resume, sizeof (resume), "", read, msg, text);


        if (resume)
            GD::save_load_online_state(all, model_file, read, text);
        else
            GD::save_load_regressor(all, model_file, read, text);
    }
}

base_learner* OjaNewton_setup(vw& all)
{
    if (missing_option(all, false, "OjaNewton", "Online Newton with Oja's Sketch"))
        return nullptr;

    new_options(all, "OjaNewton options")
        ("sketch_size", po::value<int>(), "size of sketch")
        ("epoch_size", po::value<int>(), "size of epoch")
        ("alpha", po::value<float>(), "mutiplicative constant for indentiy")
        ("alpha_inverse", po::value<float>(), "one over alpha, similar to learning rate")
        ("learning_rate_cnt", po::value<float>(), "constant for the learning rate 1/t")
        ("normalize", po::value<bool>(), "normalize the features or not")
	("random_init", po::value<bool>(), "randomize initialization of Oja or not");
    add_options(all);

    po::variables_map& vm = all.vm;

    OjaNewton& ON = calloc_or_throw<OjaNewton>();
    ON.all = &all;

    if (vm.count("sketch_size"))
        ON.m = vm["sketch_size"].as<int>();
    else
        ON.m = 10;

    if (vm.count("epoch_size"))
        ON.epoch_size = vm["epoch_size"].as<int>();
    else
        ON.epoch_size = 1;

    if (vm.count("alpha"))
        ON.alpha = vm["alpha"].as<float>();
    else
        ON.alpha = 1.f;

    if (vm.count("alpha_inverse"))
        ON.alpha = 1.f / vm["alpha_inverse"].as<float>();

    if (vm.count("learning_rate_cnt"))
        ON.learning_rate_cnt = vm["learning_rate_cnt"].as<float>();
    else
        ON.learning_rate_cnt = 2;

    if (vm.count("normalize"))
        ON.normalize = vm["normalize"].as<bool>();
    else
        ON.normalize = true;
    
    if (vm.count("random_init"))
        ON.random_init = vm["random_init"].as<bool>();
    else
        ON.random_init = true;
    
    ON.cnt = 0;
    ON.t = 1;

    ON.ev = calloc_or_throw<float>(ON.m+1);
    ON.b = calloc_or_throw<float>(ON.m+1);
    ON.D = calloc_or_throw<float>(ON.m+1);
    ON.A = calloc_or_throw<float*>(ON.m+1);
    ON.K = calloc_or_throw<float*>(ON.m+1);
    for (int i = 1; i <= ON.m; i++) {
        ON.A[i] = calloc_or_throw<float>(ON.m+1);
        ON.K[i] = calloc_or_throw<float>(ON.m+1);
        ON.A[i][i] = 1;
        ON.K[i][i] = 1;
        ON.D[i] = 1;
    }

    ON.buffer = calloc_or_throw<example*>(ON.epoch_size);
    ON.weight_buffer = calloc_or_throw<float>(ON.epoch_size);
    
    ON.zv = calloc_or_throw<float>(ON.m+1);
    ON.vv = calloc_or_throw<float>(ON.m+1);
    ON.tmp = calloc_or_throw<float>(ON.m+1);

    ON.data.ON = &ON;
    ON.data.Zx = calloc_or_throw<float>(ON.m+1);
    ON.data.AZx = calloc_or_throw<float>(ON.m+1);
    ON.data.delta = calloc_or_throw<float>(ON.m+1);

    all.weights.stride_shift((uint32_t)ceil(log2(ON.m + 2)));

    learner<OjaNewton>& l = init_learner(&ON, learn, all.weights.stride());

    l.set_predict(predict);
    l.set_save_load(save_load);
    l.set_finish_example(keep_example);
    l.set_finish(finish);
    return make_base(l);
}
