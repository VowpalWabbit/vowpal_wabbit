/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.    Released under a BSD (revised)
license as described in the file LICENSE.
*/
#include <string>
#include "gd.h"
#include "vw.h"

using namespace std;
using namespace LEARNER;

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
    double *D;
    double **A;
    double **K;

    example **buffer;
    float *weight_buffer;
    struct update_data data;

    float learning_rate_cnt;

    void initialize_Z()
    {
        size_t stride_shift = all->reg.stride_shift;
        weight* weights = all->reg.weight_vector;

        for (int i = 1; i <= m; i++)
            weights[(i << stride_shift) + i] = 1;
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
            double gamma = fmin(learning_rate_cnt * i / t, .1);
            double tmp = data.AZx[i] * data.sketch_cnt;

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
            double gamma = fmin(learning_rate_cnt * i / t, .1);
            
            data.delta[i] = gamma * data.AZx[i] * data.sketch_cnt;
            for (int j = 1; j < i; j++) {
                data.delta[i] -= A[i][j] * data.delta[j];
            }
            data.delta[i] /= A[i][i];
            printf("delta[%d] = %f\n", i, data.delta[i]);
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
                printf("K[%d][%d] = %f\n", i, j, K[i][j]); 
            }
        }
    }

    void update_A()
    {
        float *zv = calloc_or_die<float>(m+1);
        float *vv = calloc_or_die<float>(m+1);

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

            double norm = 0;
            for (int j = 1; j <= i; j++) {
	        double temp = 0;
                for (int k = 1; k <= i; k++) {
                    temp += K[j][k] * A[i][k];
                    //cout << "K = " << K[j][k] << endl;
                    //cout << "A = " << A[i][k] << endl;
                }
                norm += A[i][j]*temp;
            }
            //cout << norm << endl;
            norm = sqrt(norm);

            for (int j = 1; j <= i; j++) {
                A[i][j] /= norm;
                printf("A[%d][%d] = %f\n", i, j, A[i][j]);
            }
        }

        free(zv);
        free(vv);
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
            cout << "scale = " << scale << endl;
            if (scale < 1e-10) continue;
            for (int i = 1; i <= m; i++) {
                A[i][j] /= scale;
                K[j][i] *= scale;
                K[i][j] *= scale;
            }
            b[j] /= scale;
            D[j] *= scale;
            printf("D[%d] = %f\n", j, D[j]);
        }
    }
};

void keep_example(vw& all, OjaNewton& ON, example& ec) {
    output_and_account_example(all, ec);
}

void make_pred(update_data& data, float x, float& wref) {
    int m = data.ON->m;
    float* w = &wref;

    data.prediction += w[0] * x;
    for (int i = 1; i <= m; i++) {
        data.prediction += w[i] * x * data.ON->D[i] * data.ON->b[i];
    }
}

void predict(OjaNewton& ON, base_learner&, example& ec) {
    ON.data.prediction = 0;
    GD::foreach_feature<update_data, make_pred>(*ON.all, ec, ON.data);
    ec.partial_prediction = ON.data.prediction;
    ec.pred.scalar = GD::finalize_prediction(ON.all->sd, ec.partial_prediction);
}


void update_Z_and_wbar(update_data& data, float x, float& wref) {
    float* w = &wref;
    float s = data.sketch_cnt * x;
    int m = data.ON->m;

    for (int i = 1; i <= m; i++) {
        w[i] += data.delta[i] * s / data.ON->D[i];
    }
    w[0] -= s * data.bdelta;
}

void compute_Zx_and_norm(update_data& data, float x, float& wref) {
    float* w = &wref;

    for (int i = 1; i <= data.ON->m; i++) {
        data.Zx[i] += w[i] * x * data.ON->D[i];
    }
    data.norm2_x += x * x;
}

void update_wbar_and_Zx(update_data& data, float x, float& wref) {
    float* w = &wref;
    float g = data.g * x;

    for (int i = 1; i <= data.ON->m; i++) {
        data.Zx[i] += w[i] * x * data.ON->D[i];
    }
    w[0] -= g / data.ON->alpha;
}


void learn(OjaNewton& ON, base_learner& base, example& ec) {
    assert(ec.in_use);

    // predict
    predict(ON, base, ec);

    update_data& data = ON.data;
    data.g = ON.all->loss->first_derivative(ON.all->sd, ec.pred.scalar, ec.l.simple.label)*ec.l.simple.weight;
    data.g /= 2; // for half square loss...

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
        if (ON.t % (ON.epoch_size * 100) == 0) ON.update_D();
    }

    memset(data.Zx, 0, sizeof(float)* (ON.m+1));
    GD::foreach_feature<update_data, update_wbar_and_Zx>(*ON.all, ec, data);
    ON.compute_AZx();

    ON.update_b();

    if (ON.cnt == ON.epoch_size) {
        ON.cnt = 0;
        for (int k = 0; k < ON.epoch_size; k++) {
            VW::finish_example(*ON.all, ON.buffer[k]);
        }
    }
}


void save_load(OjaNewton& ON, io_buf& model_file, bool read, bool text) {
    vw* all = ON.all;
    if (read) {
        initialize_regressor(*all);
        ON.initialize_Z();
    }

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

base_learner* OjaNewton_setup(vw& all) {
    if (missing_option(all, false, "OjaNewton", "Online Newton with Oja's Sketch"))
        return nullptr;

    new_options(all, "OjaNewton options")
        ("sketch_size", po::value<int>(), "size of sketch")
        ("epoch_size", po::value<int>(), "size of epoch")
        ("alpha", po::value<float>(), "mutiplicative constant for indentiy")
        ("learning_rate_cnt", po::value<float>(), "constant for the learning rate 1/t");
    add_options(all);

    po::variables_map& vm = all.vm;

    OjaNewton& ON = calloc_or_die<OjaNewton>();
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
        ON.alpha = 1.0;

    if (vm.count("learning_rate_cnt"))
        ON.learning_rate_cnt = vm["learning_rate_cnt"].as<float>();
    else
        ON.learning_rate_cnt = 10;


    ON.cnt = 0;
    ON.t = 1;

    ON.ev = calloc_or_die<float>(ON.m+1);
    ON.b = calloc_or_die<float>(ON.m+1);
    ON.D = calloc_or_die<double>(ON.m+1);
    ON.A = calloc_or_die<double*>(ON.m+1);
    ON.K = calloc_or_die<double*>(ON.m+1);
    for (int i = 1; i <= ON.m; i++) {
        ON.A[i] = calloc_or_die<double>(ON.m+1);
        ON.K[i] = calloc_or_die<double>(ON.m+1);
        ON.A[i][i] = 1;
        ON.K[i][i] = 1;
        ON.D[i] = 1;
    }

    ON.buffer = calloc_or_die<example*>(ON.epoch_size);
    ON.weight_buffer = calloc_or_die<float>(ON.epoch_size);

    ON.data.ON = &ON;
    ON.data.Zx = calloc_or_die<float>(ON.m+1);
    ON.data.AZx = calloc_or_die<float>(ON.m+1);
    ON.data.delta = calloc_or_die<float>(ON.m+1);

    all.reg.stride_shift = ceil(log2(ON.m + 1));

    learner<OjaNewton>& l = init_learner(&ON, learn, 1 << all.reg.stride_shift);

    l.set_predict(predict);
    l.set_save_load(save_load);
    l.set_finish_example(keep_example);
    return make_base(l);
}
