/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef GD_H
#define GD_H

#ifdef __FreeBSD__
#include <sys/socket.h>
#endif

#include <math.h>
#include "example.h"
#include "parse_regressor.h"
#include "parser.h"
#include "sparse_dense.h"

namespace GD{
void print_result(int f, float res, v_array<char> tag);
void print_audit_features(regressor &reg, example* ec, size_t offset);
float finalize_prediction(vw&, float ret);
void print_audit_features(vw&, example* ec);
void train_one_example(regressor& r, example* ex);
void train_offset_example(regressor& r, example* ex, size_t offset);
void compute_update(example* ec);
void offset_train(regressor &reg, example* &ec, float update, size_t offset);
void train_one_example_single_thread(regressor& r, example* ex);
learner setup(vw& all, po::variables_map& vm);
void save_load_regressor(vw& all, io_buf& model_file, bool read, bool text);
void output_and_account_example(example* ec);

 template <void (*T)(vw&, void*, float, uint32_t)>
   void foreach_feature(vw& all, void* dat, feature* begin, feature* end, uint32_t offset=0, float mult=1.)
   {
     for (feature* f = begin; f!= end; f++)
       T(all, dat, mult*f->x, f->weight_index + offset);
   }
 
 template <void (*T)(vw&, void*, float, uint32_t)>
   void foreach_feature(vw& all, example* ec, void* dat)
   {
     uint32_t offset = ec->ft_offset;

     for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
       foreach_feature<T>(all, dat, ec->atomics[*i].begin, ec->atomics[*i].end, offset);
     
     for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) {
       if (ec->atomics[(int)(*i)[0]].size() > 0) {
	 v_array<feature> temp = ec->atomics[(int)(*i)[0]];
	 for (; temp.begin != temp.end; temp.begin++)
	   {
	     uint32_t halfhash = quadratic_constant * (temp.begin->weight_index + offset);
	     foreach_feature<T>(all, dat, ec->atomics[(int)(*i)[1]].begin, ec->atomics[(int)(*i)[1]].end, 
				halfhash, temp.begin->x);
	   }
       }
     }
     
     for (vector<string>::iterator i = all.triples.begin(); i != all.triples.end();i++) {
       if ((ec->atomics[(int)(*i)[0]].size() == 0) || (ec->atomics[(int)(*i)[1]].size() == 0) || (ec->atomics[(int)(*i)[2]].size() == 0)) { continue; }
       v_array<feature> temp1 = ec->atomics[(int)(*i)[0]];
       for (; temp1.begin != temp1.end; temp1.begin++) {
	 v_array<feature> temp2 = ec->atomics[(int)(*i)[1]];
	 for (; temp2.begin != temp2.end; temp2.begin++) {
	   
	   uint32_t halfhash = cubic_constant2 * (cubic_constant * (temp1.begin->weight_index + offset) + temp2.begin->weight_index + offset);
	   float mult = temp1.begin->x * temp2.begin->x;
	   return foreach_feature<T>(all, dat, ec->atomics[(int)(*i)[2]].begin, ec->atomics[(int)(*i)[2]].end, halfhash, mult);
	 }
       }
     }
   }
 
 template <void (*T)(vw&,void*, float,uint32_t)>
   float inline_predict(vw& all, example* ec)
   {
     float prediction = all.p->lp->get_initial(ec->ld);
     foreach_feature<T>(all, ec, &prediction);
     return prediction;
   }
}

#endif
