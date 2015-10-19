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
    float sketch_cnt;
    float *r;
    float *q;
    float *Sx;
    float rb;
    float norm2_x;
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
        for (int i = 1; i <= m; i++) {
            double rn = frand48();
            if (rn < 1.0 / 6) 
                data.r[i] = sqrt(3.0 / m);
            else if (rn < 1.0 / 3) 
                data.r[i] = -sqrt(3.0 / m);
            else
                data.r[i] = 0;
        }
    }
    
    void compute_q() {
        float cnt = data.sketch_cnt;
        float tmp = cnt * cnt * data.norm2_x / 2;
        for (int i = 1; i <= m; i++) {
            data.q[i] = data.Sx[i] * cnt - tmp * data.r[i];
         }
    }

    // update H so that inv(H) <- inv(H) + uv'
    void Woodbury_update(float **H, float *u, float *v, int size)
    {
        float quad = 0;
        float *Hu = calloc_or_die<float>(size+1);
        float *vH = calloc_or_die<float>(size+1);

        for (int i = 1; i <= size; i++) {
            for (int j = 1; j <= size; j++) {
                quad += v[i] * u[j] * H[i][j];
                Hu[i] += H[i][j] * u[j];
                vH[i] += H[j][i] * v[j];
            }
        }

        for (int i = 1; i <= size; i++) {
            for (int j = 1; j <= size; j++) {
                H[i][j] -= Hu[i] * vH[j] / (1 + quad);
            }
        }

        free(Hu);
        free(vH);
    }

    void update_b() {
        for (int i = 1; i <= m; i++) {
            float tmp = 0;
            for (int j = 1; j <= m; j++) {
                tmp += H[i][j] * data.Sx[j] * data.g;
            }
            b[i] += tmp / alpha;
        }
    }

    void update_Sx() {
        for (int i = 1; i <= m; i++) 
            data.Sx[i] += data.sketch_cnt * data.norm2_x * data.r[i];
    }
};

void make_prediction(update_data& data, float x, float& wref) {
    int m = data.RPN->m;
    float* w = &wref;
    
    data.prediction += w[0] * x;
    data.norm2_x += x * x;
    for (int i = 1; i <= m; i++) {
        data.Sx[i] += w[i] * x;
        data.prediction += data.RPN->b[i] * w[i] * x;
    }
}

void predict(RPNewton& RPN, base_learner&, example& ec) {
    RPN.data.prediction = 0;
    RPN.data.norm2_x = 0;
    memset(RPN.data.Sx, 0, sizeof(float) * (RPN.m + 1));
    GD::foreach_feature<update_data, make_prediction>(*RPN.all, ec, RPN.data);
    ec.partial_prediction = RPN.data.prediction;
    ec.pred.scalar = GD::finalize_prediction(RPN.all->sd, ec.partial_prediction);
}

void update_sketch(update_data& data, float x, float& wref) {
    float* w = &wref;

    for (int i = 1; i <= data.RPN->m ;i++)
        w[i] += data.r[i] * data.sketch_cnt * x;
}

void update_weight(update_data& data, float x, float& wref) {
    float* w = &wref;

    w[0] -= x * (data.g / data.RPN->alpha + data.rb * data.sketch_cnt);
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
    data.sketch_cnt = data.g / 2;
    RPN.generate_r();

    GD::foreach_feature<update_data, update_sketch>(*RPN.all, ec, RPN.data);
    RPN.update_Sx();    

    // The rank-2 update of inv(H) will be qr' + rq'
    RPN.compute_q();

    // Update H
    RPN.Woodbury_update(RPN.H, data.q, data.r, RPN.m);
    RPN.Woodbury_update(RPN.H, data.r, data.q, RPN.m);

    data.rb = 0;
    for (int i = 1; i <= RPN.m; i++) data.rb += data.r[i] * RPN.b[i];

    GD::foreach_feature<update_data, update_weight>(*RPN.all, ec, RPN.data);
    RPN.update_b();
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
    
    RPN.b = calloc_or_die<float>(RPN.m+1);    
    RPN.H = calloc_or_die<float*>(RPN.m+1);
    for (int i = 1; i <= RPN.m; i++) {
        RPN.H[i] = calloc_or_die<float>(RPN.m+1);
        RPN.H[i][i] = 1.0 / RPN.alpha;
    }

    RPN.data.r = calloc_or_die<float>(RPN.m+1);
    RPN.data.q = calloc_or_die<float>(RPN.m+1);
    RPN.data.Sx = calloc_or_die<float>(RPN.m+1);
    RPN.data.RPN = &RPN;

    all.reg.stride_shift = ceil(log2(RPN.m + 1));

    learner<RPNewton>& l = init_learner(&RPN, learn, 1 << all.reg.stride_shift);
    l.set_predict(predict);
    l.set_save_load(save_load);
    return make_base(l);
}
