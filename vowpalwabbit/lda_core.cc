/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <fstream>
#include <vector>
#include <float.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#endif
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "parse_example.h"
#include "constant.h"
#include "sparse_dense.h"
#include "gd.h"
#include "lda_core.h"
#include "cache.h"
#include "simple_label.h"
#include "rand48.h"
#include "vw.h"

namespace LDA {

class index_feature {
public:
  uint32_t document;
  feature f;
  bool operator<(const index_feature b) const { return f.weight_index < b.f.weight_index; }
};

  struct lda {
    v_array<float> Elogtheta;
    v_array<float> decay_levels;
    v_array<float> total_new;
    v_array<example* > examples;
    v_array<float> total_lambda;
    v_array<int> doc_lengths;
    v_array<float> digammas;
    v_array<float> v;
    std::vector<index_feature> sorted_features;

    bool total_lambda_init;
    
    double example_t;
    vw* all;
  };
  
#ifdef _WIN32
inline float fmax(float f1, float f2) { return (f1 < f2 ? f2 : f1); }
inline float fmin(float f1, float f2) { return (f1 > f2 ? f2 : f1); }
#endif

#define MINEIRO_SPECIAL
#ifdef MINEIRO_SPECIAL

namespace {

inline float 
fastlog2 (float x)
{
  union { float f; uint32_t i; } vx = { x };
  union { uint32_t i; float f; } mx = { (vx.i & 0x007FFFFF) | (0x7e << 23) };
  float y = (float)vx.i;
  y *= 1.0f / (float)(1 << 23);

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
  int w = (int)clipp;
  float z = clipp - w + offset;
  union { uint32_t i; float f; } v = { (uint32_t)((1 << 23) * (clipp + 121.2740838f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z)) };

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

static inline float
v4sf_index (const v4sf x,
            unsigned int i)
{
  union { v4sf f; float array[4]; } tmp = { x };

  return tmp.array[i];
}

static inline const v4sf
v4sfl (float x)
{
  union { float array[4]; v4sf f; } tmp = { { x, x, x, x } };

  return tmp.f;
}

static inline const v4si
v4sil (uint32_t x)
{
  uint64_t wide = (((uint64_t) x) << 32) | x;
  union { uint64_t array[2]; v4si f; } tmp = { { wide, wide } };

  return tmp.f;
}

static inline v4sf
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
vexpdigammify (vw& all, float* gamma)
{
  unsigned int n = all.lda;
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

void vexpdigammify_2(vw& all, float* gamma, const float* norm)
{
  size_t n = all.lda;
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
#ifndef _WIN32
#warning "lda IS NOT using sse instructions"
#endif
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

float decayfunc(float t, float old_t, float power_t) {
  float result = 1;
  for (float i = old_t+1; i <= t; i += 1)
    result *= (1-powf(i, -power_t));
  return result;
}

float decayfunc2(float t, float old_t, float power_t) 
{
  float power_t_plus_one = 1.f - power_t;
  float arg =  - ( powf(t, power_t_plus_one) -
                   powf(old_t, power_t_plus_one));
  return exp ( arg
               / power_t_plus_one);
}

float decayfunc3(double t, double old_t, double power_t) 
{
  double power_t_plus_one = 1. - power_t;
  double logt = log((float)t);
  double logoldt = log((float)old_t);
  return (float)((old_t / t) * exp((float)(0.5*power_t_plus_one*(-logt*logt + logoldt*logoldt))));
}

float decayfunc4(double t, double old_t, double power_t)
{
  if (power_t > 0.99)
    return decayfunc3(t, old_t, power_t);
  else
    return (float)decayfunc2((float)t, (float)old_t, (float)power_t);
}

void expdigammify(vw& all, float* gamma)
{
  float sum=0;
  for (size_t i = 0; i<all.lda; i++)
    {
      sum += gamma[i];
      gamma[i] = mydigamma(gamma[i]);
    }
  sum = mydigamma(sum);
  for (size_t i = 0; i<all.lda; i++)
    gamma[i] = fmax(1e-6f, exp(gamma[i] - sum));
}

void expdigammify_2(vw& all, float* gamma, float* norm)
{
  for (size_t i = 0; i<all.lda; i++)
    {
      gamma[i] = fmax(1e-6f, exp(mydigamma(gamma[i]) - norm[i]));
    }
}

float average_diff(vw& all, float* oldgamma, float* newgamma)
{
  float sum = 0.;
  float normalizer = 0.;
  for (size_t i = 0; i<all.lda; i++) {
    sum += fabsf(oldgamma[i] - newgamma[i]);
    normalizer += newgamma[i];
  }
  return sum / normalizer;
}

// Returns E_q[log p(\theta)] - E_q[log q(\theta)].
  float theta_kl(vw& all, v_array<float>& Elogtheta, float* gamma)
{
  float gammasum = 0;
  Elogtheta.erase();
  for (size_t k = 0; k < all.lda; k++) {
    Elogtheta.push_back(mydigamma(gamma[k]));
    gammasum += gamma[k];
  }
  float digammasum = mydigamma(gammasum);
  gammasum = mylgamma(gammasum);
  float kl = -(all.lda*mylgamma(all.lda_alpha));
  kl += mylgamma(all.lda_alpha*all.lda) - gammasum;
  for (size_t k = 0; k < all.lda; k++) {
    Elogtheta[k] -= digammasum;
    kl += (all.lda_alpha - gamma[k]) * Elogtheta[k];
    kl += mylgamma(gamma[k]);
  }

  return kl;
}

float find_cw(vw& all, float* u_for_w, float* v)
{
  float c_w = 0;
  for (size_t k =0; k<all.lda; k++)
    c_w += u_for_w[k]*v[k];

  return 1.f / c_w;
}

v_array<float> new_gamma;
v_array<float> old_gamma;
// Returns an estimate of the part of the variational bound that
// doesn't have to do with beta for the entire corpus for the current
// setting of lambda based on the document passed in. The value is
// divided by the total number of words in the document This can be
// used as a (possibly very noisy) estimate of held-out likelihood.
  float lda_loop(vw& all, v_array<float>& Elogtheta, float* v,weight* weights,example* ec, float power_t)
{
  new_gamma.erase();
  old_gamma.erase();
  
  for (size_t i = 0; i < all.lda; i++)
    {
      new_gamma.push_back(1.f);
      old_gamma.push_back(0.f);
    }
  size_t num_words =0;
  for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++)
    num_words += ec->atomics[*i].end - ec->atomics[*i].begin;

  float xc_w = 0;
  float score = 0;
  float doc_length = 0;
  do
    {
      memcpy(v,new_gamma.begin,sizeof(float)*all.lda);
      myexpdigammify(all, v);

      memcpy(old_gamma.begin,new_gamma.begin,sizeof(float)*all.lda);
      memset(new_gamma.begin,0,sizeof(float)*all.lda);

      score = 0;
      size_t word_count = 0;
      doc_length = 0;
      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++)
	{
	  feature *f = ec->atomics[*i].begin;
	  for (; f != ec->atomics[*i].end; f++)
	    {
	      float* u_for_w = &weights[(f->weight_index&all.reg.weight_mask)+all.lda+1];
	      float c_w = find_cw(all, u_for_w,v);
	      xc_w = c_w * f->x;
              score += -f->x*log(c_w);
	      size_t max_k = all.lda;
	      for (size_t k =0; k<max_k; k++) {
		new_gamma[k] += xc_w*u_for_w[k];
	      }
	      word_count++;
              doc_length += f->x;
	    }
	}
      for (size_t k =0; k<all.lda; k++)
	new_gamma[k] = new_gamma[k]*v[k]+all.lda_alpha;
    }
  while (average_diff(all, old_gamma.begin, new_gamma.begin) > 0.001);

  ec->topic_predictions.erase();
  ec->topic_predictions.resize(all.lda);
  memcpy(ec->topic_predictions.begin,new_gamma.begin,all.lda*sizeof(float));

  score += theta_kl(all, Elogtheta, new_gamma.begin);

  return score / doc_length;
}

size_t next_pow2(size_t x) {
  int i = 0;
  x = x > 0 ? x - 1 : 0;
  while (x > 0) {
    x >>= 1;
    i++;
  }
  return ((size_t)1) << i;
}

void save_load(void* d, io_buf& model_file, bool read, bool text)
{
  lda* l = (lda*)d;
  vw* all = l->all;
  uint32_t length = 1 << all->num_bits;
  uint32_t stride = all->reg.stride;
  
  if (read)
    {
      initialize_regressor(*all);
      for (size_t j = 0; j < stride*length; j+=stride)
	{
	  for (size_t k = 0; k < all->lda; k++) {
	    if (all->random_weights) {
	      all->reg.weight_vector[j+k] = (float)(-log(frand48()) + 1.0f);
	      all->reg.weight_vector[j+k] *= (float)(all->lda_D / all->lda / all->length() * 200);
	    }
	  }
	  all->reg.weight_vector[j+all->lda] = all->initial_t;
	}
    }
    
  if (model_file.files.size() > 0)
    {
      uint32_t i = 0;
      uint32_t text_len;
      char buff[512];
      size_t brw = 1;
      do 
	{
	  brw = 0;
	  size_t K = all->lda;
	  
	  text_len = sprintf(buff, "%d ", i);
	  brw += bin_text_read_write_fixed(model_file,(char *)&i, sizeof (i),
					   "", read,
					   buff, text_len, text);
	  if (brw != 0)
	    for (uint32_t k = 0; k < K; k++)
	      {
		uint32_t ndx = stride*i+k;
		
		weight* v = &(all->reg.weight_vector[ndx]);
		text_len = sprintf(buff, "%f ", *v + all->lda_rho);
		
		brw += bin_text_read_write_fixed(model_file,(char *)v, sizeof (*v),
						 "", read,
						 buff, text_len, text);
		
	      }
	  if (text)
	    brw += bin_text_read_write_fixed(model_file,buff,0,
					     "", read,
					     "\n",1,text);
	  
	  if (!read)
	    i++;
	}  
      while ((!read && i < length) || (read && brw >0));
    }
}

  void learn_batch(lda& l)
  {
    float eta = -1;
    float minuseta = -1;

    if (l.total_lambda.size() == 0)
      {
	for (size_t k = 0; k < l.all->lda; k++)
	  l.total_lambda.push_back(0.f);
	
	size_t stride = l.all->reg.stride;
	for (size_t i =0; i <= l.all->reg.weight_mask;i+=stride)
	  for (size_t k = 0; k < l.all->lda; k++)
	    l.total_lambda[k] += l.all->reg.weight_vector[i+k];
      }

    l.example_t++;
    l.total_new.erase();
    for (size_t k = 0; k < l.all->lda; k++)
      l.total_new.push_back(0.f);
    
    size_t batch_size = l.examples.size();
   
    sort(l.sorted_features.begin(), l.sorted_features.end());
    
    eta = l.all->eta * powf((float)l.example_t, - l.all->power_t);
    minuseta = 1.0f - eta;
    eta *= l.all->lda_D / batch_size;
    l.decay_levels.push_back(l.decay_levels.last() + log(minuseta));
    
    l.digammas.erase();
    float additional = (float)(l.all->length()) * l.all->lda_rho;
    for (size_t i = 0; i<l.all->lda; i++) {
      l.digammas.push_back(mydigamma(l.total_lambda[i] + additional));
    }
    
    
    weight* weights = l.all->reg.weight_vector;
    
    size_t last_weight_index = -1;
    for (index_feature* s = &l.sorted_features[0]; s <= &l.sorted_features.back(); s++)
      {
	if (last_weight_index == s->f.weight_index)
	  continue;
	last_weight_index = s->f.weight_index;
	float* weights_for_w = &(weights[s->f.weight_index & l.all->reg.weight_mask]);
	float decay = fmin(1.0, exp(l.decay_levels.end[-2] - l.decay_levels.end[(int)(-1 - l.example_t+weights_for_w[l.all->lda])]));
	float* u_for_w = weights_for_w + l.all->lda+1;
	
	weights_for_w[l.all->lda] = (float)l.example_t;
	for (size_t k = 0; k < l.all->lda; k++)
	  {
	    weights_for_w[k] *= decay;
	    u_for_w[k] = weights_for_w[k] + l.all->lda_rho;
	  }
	myexpdigammify_2(*l.all, u_for_w, l.digammas.begin);
      }
    
    for (size_t d = 0; d < batch_size; d++)
      {
	float score = lda_loop(*l.all, l.Elogtheta, &(l.v[d*l.all->lda]), weights, l.examples[d],l.all->power_t);
	if (l.all->audit)
	  GD::print_audit_features(*l.all, l.examples[d]);
	// If the doc is empty, give it loss of 0.
	if (l.doc_lengths[d] > 0) {
	  l.all->sd->sum_loss -= score;
	  l.all->sd->sum_loss_since_last_dump -= score;
	}
	return_simple_example(*l.all, NULL, l.examples[d]);
      }
    
    for (index_feature* s = &l.sorted_features[0]; s <= &l.sorted_features.back();)
      {
	index_feature* next = s+1;
	while(next <= &l.sorted_features.back() && next->f.weight_index == s->f.weight_index)
	  next++;
	
	float* word_weights = &(weights[s->f.weight_index & l.all->reg.weight_mask]);
	for (size_t k = 0; k < l.all->lda; k++) {
	  float new_value = minuseta*word_weights[k];
	  word_weights[k] = new_value;
	}
	
	for (; s != next; s++) {
	  float* v_s = &(l.v[s->document*l.all->lda]);
	  float* u_for_w = &weights[(s->f.weight_index & l.all->reg.weight_mask) + l.all->lda + 1];
	  float c_w = eta*find_cw(*l.all, u_for_w, v_s)*s->f.x;
	  for (size_t k = 0; k < l.all->lda; k++) {
	    float new_value = u_for_w[k]*v_s[k]*c_w;
	    l.total_new[k] += new_value;
	    word_weights[k] += new_value;
	  }
	}
      }
    for (size_t k = 0; k < l.all->lda; k++) {
      l.total_lambda[k] *= minuseta;
      l.total_lambda[k] += l.total_new[k];
    }

    l.sorted_features.resize(0);
    
    l.examples.erase();
    l.doc_lengths.erase();
  }
  
  void learn(void* d, learner& base, example* ec) 
  {
    lda* l = (lda*)d;

    size_t num_ex = l->examples.size();
    l->examples.push_back(ec);
    l->doc_lengths.push_back(0);
    for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) {
      feature* f = ec->atomics[*i].begin;
      for (; f != ec->atomics[*i].end; f++) {
	index_feature temp = {(uint32_t)num_ex, *f};
	l->sorted_features.push_back(temp);
	l->doc_lengths[num_ex] += (int)f->x;
      }
    }
    if (++num_ex == l->all->minibatch)
      learn_batch(*l);
  }

  void end_pass(void* d)
  {
    lda* l = (lda*)d;
    
    if (l->examples.size())
      learn_batch(*l);
  }

void end_examples(void* d)
{
  lda* l = (lda*)d;

  for (size_t i = 0; i < l->all->length(); i++) {
    weight* weights_for_w = & (l->all->reg.weight_vector[i*l->all->reg.stride]);
    float decay = fmin(1.0, exp(l->decay_levels.last() - l->decay_levels.end[(int)(-1- l->example_t +weights_for_w[l->all->lda])]));
    for (size_t k = 0; k < l->all->lda; k++) 
      weights_for_w[k] *= decay;
  }
}

  void finish_example(vw& all, void*, example*ec)
{}

learner* setup(vw&all, std::vector<std::string>&opts, po::variables_map& vm)
{
  lda* ld = (lda*)calloc(1,sizeof(lda));
  ld->all = &all;
  ld->example_t = all.initial_t;

  po::options_description desc("LDA options");
  desc.add_options()
    ("lda_alpha", po::value<float>(&all.lda_alpha), "Prior on sparsity of per-document topic weights")
    ("lda_rho", po::value<float>(&all.lda_rho), "Prior on sparsity of topic distributions")
    ("lda_D", po::value<float>(&all.lda_D), "Number of documents")
    ("minibatch", po::value<size_t>(&all.minibatch), "Minibatch size, for LDA");

  po::parsed_options parsed = po::command_line_parser(opts).
    style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
    options(desc).allow_unregistered().run();
  opts = po::collect_unrecognized(parsed.options, po::include_positional);
  po::store(parsed, vm);
  po::notify(vm);

  all.p->sort_features = true;
  float temp = ceilf(logf((float)(all.lda*2+1)) / logf (2.f));
  all.reg.stride = ((size_t)1) << (int) temp;
  all.random_weights = true;
  all.add_constant = false;

  if (vm.count("lda") && all.eta > 1.)
    {
      cerr << "your learning rate is too high, setting it to 1" << endl;
      all.eta = min(all.eta,1.f);
    }

  if (vm.count("minibatch")) {
    size_t minibatch2 = next_pow2(all.minibatch);
    all.p->ring_size = all.p->ring_size > minibatch2 ? all.p->ring_size : minibatch2;
}
  
  ld->v.resize(all.lda*all.minibatch);
  
  ld->decay_levels.push_back(0.f);
  
  learner* l = new learner(ld, learn, save_load, all.reg.stride);
  l->set_save_load(save_load);
  l->set_finish_example(finish_example);
  l->set_end_examples(end_examples);  
  l->set_end_pass(end_pass);  
  
  return l;
}
}
