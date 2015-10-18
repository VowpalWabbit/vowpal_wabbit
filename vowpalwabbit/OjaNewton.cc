/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
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
  float sketch;
  float xx;
  float *Zx;
  float *AZx;
  float *beta;
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
  double **A;
  double **GZ;

  example **buffer;
  float *weight_buffer;
  struct update_data data;

  void initialize_Z()
  {
    size_t stride_shift = all->reg.stride_shift;
    weight* weights = all->reg.weight_vector;

    for (int i = 1; i <= m; i++)
      weights[(i << stride_shift) + i - 1] = 1;
  }

  void compute_AZx()
  {
    for (int i = 0; i < m; i++) {
      data.AZx[i] = 0;
      for (int j = 0; j <= i; j++) {
        data.AZx[i] += A[i][j] * data.Zx[j];
      }
    }
  }

  void update_eigenvalues(float gamma)
  {
    for (int i = 0; i < m; i++) {
      float tmp = data.AZx[i] * data.sketch;

      if (t == 1) {
        ev[i] = gamma * tmp * tmp;
      }
      else {
        ev[i] = (1 - gamma) * t * ev[i] / (t - 1) + gamma * t * tmp * tmp;
      }
    }
  }

  void compute_beta(float gamma)
  {
    for (int i = 0; i < m; i++) {
      data.beta[i] = (gamma * (5 * i + 1)) * data.AZx[i] * data.sketch;
      for (int j = 0; j < i; j++) {
        data.beta[i] -= A[i][j] * data.beta[j];
      }
      data.beta[i] /= A[i][i];
    }
  }

  void update_GZ()
  {
    float tmp = data.xx * data.sketch * data.sketch;
    //cout << tmp << endl;
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < m; j++) {
        GZ[i][j] += data.beta[i] * data.Zx[j] * data.sketch;
        GZ[i][j] += data.beta[j] * data.Zx[i] * data.sketch;
        GZ[i][j] += data.beta[i] * data.beta[j] * tmp;
      }
    }
  }

  void update_A()
  {
    float *zv = calloc_or_die<float>(m);
    float *vv = calloc_or_die<float>(m);

    for (int i = 0; i < m; i++) {

      for (int j = 0; j < i; j++) {
        zv[j] = 0;
        for (int k = 0; k <= i; k++) {
          zv[j] += A[i][k] * GZ[k][j];
        }
        //cout << zv[j] << endl;
      }

      for (int j = 0; j < i; j++) {
        vv[j] = 0;
        for (int k = 0; k <= j; k++) {
          vv[j] += A[j][k] * zv[k];
        }
      }

      for (int j = 0; j < i; j++) {
        for (int k = j; k < i; k++) {
          A[i][j] -= vv[k] * A[k][j];
        }
      }

      double norm = 0;
      for (int j = 0; j <= i; j++) {
	double temp = 0;
        for (int k = 0; k <= i; k++) {
          temp += GZ[j][k] * A[i][k];
//          cout << "GZ = " << GZ[j][k] << " A[i][k] = " << A[i][k] << endl;
        }
//        cout << "temp = " << temp << " A[i][j] = " << A[i][j] << endl;
        norm += A[i][j]*temp;
      }
//      cout << "norm = " << norm << endl;
      norm = sqrt(norm);

      for (int j = 0; j <= i; j++) {
        A[i][j] /= norm;
      }
    }

    free(zv);
    free(vv);
  }

  void update_b()
  {
    for (int j = 0; j < m; j++) {
      for (int i = j; i < m; i++) {
        b[j] += ev[i] * data.AZx[i] * A[i][j] / (alpha * (alpha + ev[i]));
      }
    }
  }
};

void keep_example(vw& all, OjaNewton& ON, example& ec) {}

void make_pred(update_data& data, float x, float& wref) {
  int m = data.ON->m;
  float* w = &wref;

  for (int i = 0; i < m; i++) {
    data.prediction += w[i] * data.ON->b[i] * x;
  }
  data.prediction += w[m] * x;
}

void predict(OjaNewton& ON, base_learner&, example& ec) {
  ON.data.prediction = 0;
  GD::foreach_feature<update_data, make_pred>(*ON.all, ec, ON.data);
  ec.partial_prediction = ON.data.prediction;
//  cout << ON.data.prediction << endl;
  ec.pred.scalar = GD::finalize_prediction(ON.all->sd, ec.partial_prediction);
}


void update_Z_and_wbar(update_data& data, float x, float& wref) {
  float* w = &wref;
  float s = data.sketch * x;
  int m = data.ON->m;

  for (int i = 0; i < m; i++) {
    w[i] += data.beta[i] * s;
    w[m] -= data.beta[i] * s * data.ON->b[i];
  }
}

void get_Zx_and_xx(update_data& data, float x, float& wref) {
  float* w = &wref;

  for (int i = 0; i < data.ON->m; i++) {
    data.Zx[i] += w[i] * x;
  }
  data.xx += x * x;
}

void update_wbar_and_Zx(update_data& data, float x, float& wref) {
  float* w = &wref;
  float g = data.g * x;

  for (int i = 0; i < data.ON->m; i++) {
    data.Zx[i] += w[i] * g;
  }
  w[data.ON->m] -= g / data.ON->alpha;
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
      data.sketch = ON.weight_buffer[k];

      data.xx = 0;
      memset(data.Zx, 0, sizeof(float)* ON.m);
      GD::foreach_feature<update_data, get_Zx_and_xx>(*ON.all, ex, data);
      ON.compute_AZx();
      //cout << ON.data.Zx[0] << ' ' << ON.data.Zx[1] << endl;

      float gamma = 10.0 / ON.t;

      ON.update_eigenvalues(gamma);
      //cout << ON.ev[0] << ' ' << ON.ev[1] << endl;
      ON.compute_beta(gamma);
      //cout << ON.data.beta[0] << ' ' << ON.data.beta[1] << endl;

      ON.update_GZ();

      //cout << ON.GZ[0][0] << ' ' << ON.GZ[0][1] << endl;
      //cout << ON.GZ[1][0] << ' ' << ON.GZ[1][1] << endl;

      GD::foreach_feature<update_data, update_Z_and_wbar>(*ON.all, ex, data);
    }

    ON.update_A();

    //cout << ON.A[0][0] << ' ' << ON.A[0][1] << endl;
    //cout << ON.A[1][0] << ' ' << ON.A[1][1] << endl;
  }

  memset(data.Zx, 0, sizeof(float)* ON.m);
  GD::foreach_feature<update_data, update_wbar_and_Zx>(*ON.all, ec, data);
  ON.compute_AZx();

  //cout << ON.data.Zx[0] << ' ' << ON.data.Zx[1] << endl;

  ON.update_b();

  //cout << ON.b[0] << ' ' << ON.b[1] << endl;

  if (ON.cnt == ON.epoch_size) {
    ON.cnt = 0;
    //for (int k = 0; k < ON.epoch_size; k++) {
      //return_simple_example(*ON.all, nullptr, *ON.buffer[k]);
    //}
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
    ("epoch_size", po::value<float>(), "size of epoch")
    ("alpha", po::value<float>(), "mutiplicative constant for indentiy");
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

  ON.cnt = 0;
  ON.t = 1;

  ON.ev = calloc_or_die<float>(ON.m);
  ON.b = calloc_or_die<float>(ON.m);
  ON.A = calloc_or_die<double*>(ON.m);
  ON.GZ = calloc_or_die<double*>(ON.m);
  for (int i = 0; i < ON.m; i++) {
    ON.A[i] = calloc_or_die<double>(ON.m);
    ON.GZ[i] = calloc_or_die<double>(ON.m);
    ON.A[i][i] = 1;
    ON.GZ[i][i] = 1;
  }

  ON.buffer = calloc_or_die<example*>(ON.epoch_size);
  ON.weight_buffer = calloc_or_die<float>(ON.epoch_size);

  ON.data.ON = &ON;
  ON.data.Zx = calloc_or_die<float>(ON.m);
  ON.data.AZx = calloc_or_die<float>(ON.m);
  ON.data.beta = calloc_or_die<float>(ON.m);

  all.reg.stride_shift = ceil(log2(ON.m + 1));

  learner<OjaNewton>& l = init_learner(&ON, learn, 1 << all.reg.stride_shift);

  l.set_predict(predict);
  l.set_save_load(save_load);
  //l.set_finish_example(keep_example);
  return make_base(l);
}
