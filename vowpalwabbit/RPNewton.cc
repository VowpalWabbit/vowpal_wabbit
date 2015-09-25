/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/
#include <string>
#include "gd.h"
#include "rand48.h"
#include <random>

using namespace std;
using namespace LEARNER;

struct update_data {
	struct RPNewton *RPN;
	float g;
	float sketch;
	float *r;
	float *u;
	float rb;
	float prediction;
};

struct RPNewton {
	vw* all; 
	int m;
	float alpha;
	float *b;
	float **H;
	struct update_data data;	

	void generate_r() {
		for (int i = 0; i < m; i++) {
			double rn = frand48();
			if (rn < 1.0 / 6) 
				data.r[i] = sqrt(3.0 / m);
			else if (rn < 1.0 / 3) 
				data.r[i] = -sqrt(3.0 / m);
			else
				data.r[i] = 0;
		}
	}
};

void make_prediction(update_data& data, float x, float& wref) {
	int m = data.RPN->m;
	float* w = &wref;
	
	for (int i = 0; i < m; i++) {
		data.prediction += w[i] * data.RPN->b[i] * x;
	}
	data.prediction += w[m] * x;
}

void predict(RPNewton& RPN, base_learner&, example& ec) {
	RPN.data.prediction = 0;
	GD::foreach_feature<update_data, make_prediction>(*RPN.all, ec, RPN.data);
	ec.partial_prediction = RPN.data.prediction;
	ec.pred.scalar = GD::finalize_prediction(RPN.all->sd, ec.partial_prediction);
}


void weight_update(update_data& data, float x, float& wref) {
	float* w = &wref;
	float gradient = data.g * x;
	float s = data.sketch * x;
	int m = data.RPN->m;

	for (int i = 0; i < m; i++) {
		w[i] += data.r[i] * s;		
		data.u[i] += w[i] * gradient;
	}
	w[m] -= gradient / data.RPN->alpha + data.rb * s;
}

void get_u(update_data& data, float x, float& wref) {
	float* w = &wref;
	float s = data.sketch * x;

	for (int i = 0; i < data.RPN->m; i++) {
		data.u[i] += w[i] * s + s * s * data.r[i] / 2;
	}
}

// update H so that inv(H) <- inv(H) + uv'
void Woodbury_update(float **H, float *u, float *v, int size)
{
	float q = 0;
	float *Hu = calloc_or_die<float>(size);
	float *vH = calloc_or_die<float>(size);

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			q += v[i] * u[j] * H[i][j];
			Hu[i] += H[i][j] * u[j];
			vH[i] += H[j][i] * v[j];
		}
	}

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			H[i][j] -= Hu[i] * vH[j] / (1 + q);
		}
	}

	free(Hu);
	free(vH);
}

void learn(RPNewton& RPN, base_learner& base, example& ec) {
	assert(ec.in_use);
	
	// predict
	predict(RPN, base, ec);

	//update state based on the prediction
	update_data& data = RPN.data;

	data.g = RPN.all->loss->first_derivative(RPN.all->sd, ec.pred.scalar, ec.l.simple.label)
		*ec.l.simple.weight;
	data.g /= 2; // for half square loss...
	data.sketch = data.g / 2;
	RPN.generate_r();

	// The rank-2 update of inv(H) would be ur' + ru'
	memset(data.u, 0, sizeof(float)* RPN.m);
	GD::foreach_feature<update_data, get_u>(*RPN.all, ec, data);

	// Update H
	Woodbury_update(RPN.H, data.u, data.r, RPN.m);
	Woodbury_update(RPN.H, data.r, data.u, RPN.m);

	data.rb = 0;
	for (int i = 0; i < RPN.m; i++) data.rb += data.r[i] * RPN.b[i];

	// u will be Sg after update
	memset(data.u, 0, sizeof(float)* RPN.m);
	GD::foreach_feature<update_data, weight_update>(*RPN.all, ec, data);

	// update coefficients b
	for (int i = 0; i < RPN.m; i++) {
		float tmp = 0;
		for (int j = 0; j < RPN.m; j++) {
			tmp += RPN.H[i][j] * data.u[j];
		}
		RPN.b[i] += tmp / RPN.alpha;
	}
}


void save_load(RPNewton& RPN, io_buf& model_file, bool read, bool text) {
	vw* all = RPN.all;
	if (read)
		initialize_regressor(*all);

	if (model_file.files.size() > 0) {
		bool resume = all->save_resume;
		char buff[512];
		uint32_t text_len = sprintf(buff, ":%d\n", resume);
		bin_text_read_write_fixed(model_file, (char *)&resume, sizeof (resume), "", read, buff, text_len, text);

		if (resume)
			GD::save_load_online_state(*all, model_file, read, text);
		else
			GD::save_load_regressor(*all, model_file, read, text);
	}
}


base_learner* RPNewton_setup(vw& all) {
	if (missing_option(all, false, "RPNewton", "Online Newton with Random Projection Sketch"))
		return nullptr;

	new_options(all, "RPNewton options")
		("sketch_size", po::value<int>(), "size of sketch")
		("alpha", po::value<float>(), "mutiplicative constant for indentiy");
	add_options(all);

	po::variables_map& vm = all.vm;

	RPNewton& RPN = calloc_or_die<RPNewton>();
	RPN.all = &all;
	
	if (vm.count("sketch_size"))
		RPN.m = vm["sketch_size"].as<int>();
	else
		RPN.m = 10;

	if (vm.count("alpha"))
		RPN.alpha = vm["alpha"].as<float>();
	else
		RPN.alpha = 1.0;
	
	RPN.b = calloc_or_die<float>(RPN.m);	
	RPN.H = calloc_or_die<float*>(RPN.m);
	for (int i = 0; i < RPN.m; i++) {
		RPN.H[i] = calloc_or_die<float>(RPN.m);
		RPN.H[i][i] = 1.0 / RPN.alpha;
	}

	RPN.data.r = calloc_or_die<float>(RPN.m);
	RPN.data.u = calloc_or_die<float>(RPN.m);
	RPN.data.RPN = &RPN;

	all.reg.stride_shift = ceil(log2(RPN.m + 1));

	learner<RPNewton>& l = init_learner(&RPN, learn, 1 << all.reg.stride_shift);
	l.set_predict(predict);
	l.set_save_load(save_load);
	return make_base(l);
}