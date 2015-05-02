#include "interactions.h"

// uncomment to perform generic loop for interactions of all levels
// #define GEN_INTER_LOOP

namespace INTERACTIONS
{

//struct eval_gen_data
//{
//    size_t& new_features_cnt;
//    float& new_features_weight;
//    eval_gen_data(size_t& features_cnt, float& features_weight): new_features_cnt(features_cnt), new_features_weight(features_weight) {}
//};

//void ft_cnt(eval_gen_data& dat, float fx, uint32_t /*fw*/)
//{
//    ++ dat.new_features_cnt;
//    dat.new_features_weight += fx * fx;
//}

inline size_t factor(const size_t n, size_t start_from = 2)
{
    if (n == 0) return 1.;
    else
    {
        size_t res = 1;
        for (size_t i = start_from; i <= n; ++i)
            res *= i;
        return res;
    }
}

void eval_count_of_generated_ft(vw& all, example& ec, size_t& new_features_cnt, float& new_features_weight)
{
    new_features_cnt = 0;
    new_features_weight = 0.;

    v_array<float> results = v_init<float>();

    if (all.permutations)
    {
        for (vector<string>::const_iterator inter = all.interactions.begin(); inter != all.interactions.end(); ++inter)
        {
            size_t num_features_in_inter = 1;
            float sum_feat_sq_in_inter = 1.;
            const size_t len = inter->length();


            for (size_t j = 0; j < len; ++j)
            {
                const int ns = (*inter)[j];
                num_features_in_inter *= ec.atomics[ns].size();
                sum_feat_sq_in_inter *= ec.sum_feat_sq[ns];
                if (num_features_in_inter == 0) break;
            }

            if (num_features_in_inter == 0) continue;

            new_features_cnt += num_features_in_inter;
            new_features_weight += sum_feat_sq_in_inter;
        }


    } else
    {
//        eval_gen_data dat(new_features_cnt, new_features_weight);
//        generate_interactions<eval_gen_data, uint32_t, ft_cnt>(all, ec, dat);
//return;
        for (vector<string>::const_iterator inter = all.interactions.begin(); inter != all.interactions.end(); ++inter)
        {
            size_t num_features_in_inter = 1;
            float sum_feat_sq_in_inter = 1.;

            for (std::string::const_iterator ns = inter->begin(); ns != inter->end(); ++ns)
            {
                if (*ns != *(ns+1)) // different ns
                {
                    const int nsc = *ns;
                    num_features_in_inter *= ec.atomics[nsc].size();
                    sum_feat_sq_in_inter *= ec.sum_feat_sq[nsc];
                    if (num_features_in_inter == 0) break;
                } else {
                    // let's find a length of a block of same ns
                    size_t order_of_inter = 2;
                    std::string::const_iterator ns_end = ns+1;
                    while (*ns == *(++ns_end)) ++order_of_inter;

                    v_array<feature>& features = ec.atomics[(const int)*ns];
                    const size_t ft_size = features.size();
                    if (ft_size < order_of_inter) break;

                    // let's calculate num of features and their weight for whole block

                    float ft_wt_sum = 0.;
                    size_t cnt_ft_weight_non_1 = 0;

                    for(size_t i = 0; i < results.size(); ++i) results[i] = 0;
                    while (results.size() < order_of_inter) results.push_back(0.);

                    for (feature* ft = features.begin; ft != features.end; ++ft)
                    {
                        const float x = ft->x*ft->x;

                        for (size_t i = 0; i < order_of_inter-1; ++i)
                        results[i] += results[i+1]*x;

                        results[order_of_inter-1] += ft_wt_sum*x;
                        ft_wt_sum += x;

                        if (ft->x != 1. && feature_self_interactions_for_weight_other_than_1)
                        {
                            results[order_of_inter-1] += x*x;
                            ++cnt_ft_weight_non_1;
                        }

                    }

                    // number of generated features = c(n,k) = n!/(n-k)!/k!
                    size_t n = factor(ft_size, ft_size-order_of_inter+1);
                    n /= factor(order_of_inter); // k!

                    // let's adjust with cnt_ft_weight_non_1 features
                    if (cnt_ft_weight_non_1 != 0)
                    {
                        size_t num = 1;
                        for (size_t m = 1; m < order_of_inter-1; ++m)
                            num += factor(ft_size-1, ft_size-m)/factor(m);
                        num *= cnt_ft_weight_non_1;

                        for (size_t j = 2; j <= order_of_inter / 2; ++j)
                        {
                            size_t spaces_left = order_of_inter - j*2;
                            num += factor(cnt_ft_weight_non_1 + spaces_left -1, cnt_ft_weight_non_1)/factor(spaces_left);
                        }
                        n += num;
                    }

                    num_features_in_inter *= n;


                    sum_feat_sq_in_inter *= results[0];

                    ns += order_of_inter-1;

                }
            }


            if (num_features_in_inter == 0) continue;

            new_features_cnt += num_features_in_inter;
            new_features_weight += sum_feat_sq_in_inter;
        }





    }

    results.delete_v();
}

//some helper functions

inline bool valid_ns(char c)
{
    return !(c == '|' || c == ':');
}

// expand namespace interactions if contain wildcards

void expand_namespace_depth(const string& ns, vector<string>& res, string val,  size_t pos)
{
    assert (pos <= ns.length());

    if (pos == ns.length())
    {
        res.push_back(val);
    }
    else
        if (ns[pos] != ':')
        {
            val.push_back(ns[pos]);
            expand_namespace_depth(ns, res, val, pos+1);
        }
        else
        {
            res.reserve(res.size() + valid_ns_size);
            for (unsigned char j = printable_start; j <= printable_end; j++)
            {
                if(valid_ns(j))
                {
                    val.push_back(j);
                    expand_namespace_depth(ns, res, val, pos+1);
                    val.pop_back();
                }
            }
        }
}

size_t filter_duplicate_interactions(vector<string>& vec)
{
    // always preliminary sort interactions
    for (vector<string>::iterator i = vec.begin(); i != vec.end(); ++i)
        std::sort(i->begin(), i->end());

    size_t cnt = vec.size();
    sort(vec.begin(), vec.end());
    vec.erase(unique(vec.begin(), vec.end()), vec.end());
    return cnt - vec.size();
}

vector<string> expand_interactions(const vector<string>& vec, const size_t req_length, const string err_msg)
{
    vector<string> res;

    for (vector<string>::const_iterator i = vec.begin(); i != vec.end(); ++i)
    {
        const size_t len = i->length();
        if (req_length > 0 && len != req_length)
        {
            cerr << endl << err_msg << endl;
            throw exception();
        } else
            if (len < 2)
            {
                cerr << endl << "error, feature interactions must involve at least two namespaces.\n";
                throw exception();
            }

        INTERACTIONS::expand_namespace(*i, res);
    }
    return res;
}

}
