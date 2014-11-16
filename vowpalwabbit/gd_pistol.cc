/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <algorithm>
#include <fstream>
#include <sstream>
#include <float.h>
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <netdb.h>
#endif
#include <string.h>
#include <stdio.h>
#include <assert.h>

#if defined(__SSE2__) && !defined(VW_LDA_NO_SSE)
#include <xmmintrin.h>
#endif

#include "gd_pistol.h"
#include "simple_label.h"
#include "accumulate.h"
#include "reductions.h"

using namespace std;
using namespace LEARNER;

namespace GD_PISTOL {

    struct gd {
        size_t no_win_counter;
        size_t early_stop_thres;
        float initial_constant;

        float a;
        float b;
        float alpha_single;
        float max_input;
        float norm_current_example;
        float squared_norm_current_example;
        float squared_norm_theta;
        void (*predict)(gd&, learner&, example&);

        vw* all;
    };
    
    struct update_struct {
        float weighted_deriv;
        float a;
        float b;
    };

    template<bool feature_mask_off, bool w_only>
    inline void update_theta_alpha_w_multi(update_struct& us, float x, float& fw) {
        if (feature_mask_off || fw != 0.) {
            weight* w = &fw;
            if (!w_only) {
                w[1] -= x * us.weighted_deriv; //update theta
                w[2] += fabs(x * us.weighted_deriv); //update alpha for each coordinate
            }
            float squared_val = w[1] * w[1];
            float tmp = 1.f / (us.a * w[3]*(w[2] + w[3]));
            //w[0] = sqrt(2 * w[3] * us.a * us.b) * w[1] * exp(squared_val / 2 * tmp) * tmp;
            //w[0] = w[3] * us.b * w[1] * exp(squared_val / 2 * tmp) * tmp;
            w[0] = sqrt(w[2]) * us.b * w[1] * exp(squared_val / 2 * tmp) * tmp;
        }
    }

    template<bool feature_mask_off>
    inline void update_theta_single(float& weighted_deriv, float x, float& fw) {
        if (feature_mask_off || fw != 0.) {
            weight* w = &fw;
            w[0] -= x*weighted_deriv; //update theta
        }
    }

    template<bool feature_mask_off>
    inline void calculate_squared_l2_example(float& squared_norm, float x, float& fw) {
        if (feature_mask_off) {
            squared_norm += x*x; //update alpha for each coordinate
        }
    }

    template<bool feature_mask_off>
    inline void calculate_l_inf_example(update_struct& us, float x, float& fw) {
        if (feature_mask_off) {
            weight* w = &fw;
            float fx = fabs(x);
            if (fx > w[3]) {
                w[3] = fx;
                float squared_val = w[1] * w[1];
                float tmp = 1.f / (us.a * w[3] * (w[2] + w[3]));
                //w[0] = sqrt(2 * w[3] * us.a * us.b) * w[1] * exp(squared_val / 2 * tmp) * tmp;
                //w[0] = w[3] * us.b * w[1] * exp(squared_val / 2 * tmp) * tmp;
                w[0] = sqrt(w[2]) * us.b * w[1] * exp(squared_val / 2 * tmp) * tmp;
            }
        }
    }

    void end_pass(gd& g) {
        vw& all = *g.all;

        if (all.sd->contraction!=1) {
            /* It multiplies the weight vector to the constant factor.
             * The vector theta always contains the "true" information.
             */
            uint32_t length = 1 << all.num_bits;
            size_t stride = 1 << all.reg.stride_shift;
            for (uint32_t i = 0; i < length && all.reg_mode; i++)
                all.reg.weight_vector[stride * i] *= (float) all.sd->contraction;
            all.sd->contraction = 1.;
        }
        
        if (all.span_server != "") {
            if (all.adaptive)
                accumulate_weighted_avg(all, all.span_server, all.reg);
            else
                accumulate_avg(all, all.span_server, all.reg, 0);
        }

        if (all.save_per_pass)
            save_predictor(all, all.final_regressor_name, all.current_pass);

        all.current_pass++;

        if (!all.holdout_set_off) {
            if (summarize_holdout_set(all, g.no_win_counter))
                finalize_regressor(all, all.final_regressor_name);
            if ((g.early_stop_thres == g.no_win_counter) &&
                    ((all.check_holdout_every_n_passes <= 1) ||
                    ((all.current_pass % all.check_holdout_every_n_passes) == 0)))
                set_done(all);
        }
    }

    struct string_value {
        float v;
        string s;
        friend bool operator<(const string_value& first, const string_value& second);
    };

    bool operator<(const string_value& first, const string_value& second) {
        return fabs(first.v) > fabs(second.v);
    }

    void audit_feature(vw& all, feature* f, audit_data* a, vector<string_value>& results, string prepend, string& ns_pre, size_t offset = 0, float mult = 1) {
        ostringstream tempstream;
        size_t index = (f->weight_index + offset) & all.reg.weight_mask;
        weight* weights = all.reg.weight_vector;
        size_t stride_shift = all.reg.stride_shift;

        if (all.audit) tempstream << prepend;

        string tmp = "";

        if (a != NULL) {
            tmp += a->space;
            tmp += '^';
            tmp += a->feature;
        }

        if (a != NULL && all.audit) {
            tempstream << tmp << ':';
        } else if (index == ((((constant << stride_shift) * all.wpp + offset) & all.reg.weight_mask)) && all.audit) {
            tempstream << "Constant:";
        }
        if (all.audit) {
            tempstream << ((index >> stride_shift) & all.parse_mask) << ':' << mult * f->x;
            tempstream << ':' << weights[index];
        }
        if (all.current_pass == 0 && all.inv_hash_regressor_name != "") { //for invert_hash
            if (index == (((constant << stride_shift) * all.wpp + offset) & all.reg.weight_mask))
                tmp = "Constant";

            ostringstream convert;
            convert << ((index >> stride_shift) & all.parse_mask);
            tmp = ns_pre + tmp + ":" + convert.str();

            if (!all.name_index_map.count(tmp)) {
                all.name_index_map.insert(std::map< std::string, size_t>::value_type(tmp, ((index >> stride_shift) & all.parse_mask)));
            }
        }

        if (all.adaptive && all.audit)
            tempstream << '@' << weights[index + 1];
        string_value sv = {weights[index] * f->x, tempstream.str()};
        results.push_back(sv);
    }

    void audit_features(vw& all, v_array<feature>& fs, v_array<audit_data>& as, vector<string_value>& results, string prepend, string& ns_pre, size_t offset = 0, float mult = 1) {
        for (size_t j = 0; j < fs.size(); j++)
            if (as.begin != as.end)
                audit_feature(all, & fs[j], & as[j], results, prepend, ns_pre, offset, mult);
            else
                audit_feature(all, & fs[j], NULL, results, prepend, ns_pre, offset, mult);
    }

    void audit_quad(vw& all, feature& left_feature, audit_data* left_audit, v_array<feature> &right_features, v_array<audit_data> &audit_right, vector<string_value>& results, string& ns_pre, uint32_t offset = 0) {
        size_t halfhash = quadratic_constant * (left_feature.weight_index + offset);

        ostringstream tempstream;
        if (audit_right.size() != 0 && left_audit && all.audit)
            tempstream << left_audit->space << '^' << left_audit->feature << '^';
        string prepend = tempstream.str();

        if (all.current_pass == 0 && audit_right.size() != 0 && left_audit)//for invert_hash
        {
            ns_pre = left_audit->space;
            ns_pre = ns_pre + '^' + left_audit->feature + '^';
        }

        audit_features(all, right_features, audit_right, results, prepend, ns_pre, halfhash + offset, left_audit->x);
    }

    void audit_triple(vw& all, feature& f0, audit_data* f0_audit, feature& f1, audit_data* f1_audit,
            v_array<feature> &right_features, v_array<audit_data> &audit_right, vector<string_value>& results, string& ns_pre, uint32_t offset = 0) {
        size_t halfhash = cubic_constant2 * (cubic_constant * (f0.weight_index + offset) + f1.weight_index + offset);

        ostringstream tempstream;
        if (audit_right.size() > 0 && f0_audit && f1_audit && all.audit)
            tempstream << f0_audit->space << '^' << f0_audit->feature << '^'
                << f1_audit->space << '^' << f1_audit->feature << '^';
        string prepend = tempstream.str();

        if (all.current_pass == 0 && audit_right.size() != 0 && f0_audit && f1_audit)//for invert_hash
        {
            ns_pre = f0_audit->space;
            ns_pre = ns_pre + '^' + f0_audit->feature + '^' + f1_audit->space + '^' + f1_audit->feature + '^';
        }
        audit_features(all, right_features, audit_right, results, prepend, ns_pre, halfhash + offset);
    }

    void print_features(vw& all, example& ec) {
        weight* weights = all.reg.weight_vector;

        if (all.lda > 0) {
            size_t count = 0;
            for (unsigned char* i = ec.indices.begin; i != ec.indices.end; i++)
                count += ec.atomics[*i].size();
            for (unsigned char* i = ec.indices.begin; i != ec.indices.end; i++)
                for (audit_data *f = ec.audit_features[*i].begin; f != ec.audit_features[*i].end; f++) {
                    cout << '\t' << f->space << '^' << f->feature << ':' << ((f->weight_index >> all.reg.stride_shift) & all.parse_mask) << ':' << f->x;
                    for (size_t k = 0; k < all.lda; k++)
                        cout << ':' << weights[(f->weight_index + k) & all.reg.weight_mask];
                }
            cout << " total of " << count << " features." << endl;
        } else {
            vector<string_value> features;
            string empty;
            string ns_pre;

            for (unsigned char* i = ec.indices.begin; i != ec.indices.end; i++) {
                ns_pre = "";
                audit_features(all, ec.atomics[*i], ec.audit_features[*i], features, empty, ns_pre, ec.ft_offset);
                ns_pre = "";
            }
            for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end(); i++) {
                int fst = (*i)[0];
                int snd = (*i)[1];
                for (size_t j = 0; j < ec.atomics[fst].size(); j++) {
                    audit_data* a = NULL;
                    if (ec.audit_features[fst].size() > 0)
                        a = &ec.audit_features[fst][j];
                    audit_quad(all, ec.atomics[fst][j], a, ec.atomics[snd], ec.audit_features[snd], features, ns_pre);
                }
            }

            for (vector<string>::iterator i = all.triples.begin(); i != all.triples.end(); i++) {
                int fst = (*i)[0];
                int snd = (*i)[1];
                int trd = (*i)[2];
                for (size_t j = 0; j < ec.atomics[fst].size(); j++) {
                    audit_data* a1 = NULL;
                    if (ec.audit_features[fst].size() > 0)
                        a1 = &ec.audit_features[fst][j];
                    for (size_t k = 0; k < ec.atomics[snd].size(); k++) {
                        audit_data* a2 = NULL;
                        if (ec.audit_features[snd].size() > 0)
                            a2 = &ec.audit_features[snd][k];
                        audit_triple(all, ec.atomics[fst][j], a1, ec.atomics[snd][k], a2, ec.atomics[trd], ec.audit_features[trd], features, ns_pre);
                    }
                }
            }

            sort(features.begin(), features.end());
            if (all.audit) {
                for (vector<string_value>::iterator sv = features.begin(); sv != features.end(); sv++)
                    cout << '\t' << (*sv).s;
                cout << endl;
            }
        }
    }

    void print_audit_features(vw& all, example& ec) {
        label_data& ld = *(label_data*) ec.ld;
        if (all.audit)
            print_result(all.stdout_fileno, ld.prediction, -1, ec.tag);
        fflush(stdout);
        print_features(all, ec);
    }

    /* Truncate the prediction between sd->min_label and sd->max_label*/
    float finalize_prediction(shared_data* sd, float ret) {
        if (nanpattern(ret)) {
            cerr << "NAN prediction in example " << sd->example_number + 1 << ", forcing 0.0" << endl;
            return 0.;
        }
        if (ret > sd->max_label)
            return (float) sd->max_label;
        if (ret < sd->min_label)
            return (float) sd->min_label;
        return ret;
    }

    template<bool feature_mask_off, bool adaptive, bool audit>
    void predict(gd& g, learner& base, example& ec) {
        vw& all = *g.all;

        ec.partial_prediction = inline_predict(all, ec) * (float)g.all->sd->contraction;

        label_data& ld = *(label_data*) ec.ld;
        ld.prediction = finalize_prediction(all.sd, ec.partial_prediction);

        if (audit)
            print_audit_features(all, ec);
    }

    /* invariant: not a test label, importance weight > 0 */
    template<bool feature_mask_off, size_t adaptive>
    void update(gd& g, learner& base, example& ec) {
        label_data* ld = (label_data*) ec.ld;
        vw& all = *g.all;

        //float loss = all.loss->getLoss(all.sd, ld->prediction, ld->label);
        float weighted_deriv = all.loss->first_derivative(all.sd, ld->prediction, ld->label) * ld->weight;

        if (fabs(weighted_deriv) > 1e-8) {
            if (adaptive) {
                update_struct us;
                us.weighted_deriv = weighted_deriv;
                us.a = g.a;
                us.b = g.b;
                foreach_feature<update_struct, update_theta_alpha_w_multi<feature_mask_off, false> >(*g.all, ec, us);
            } else {
                foreach_feature<float, update_theta_single<feature_mask_off> >(*g.all, ec, weighted_deriv);
                g.alpha_single += g.norm_current_example * fabs(weighted_deriv);
                g.squared_norm_theta = g.squared_norm_theta - 2 * weighted_deriv * ec.partial_prediction + weighted_deriv * weighted_deriv * g.squared_norm_current_example;
                if (g.squared_norm_theta < 0)
                    g.squared_norm_theta = 0; // numerical problems might happen, better be safe.
                float tmp = 1.f / (g.max_input * g.a * (g.alpha_single + g.max_input));
                //g.all->sd->contraction = sqrt(2 * g.max_input * g.a * g.b) * exp(g.squared_norm_theta / 2 * tmp) * tmp;
                g.all->sd->contraction = sqrt(g.alpha_single) * g.b * exp(g.squared_norm_theta / 2 * tmp) * tmp;
            }
        }
    }

    template<bool feature_mask_off, size_t adaptive>
    void learn(gd& g, learner& base, example& ec) {//invariant: not a test label, importance weight > 0
        assert(ec.in_use);
        assert(((label_data*) ec.ld)->label != FLT_MAX);
        assert(((label_data*) ec.ld)->weight > 0.);

        if (adaptive) {            
            //g.b++;
            update_struct us;
            us.a = g.a;
            us.b = g.b;
            foreach_feature<update_struct, calculate_l_inf_example<feature_mask_off> >(*g.all, ec, us);
        } else {
            g.squared_norm_current_example = 0;
            foreach_feature<float, calculate_squared_l2_example<feature_mask_off> >(*g.all, ec, g.squared_norm_current_example);
            g.norm_current_example = sqrt(g.squared_norm_current_example);
            if (g.max_input < g.norm_current_example) {
                g.max_input = g.norm_current_example;
                float tmp = 1.f / (g.max_input * g.a * (g.alpha_single + g.max_input));
                //g.all->sd->contraction = sqrt(2 * g.max_input * g.a * g.b) * exp(g.squared_norm_theta / 2 * tmp) * tmp;
                g.all->sd->contraction = sqrt(g.alpha_single) * g.b * exp(g.squared_norm_theta / 2 * tmp) * tmp;
            }
        }

        g.predict(g, base, ec);

        update<feature_mask_off, adaptive>(g, base, ec);
    }

    void save_load_regressor(vw& all, io_buf& model_file, bool read, bool text) {
        uint32_t length = 1 << all.num_bits;
        uint32_t stride = 1 << all.reg.stride_shift;
        int c = 0;
        uint32_t i = 0;
        size_t brw = 1;

        if (all.print_invert) { //write readable model with feature names           
            weight* v;
            char buff[512];
            int text_len;
            typedef std::map< std::string, size_t> str_int_map;

            for (str_int_map::iterator it = all.name_index_map.begin(); it != all.name_index_map.end(); ++it) {
                v = &(all.reg.weight_vector[stride * (it->second)]);
                if (*v != 0.) {
                    text_len = sprintf(buff, "%s", (char*) it->first.c_str());
                    brw = bin_text_write_fixed(model_file, (char*) it->first.c_str(), sizeof (*it->first.c_str()), buff, text_len, true);
                    text_len = sprintf(buff, ":%f\n", *v);
                    brw += bin_text_write_fixed(model_file, (char *) v, sizeof (*v), buff, text_len, true);
                }
            }
        } else {
            do {
                brw = 1;
                weight* v;
                if (read) {
                    c++;
                    brw = bin_read_fixed(model_file, (char*) &i, sizeof (i), "");
                    if (brw > 0) {
                        assert(i < length);
                        v = &(all.reg.weight_vector[stride * i]);
                        brw += bin_read_fixed(model_file, (char*) v, sizeof (*v), "");
                    }
                } else { // write binary or text
                    v = &(all.reg.weight_vector[stride * i]);
                    if (*v != 0.) {
                        c++;
                        char buff[512];
                        int text_len;

                        text_len = sprintf(buff, "%d", i);
                        brw = bin_text_write_fixed(model_file, (char *) &i, sizeof (i),
                                buff, text_len, text);

                        text_len = sprintf(buff, ":%f\n", *v);
                        brw += bin_text_write_fixed(model_file, (char *) v, sizeof (*v),
                                buff, text_len, text);
                    }
                }

                if (!read)
                    i++;
            } while ((!read && i < length) || (read && brw > 0));
        }
    }

    void save_load_online_state(gd& g, io_buf& model_file, bool read, bool text) {
        vw& all = *g.all;

        char buff[512];

        uint32_t text_len = sprintf(buff, "a %f\n", g.a);
        bin_text_read_write_fixed(model_file, (char*) &(g.a), sizeof (g.a),
                "", read,
                buff, text_len, text);

        text_len = sprintf(buff, "b %f\n", g.b);
        bin_text_read_write_fixed(model_file, (char*) &(g.b), sizeof (g.b),
                "", read,
                buff, text_len, text);

        text_len = sprintf(buff, "alpha_single %f\n", g.alpha_single);
        bin_text_read_write_fixed(model_file, (char*) &(g.alpha_single), sizeof (g.alpha_single),
                "", read,
                buff, text_len, text);
        
        text_len = sprintf(buff, "alpha_single %f\n", g.max_input);
        bin_text_read_write_fixed(model_file, (char*) &(g.max_input), sizeof (g.max_input),
                "", read,
                buff, text_len, text);
        
        text_len = sprintf(buff, "squared_norm_theta %f\n", g.squared_norm_theta);
        bin_text_read_write_fixed(model_file, (char*) &(g.squared_norm_theta), sizeof (g.squared_norm_theta),
                "", read,
                buff, text_len, text);
        
        text_len = sprintf(buff, "sum_loss %f\n", all.sd->sum_loss);
        bin_text_read_write_fixed(model_file, (char*) &all.sd->sum_loss, sizeof (all.sd->sum_loss),
                "", read,
                buff, text_len, text);

        text_len = sprintf(buff, "sum_loss_since_last_dump %f\n", all.sd->sum_loss_since_last_dump);
        bin_text_read_write_fixed(model_file, (char*) &all.sd->sum_loss_since_last_dump, sizeof (all.sd->sum_loss_since_last_dump),
                "", read,
                buff, text_len, text);

        text_len = sprintf(buff, "dump_interval %f\n", all.sd->dump_interval);
        bin_text_read_write_fixed(model_file, (char*) &all.sd->dump_interval, sizeof (all.sd->dump_interval),
                "", read,
                buff, text_len, text);

        text_len = sprintf(buff, "min_label %f\n", all.sd->min_label);
        bin_text_read_write_fixed(model_file, (char*) &all.sd->min_label, sizeof (all.sd->min_label),
                "", read,
                buff, text_len, text);

        text_len = sprintf(buff, "max_label %f\n", all.sd->max_label);
        bin_text_read_write_fixed(model_file, (char*) &all.sd->max_label, sizeof (all.sd->max_label),
                "", read,
                buff, text_len, text);

        text_len = sprintf(buff, "weighted_examples %f\n", all.sd->weighted_examples);
        bin_text_read_write_fixed(model_file, (char*) &all.sd->weighted_examples, sizeof (all.sd->weighted_examples),
                "", read,
                buff, text_len, text);

        text_len = sprintf(buff, "weighted_labels %f\n", all.sd->weighted_labels);
        bin_text_read_write_fixed(model_file, (char*) &all.sd->weighted_labels, sizeof (all.sd->weighted_labels),
                "", read,
                buff, text_len, text);

        text_len = sprintf(buff, "weighted_unlabeled_examples %f\n", all.sd->weighted_unlabeled_examples);
        bin_text_read_write_fixed(model_file, (char*) &all.sd->weighted_unlabeled_examples, sizeof (all.sd->weighted_unlabeled_examples),
                "", read,
                buff, text_len, text);

        text_len = sprintf(buff, "example_number %u\n", (uint32_t) all.sd->example_number);
        bin_text_read_write_fixed(model_file, (char*) &all.sd->example_number, sizeof (all.sd->example_number),
                "", read,
                buff, text_len, text);

        text_len = sprintf(buff, "total_features %u\n", (uint32_t) all.sd->total_features);
        bin_text_read_write_fixed(model_file, (char*) &all.sd->total_features, sizeof (all.sd->total_features),
                "", read,
                buff, text_len, text);
        
        if (!all.training) // reset various things so that we report test set performance properly
        {
            all.sd->sum_loss = 0;
            all.sd->sum_loss_since_last_dump = 0;
            all.sd->dump_interval = 1.;
            all.sd->weighted_examples = 0.;
            all.sd->weighted_labels = 0.;
            all.sd->weighted_unlabeled_examples = 0.;
            all.sd->example_number = 0;
            all.sd->total_features = 0;
        }

        uint32_t length = 1 << all.num_bits;
        uint32_t stride = 1 << all.reg.stride_shift;
        int c = 0;
        uint32_t i = 0;
        size_t brw = 1;
        do {
            brw = 1;
            weight* v;
            if (read) {
                c++;
                brw = bin_read_fixed(model_file, (char*) &i, sizeof (i), "");
                if (brw > 0) {
                    assert(i < length);
                    v = &(all.reg.weight_vector[stride * i]);
                    brw += bin_read_fixed(model_file, (char*) v, sizeof (*v) * stride, "");
                    //if (!all.training)
                    //    v[1] = v[2] = 0.;
                }
            } else { // write binary or text
                v = &(all.reg.weight_vector[stride * i]);
                if (*v != 0.) {
                    c++;
                    char buff[512];
                    int text_len = sprintf(buff, "%d", i);
                    brw = bin_text_write_fixed(model_file, (char *) &i, sizeof (i),
                            buff, text_len, text);

                    text_len = sprintf(buff, ":%f %f\n", *v, *(v + 1));
                    brw += bin_text_write_fixed(model_file, (char *) v, stride * sizeof (*v),
                            buff, text_len, text);
                }
            }
            if (!read)
                i++;
        } while ((!read && i < length) || (read && brw > 0));
    }

    /* Main save_load function */
    void save_load(gd& g, io_buf& model_file, bool read, bool text) {
        vw& all = *g.all;
        if (read) {
            initialize_regressor(all);

            if (!all.adaptive)
                g.alpha_single = 0;

            if (g.initial_constant != 0.0)
                VW::set_weight(all, constant, 0, g.initial_constant);
        }

        if (model_file.files.size() > 0) {
            bool resume = all.save_resume;
            char buff[512];
            uint32_t text_len = sprintf(buff, ":%d\n", resume);
            bin_text_read_write_fixed(model_file, (char *) &resume, sizeof (resume), "", read, buff, text_len, text);
            if (resume)
                save_load_online_state(g, model_file, read, text);
            else
                save_load_regressor(all, model_file, read, text);
        }
    }

    uint32_t ceil_log_2(uint32_t v) {
        if (v == 0)
            return 0;
        else
            return 1 + ceil_log_2(v >> 1);
    }

    learner* setup(vw& all, po::variables_map& vm) {
        all.adaptive = false;
        gd* g = (gd*) calloc_or_die(1, sizeof (gd));
        g->all = &all;
        g->no_win_counter = 0;
        g->early_stop_thres = 3;

        g->a = 1;
        g->b = .5;
        g->max_input = 0;
        g->squared_norm_theta = 0;
        g->alpha_single = 0;
        g->all->sd->contraction = 1;

        bool feature_mask_off = true;
        if (vm.count("feature_mask"))
            feature_mask_off = false;

        if (!all.holdout_set_off) {
            all.sd->holdout_best_loss = FLT_MAX;
            if (vm.count("early_terminate"))
                g->early_stop_thres = vm["early_terminate"].as< size_t>();
        }

        if (vm.count("constant"))
            g->initial_constant = vm["constant"].as<float>();

        if (vm.count("adaptive"))
            all.adaptive = true;

        learner* ret = new learner(g, 1);

        if (all.adaptive) {
            if (feature_mask_off) {
                ret->set_learn<gd, learn<true, true> >();
                ret->set_update<gd, update<true, true> >();
                if (all.audit || all.hash_inv) {
                    ret->set_predict<gd, predict<true, true, true> >();
                    g->predict = predict<true, true, true>;
                } else {
                    ret->set_predict<gd, predict<true, true, false> >();
                    g->predict = predict<true, true, false>;
                }
            } else {
                ret->set_learn<gd, learn<false, true> >();
                ret->set_update<gd, update<false, true> >();
                if (all.audit || all.hash_inv) {
                    ret->set_predict<gd, predict<false, true, true> >();
                    g->predict = predict<false, true, true>;
                } else {
                    ret->set_predict<gd, predict<false, true, false> >();
                    g->predict = predict<false, true, false>;
                }
            }
        } else {
            if (feature_mask_off) {
                ret->set_learn<gd, learn<true, false> >();
                ret->set_update<gd, update<true, false> >();
                if (all.audit || all.hash_inv) {
                    ret->set_predict<gd, predict<true, false, true> >();
                    g->predict = predict<true, false, true>;
                } else {
                    ret->set_predict<gd, predict<true, false, false> >();
                    g->predict = predict<true, false, false>;
                }
            } else {
                ret->set_learn<gd, learn<false, false> >();
                ret->set_update<gd, update<false, false> >();
                if (all.audit || all.hash_inv) {
                    ret->set_predict<gd, predict<false, false, true> >();
                    g->predict = predict<false, false, true>;
                } else {
                    ret->set_predict<gd, predict<false, false, false> >();
                    g->predict = predict<false, false, false>;
                }
            }
        }

        uint32_t stride; // number of vectors are kept in memory
        if (all.adaptive) {
            stride = 4;
        } else
            stride = 2;

        all.reg.stride_shift = ceil_log_2(stride - 1);
        ret->increment = ((uint64_t) 1 << all.reg.stride_shift);

        ret->set_save_load<gd, save_load>();
        ret->set_end_pass<gd, end_pass>();

        return ret;
    }
}
