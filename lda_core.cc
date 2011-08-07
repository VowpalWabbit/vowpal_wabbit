/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */
#include <fstream>
#include <float.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "gd.h"
#include "lda.h"
#include "cache.h"
#include "multisource.h"
#include "simple_label.h"
#include "delay_ring.h"

#define MINEIRO_SPECIAL
#ifdef MINEIRO_SPECIAL

namespace {

inline float 
fastlog2 (float x)
{
  union { float f; uint32_t i; } vx = { x };
  union { uint32_t i; float f; } mx = { (vx.i & 0x007FFFFF) | (0x7e << 23) };
  float y = vx.i;
  y *= 1.0f / (1 << 23);

  return 
    y - 124.22544637f - 1.498030302f * mx.f - 1.72587999f / (0.3520887068f + mx.f);
}

inline float
fastlog (float x)
{
  return 0.69314718f * fastlog2 (x);
}

inline float
fastpow2 (float p)
{
  float offset = (p < 0) ? 1.0f : 0.0f;
  float clipp = (p < -126) ? -126.0f : p;
  int w = clipp;
  float z = clipp - w + offset;
  union { uint32_t i; float f; } v = { (1 << 23) * (clipp + 121.2740838f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z) };

  return v.f;
}
 
inline float
fastexp (float p)
{
  return fastpow2 (1.442695040f * p);
}

inline float
fastpow (float x,
         float p)
{
  return fastpow2 (p * fastlog2 (x));
}

inline float
fastlgamma (float x)
{
  float logterm = fastlog (x * (1.0f + x) * (2.0f + x));
  float xp3 = 3.0f + x;

  return 
    -2.081061466f - x + 0.0833333f / xp3 - logterm + (2.5f + x) * fastlog (xp3);
}

inline float
fastdigamma (float x)
{
  float twopx = 2.0f + x;
  float logterm = fastlog (twopx);

  return - (1.0f + 2.0f * x) / (x * (1.0f + x)) 
         - (13.0f + 6.0f * x) / (12.0f * twopx * twopx) 
         + logterm;
}

#define log fastlog
#define exp fastexp
#define powf fastpow
#define mydigamma fastdigamma
#define mylgamma fastlgamma

#if defined(__SSE2__) && !defined(VW_LDA_NO_SSE)

#include <emmintrin.h>

typedef __m128 v4sf;
typedef __m128i v4si;

#define v4si_to_v4sf _mm_cvtepi32_ps
#define v4sf_to_v4si _mm_cvttps_epi32

#define v4sfl(x) ((const v4sf) { (x), (x), (x), (x) })
#define v2dil(x) ((const v4si) { (x), (x) })
#define v4sil(x) v2dil((((unsigned long long) (x)) << 32) | (x))

typedef union { v4sf f; float array[4]; } v4sfindexer;
#define v4sf_index(_findx, _findi)      \
  ({                                    \
     v4sfindexer _findvx = { _findx } ; \
     _findvx.array[_findi];             \
  })

inline v4sf
vfastpow2 (const v4sf p)
{
  v4sf ltzero = _mm_cmplt_ps (p, v4sfl (0.0f));
  v4sf offset = _mm_and_ps (ltzero, v4sfl (1.0f));
  v4sf lt126 = _mm_cmplt_ps (p, v4sfl (-126.0f));
  v4sf clipp = _mm_andnot_ps (lt126, p) + _mm_and_ps (lt126, v4sfl (-126.0f));
  v4si w = v4sf_to_v4si (clipp);
  v4sf z = clipp - v4si_to_v4sf (w) + offset;

  const v4sf c_121_2740838 = v4sfl (121.2740838f);
  const v4sf c_27_7280233 = v4sfl (27.7280233f);
  const v4sf c_4_84252568 = v4sfl (4.84252568f);
  const v4sf c_1_49012907 = v4sfl (1.49012907f);
  union { v4si i; v4sf f; } v = {
    v4sf_to_v4si (
      v4sfl (1 << 23) * 
      (clipp + c_121_2740838 + c_27_7280233 / (c_4_84252568 - z) - c_1_49012907 * z)
    )
  };

  return v.f;
}

inline v4sf
vfastexp (const v4sf p)
{
  const v4sf c_invlog_2 = v4sfl (1.442695040f);

  return vfastpow2 (c_invlog_2 * p);
}

inline v4sf
vfastlog2 (v4sf x)
{
  union { v4sf f; v4si i; } vx = { x };
  union { v4si i; v4sf f; } mx = { (vx.i & v4sil (0x007FFFFF)) | v4sil (0x3f000000) };
  v4sf y = v4si_to_v4sf (vx.i);
  y *= v4sfl (1.1920928955078125e-7f);

  const v4sf c_124_22551499 = v4sfl (124.22551499f);
  const v4sf c_1_498030302 = v4sfl (1.498030302f);
  const v4sf c_1_725877999 = v4sfl (1.72587999f);
  const v4sf c_0_3520087068 = v4sfl (0.3520887068f);

  return y - c_124_22551499
           - c_1_498030302 * mx.f 
           - c_1_725877999 / (c_0_3520087068 + mx.f);
}

inline v4sf
vfastlog (v4sf x)
{
  const v4sf c_0_69314718 = v4sfl (0.69314718f);

  return c_0_69314718 * vfastlog2 (x);
}

inline v4sf
vfastdigamma (v4sf x)
{
  v4sf twopx = v4sfl (2.0f) + x;
  v4sf logterm = vfastlog (twopx);

  return (v4sfl (-48.0f) + x * (v4sfl (-157.0f) + x * (v4sfl (-127.0f) - v4sfl (30.0f) * x))) /
         (v4sfl (12.0f) * x * (v4sfl (1.0f) + x) * twopx * twopx)
         + logterm;
}

void
vexpdigammify (float* gamma)
{
  unsigned int n = global.lda;
  float extra_sum = 0.0f;
  v4sf sum = v4sfl (0.0f);
  size_t i;

  for (i = 0; i < n && ((uintptr_t) (gamma + i)) % 16 > 0; ++i)
    { 
      extra_sum += gamma[i];
      gamma[i] = fastdigamma (gamma[i]);
    }

  for (; i + 4 < n; i += 4)
    { 
      v4sf arg = _mm_load_ps (gamma + i);
      sum += arg;
      arg = vfastdigamma (arg);
      _mm_store_ps (gamma + i, arg);
    }

  for (; i < n; ++i)
    { 
      extra_sum += gamma[i];
      gamma[i] = fastdigamma (gamma[i]);
    } 

  extra_sum += v4sf_index (sum, 0) + v4sf_index (sum, 1) +
               v4sf_index (sum, 2) + v4sf_index (sum, 3);
  extra_sum = fastdigamma (extra_sum);
  sum = v4sfl (extra_sum);

  for (i = 0; i < n && ((uintptr_t) (gamma + i)) % 16 > 0; ++i)
    { 
      gamma[i] = fmaxf (1e-10f, fastexp (gamma[i] - v4sf_index (sum, 0)));
    }

  for (; i + 4 < n; i += 4)
    { 
      v4sf arg = _mm_load_ps (gamma + i);
      arg -= sum;
      arg = vfastexp (arg);
      arg = _mm_max_ps (v4sfl (1e-10f), arg);
      _mm_store_ps (gamma + i, arg);
    }

  for (; i < n; ++i)
    {
      gamma[i] = fmaxf (1e-10f, fastexp (gamma[i] - v4sf_index (sum, 0)));
    } 
}

void 
vexpdigammify_2(float*       gamma, 
                const float* norm)
{
  size_t n = global.lda;
  size_t i;

  for (i = 0; i < n && ((uintptr_t) (gamma + i)) % 16 > 0; ++i)
    { 
      gamma[i] = fmaxf (1e-10f, fastexp (fastdigamma (gamma[i]) - norm[i]));
    }

  for (; i + 4 < n; i += 4)
    {
      v4sf arg = _mm_load_ps (gamma + i);
      arg = vfastdigamma (arg);
      v4sf vnorm = _mm_loadu_ps (norm + i);
      arg -= vnorm;
      arg = vfastexp (arg);
      arg = _mm_max_ps (v4sfl (1e-10f), arg);
      _mm_store_ps (gamma + i, arg);
    }

  for (; i < n; ++i)
    {
      gamma[i] = fmaxf (1e-10f, fastexp (fastdigamma (gamma[i]) - norm[i]));
    }
}

#define myexpdigammify vexpdigammify
#define myexpdigammify_2 vexpdigammify_2

#else
#warning "lda IS NOT using sse instructions"
#define myexpdigammify expdigammify
#define myexpdigammify_2 expdigammify_2

#endif // __SSE2__

} // end anonymous namespace

#else 

#include <boost/math/special_functions/digamma.hpp>
#include <boost/math/special_functions/gamma.hpp>

using namespace boost::math::policies;

#define mydigamma boost::math::digamma
#define mylgamma boost::math::lgamma
#define myexpdigammify expdigammify
#define myexpdigammify_2 expdigammify_2

#endif // MINEIRO_SPECIAL

size_t max_w = 0;

float decayfunc(float t, float old_t, float power_t) {
  float result = 1;
  for (float i = old_t+1; i <= t; i += 1)
    result *= (1-powf(i, -power_t));
  return result;
}

float decayfunc2(float t, float old_t, float power_t) 
{
  float power_t_plus_one = 1. - power_t;
  float arg =  - ( powf(t, power_t_plus_one) -
                   powf(old_t, power_t_plus_one));
  return exp ( arg
               / power_t_plus_one);
}

float decayfunc3(double t, double old_t, double power_t) 
{
  double power_t_plus_one = 1. - power_t;
  double logt = log(t);
  double logoldt = log(old_t);
  return (old_t / t) * exp(0.5*power_t_plus_one*(-logt*logt + logoldt*logoldt));
}

float decayfunc4(double t, double old_t, double power_t)
{
  if (power_t > 0.99)
    return decayfunc3(t, old_t, power_t);
  else
    return decayfunc2(t, old_t, power_t);
}

void expdigammify(float* gamma)
{
  float sum=0;
  for (size_t i = 0; i<global.lda; i++)
    {
      sum += gamma[i];
      gamma[i] = mydigamma(gamma[i]);
    }
  sum = mydigamma(sum);
  for (size_t i = 0; i<global.lda; i++)
    gamma[i] = fmax(1e-10, exp(gamma[i] - sum));
}

void expdigammify_2(float* gamma, float* norm)
{
  for (size_t i = 0; i<global.lda; i++)
    {
      gamma[i] = fmax(1e-10, exp(mydigamma(gamma[i]) - norm[i]));
    }
}

float average_diff(float* oldgamma, float* newgamma)
{
  float sum = 0.;
  for (size_t i = 0; i<global.lda; i++)
    sum += fabsf(oldgamma[i] - newgamma[i]);
  return sum;
}

// Returns E_q[log p(\theta)] - E_q[log q(\theta)].
float theta_kl(float* gamma)
{
  float Elogtheta[global.lda];
  float gammasum = 0;
  for (size_t k = 0; k < global.lda; k++) {
    Elogtheta[k] = mydigamma(gamma[k]);
    gammasum += gamma[k];
  }
  float digammasum = mydigamma(gammasum);
  gammasum = mylgamma(gammasum);
  float kl = -(global.lda*mylgamma(global.lda_alpha));
  kl += mylgamma(global.lda_alpha*global.lda) - gammasum;
  for (size_t k = 0; k < global.lda; k++) {
    Elogtheta[k] -= digammasum;
    kl += (global.lda_alpha - gamma[k]) * Elogtheta[k];
    kl += mylgamma(gamma[k]);
  }

  return kl;
}

float find_cw(float* u_for_w, float* v)
{
  float c_w = 0;
  for (size_t k =0; k<global.lda; k++)
    c_w += u_for_w[k]*v[k];

  return 1.f / c_w;
}

// Returns an estimate of the part of the variational bound that
// doesn't have to do with beta for the entire corpus for the current
// setting of lambda based on the document passed in. The value is
// divided by the total number of words in the document This can be
// used as a (possibly very noisy) estimate of held-out likelihood.
float lda_loop(float* v,weight* weights,example* ec, float power_t)
{
  float new_gamma[global.lda];
  float old_gamma[global.lda];
  
  for (size_t i = 0; i < global.lda; i++)
    {
      new_gamma[i] = 1.;
      old_gamma[i] = 0.;
    }
  size_t num_words =0;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
    num_words += ec->subsets[*i][1] - ec->subsets[*i][0];

  float xc_w = 0;
  float score = 0;
  float doc_length = 0;
  do
    {
      memcpy(v,new_gamma,sizeof(float)*global.lda);
      myexpdigammify(v);

      memcpy(old_gamma,new_gamma,sizeof(float)*global.lda);
      memset(new_gamma,0,sizeof(float)*global.lda);

      score = 0;
      size_t word_count = 0;
      doc_length = 0;
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
	{
	  feature *f = ec->subsets[*i][0];
	  for (; f != ec->subsets[*i][1]; f++)
	    {
	      float* u_for_w = &weights[(f->weight_index&global.thread_mask)+global.lda+1];
	      float c_w = find_cw(u_for_w,v);
	      xc_w = c_w * f->x;
              score += -f->x*log(c_w);
	      size_t max_k = global.lda;
	      for (size_t k =0; k<max_k; k++) {
		new_gamma[k] += xc_w*u_for_w[k];
	      }
	      word_count++;
              doc_length += f->x;
	    }
	}
      for (size_t k =0; k<global.lda; k++) {
	new_gamma[k] = new_gamma[k]*v[k]+global.lda_alpha;
      }
    }
  while (average_diff(old_gamma, new_gamma) > 0.1);

  ec->topic_predictions.erase();
  if (ec->topic_predictions.end_array - ec->topic_predictions.begin < (int)global.lda)
    reserve(ec->topic_predictions,global.lda);
  memcpy(ec->topic_predictions.begin,new_gamma,global.lda*sizeof(float));

  score += theta_kl(new_gamma);

  return score / doc_length;
}

struct index_triple {
  uint32_t document;
  feature f;
};

v_array<v_array<index_triple> > merge_set;

void merge_pair(v_array<index_triple>& source, v_array<index_triple>& dest)
{
  size_t dest_size = dest.index();
  size_t limit = source.index()+dest_size;
  if (dest.end_array - dest.begin < (int)limit)
    reserve(dest,limit);
  memmove(dest.begin+source.index(),dest.begin,dest_size*sizeof(index_triple));
  dest.end = dest.begin+limit;

  size_t old_index = source.index();
  size_t new_index = 0;
  
  for (index_triple* s=source.begin; s != source.end; s++)
    {
      while((old_index < limit) && (dest[old_index].f.weight_index < s->f.weight_index)) {
	dest[new_index++] = dest[old_index++];
      }
      dest[new_index++] = *s;
    }
  source.erase();
}

void merge()
{
  for (size_t j = merge_set.index()-1; j > 0;j--)
    if (merge_set[j].index()*2 > merge_set[j-1].index())
      {
	merge_pair(merge_set[j], merge_set[j-1]);
	merge_set.pop();
      }
    else
      break;
}

void merge_all()
{
  for (size_t j = merge_set.index()-1; j > 0;j--)
    {
      merge_pair(merge_set[j], merge_set[j-1]);
      merge_set.pop();
    }
}

void merge_in(example* ec, size_t document)
{
  size_t next_index = merge_set.index();
  if ((int)(next_index + ec->indices.index()) > merge_set.end_array-merge_set.begin)
    reserve(merge_set,next_index+ec->indices.index());
  merge_set.end = merge_set.begin+next_index+ec->indices.index();
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++)
    {
      feature* f = ec->subsets[*i][0];
      for (; f != ec->subsets[*i][1]; f++)
	{
	  index_triple temp = {document,*f};
	  push(merge_set[next_index], temp);
	}
      next_index++;
    }

  merge();
}

void start_lda(gd_thread_params t)
{
  regressor reg = t.reg;
  example* ec = NULL;

  double total_lambda[global.lda];

  for (size_t k = 0; k < global.lda; k++)
    total_lambda[k] = 0;
  size_t stride = global.stride;
  weight* weights = reg.weight_vectors[0];

  for (size_t i =0; i <= global.thread_mask;i+=stride)
    for (size_t k = 0; k < global.lda; k++)
      total_lambda[k] += weights[i+k];

  v_array<float> decay_levels;
  push(decay_levels, (float)0);
  double example_t = global.initial_t;
  while ( true )
    {
      example_t++;
      float total_new[global.lda];
      for (size_t k = 0; k < global.lda; k++)
	total_new[k] = 0.f;

      while (merge_set.index() > 0)
	{
	  merge_set[merge_set.index()-1].erase();
	  merge_set.pop();
	}

      float eta = -1;
      float minuseta = -1;
      example* examples[global.minibatch];
      size_t batch_size = global.minibatch;
      for (size_t d = 0; d < batch_size; d++)
	{
	  if ((ec = get_example(0)) != NULL)//semiblocking operation.
	    {
	      examples[d] = ec;
	      merge_in(ec,d);
	    }
	  else if (thread_done(0))
	    batch_size = d;
	  else
	    d--;
	}

      merge_all(); //Now merge_set[0] contains everything.

      eta = global.eta * powf(example_t, -t.vars->power_t);
      minuseta = 1.0 - eta;
      eta *= global.lda_D / batch_size;
      push(decay_levels, decay_levels.last() + log(minuseta));

      float digammas[global.lda];
      float additional = (float)(global.length()) * global.lda_rho;
      for (size_t i = 0; i<global.lda; i++) {
	digammas[i] = mydigamma(total_lambda[i] + additional);
      }
      
      size_t last_weight_index = -1;
      for (index_triple* s = merge_set[0].begin; s != merge_set[0].end; s++)
	{
	  if (last_weight_index == s->f.weight_index)
	    continue;
	  last_weight_index = s->f.weight_index;
	  float* weights_for_w = &(weights[s->f.weight_index & global.thread_mask]);
          float decay = fmin(1.0, exp(decay_levels.end[-2] - decay_levels.end[(int)(-1-example_t+weights_for_w[global.lda])]));
	  float* u_for_w = weights_for_w + global.lda+1;

	  weights_for_w[global.lda] = example_t;
	  for (size_t k = 0; k < global.lda; k++)
	    {
	      weights_for_w[k] *= decay;
	      u_for_w[k] = weights_for_w[k] + global.lda_rho;
	    }
	  myexpdigammify_2(u_for_w, digammas);
	}

      float v[batch_size*global.lda];

      for (size_t d = 0; d < batch_size; d++)
	{
          float score = lda_loop(&v[d*global.lda], weights, examples[d],t.vars->power_t);
          if (global.audit)
	    print_audit_features(reg, examples[d]);
          global.sum_loss -= score;
          global.sum_loss_since_last_dump -= score;
          finish_example(examples[d]);
	}

      for (index_triple* s = merge_set[0].begin; s != merge_set[0].end;)
	{
	  index_triple* next = s+1;
	  while(next != merge_set[0].end && next->f.weight_index == s->f.weight_index)
	    next++;

	  float* word_weights = &(weights[s->f.weight_index & global.thread_mask]);
	  for (size_t k = 0; k < global.lda; k++) {
	    float new_value = minuseta*word_weights[k];
	    word_weights[k] = new_value;
	  }

	  for (; s != next; s++) {
	    float* v_s = &v[s->document*global.lda];
	    float* u_for_w = &weights[(s->f.weight_index & global.thread_mask) + global.lda + 1];
	    float c_w = eta*find_cw(u_for_w, v_s)*s->f.x;
	    for (size_t k = 0; k < global.lda; k++) {
	      float new_value = u_for_w[k]*v_s[k]*c_w;
	      total_new[k] += new_value;
 	      word_weights[k] += new_value;
	    }
	  }
	}
      for (size_t k = 0; k < global.lda; k++) {
	total_lambda[k] *= minuseta;
	total_lambda[k] += total_new[k];
      }

      if (thread_done(0))
	{
	  for (size_t i = 0; i < global.length(); i++) {
	    weight* weights_for_w = & (weights[i*global.stride]);
            float decay = fmin(1.0, exp(decay_levels.last() - decay_levels.end[(int)(-1-example_t+weights_for_w[global.lda])]));
	    for (size_t k = 0; k < global.lda; k++) {
	      weights_for_w[k] *= decay;
            }
	  }

	  if (global.local_prediction > 0)
	    shutdown(global.local_prediction, SHUT_WR);

	  for (int i = 0; i < merge_set.end_array-merge_set.begin; i++)
	    free(merge_set[i].begin);
	  free(merge_set.begin);
	  return;
	}
    }
}

void end_lda()
{
  
}
