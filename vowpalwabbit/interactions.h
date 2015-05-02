#pragma once

#include "global_data.h"
#include "constant.h"

namespace INTERACTIONS
{

// By default include interactions of feature with itself when its weight != weight^2.
const bool feature_self_interactions_for_weight_other_than_1 = true;

// http://www.isthe.com/chongo/tech/comp/fnv/#FNV-param
const uint32_t FNV_prime = 16777619;

// state data for non-recursive iteration algorithm
// consists on N feature_gen_data records
// where N is length of interaction

struct feature_gen_data
{
    size_t loop_idx; // current feature id in namespace
    uint32_t hash;   // hash of feature interactions of previous namespaces in the list
    double x;        // weight of feature interactions of previous namespaces in the list
    size_t loop_end; // last feature id. May be less than number of features if namespace involved in interaction more than once
                     // calculated at preprocessing together with same_ns
    size_t same_ns;  // true if same namespace as previous is interaction list
    feature_gen_data(): loop_idx(0), x(1.), loop_end(0), same_ns(false) {}
};

void eval_count_of_generated_ft(vw& all, example& ec, size_t& new_features_cnt, float& new_features_weight);

template <class R, void (*T)(R&, const float, float&)>
inline void call_T( R& dat, weight* weight_vector, const size_t weight_mask, const float ft_weight, const size_t ft_idx)
{
    T(dat, ft_weight, weight_vector[ft_idx & weight_mask]);
}

template <class R, void (*T)(R&, float, uint32_t)>
inline void call_T( R& dat, weight* /*weight_vector*/, const size_t /*weight_mask*/, const float ft_weight, const size_t ft_idx)
{
    T(dat, ft_weight, ft_idx);
}

// generates features for all.interations
template <class R, class S, void (*T)(R&, float, S)>
inline void generate_interactions(vw& all, example& ec, R& dat)
{
//    if (!all.interactions.empty() && !v_array_contains(ec.indices, (unsigned char)interactions_namespace) )
//    {   // if cache wasn't created yet

//        v_array<feature>& cache = ec.atomics[interactions_namespace];
//        float& sum_feat_sq = ec.sum_feat_sq[interactions_namespace];

        const uint32_t offset = ec.ft_offset;
        const uint32_t stride_shift = all.reg.stride_shift;
        weight* weight_vector = all.reg.weight_vector;
        const size_t  weight_mask   = all.reg.weight_mask;

        v_array<feature_gen_data> state_data = v_init<feature_gen_data>(); // statedata for non-recursive iteration
        feature_gen_data empty_ns_data;  // befare: micro-optimization. don't want to call its constructor each time in loop.

        for (vector<string>::const_iterator it = all.interactions.begin(); it != all.interactions.end(); ++it)
        {
            const string ns = (*it);  // current list of namespaces to interact.
            // 'const string::operator[]' seems to be faster (http://stackoverflow.com/a/19920071/841424)
            const size_t len = ns.length();

#ifndef GEN_INTER_LOOP

            if (len == 2) //special case of pairs
            {
                const size_t fst_ns = ns[0];
                if (ec.atomics[fst_ns].size() > 0) {

                    const size_t snd_ns = ns[1];
                    if (ec.atomics[snd_ns].size() > 0) {


                        const bool same_namespace = ( !all.permutations && ( fst_ns == snd_ns ) );
                        const feature* fst     = ec.atomics[fst_ns].begin;
                        const feature* fst_end = ec.atomics[fst_ns].end;
                        const feature* snd_end = ec.atomics[snd_ns].end;

                        for (; fst != fst_end; ++fst)
                        {
                            const uint32_t halfhash = FNV_prime * ((fst->weight_index + offset) >> stride_shift);
                            const feature* snd = (!same_namespace) ? ec.atomics[snd_ns].begin :
                                                                     (fst->x != 1. && feature_self_interactions_for_weight_other_than_1) ? fst : fst+1;

//                            ec.num_features += snd_end - snd;

                            for (; snd < snd_end; ++snd)
                            {

                                const float ft_weight = fst->x*snd->x;
                                const size_t ft_idx = (snd->weight_index ^ halfhash) << stride_shift;
                                call_T<R, T> (dat, weight_vector, weight_mask, ft_weight, ft_idx);

//                                sum_feat_sq += ft_weight * ft_weight;

                            } // end for snd
                        } // end for fst

                    } // end if atomics[snd] size > 0
                } // end if atomics[fst] size > 0

            } else

                if (len == 3) // special case for triples
                {
                    const size_t fst_ns = ns[0];
                    if (ec.atomics[fst_ns].size() > 0) {

                        const size_t snd_ns = ns[1];
                        if (ec.atomics[snd_ns].size() > 0) {

                            const size_t thr_ns = ns[2];
                            if (ec.atomics[thr_ns].size() > 0) {


                                // don't compare 1 and 3 as string is sorted
                                const bool same_namespace1 = ( !all.permutations && ( fst_ns == snd_ns ) );
                                const bool same_namespace2 = ( !all.permutations && ( snd_ns == thr_ns ) );

                                /* // make sure ns in form <a,b,c>, <a,a,b> or <a,a,a>
                                if (same_namespace2 && !same_namespace1)
                                { // <a,b,a> -> <a,a,b>
                                  thr_ns = fst_ns;
                                  fst_ns = snd_ns;
                                  same_namespace2 = false;
                                  same_namespace1 = true;
                                }

                                const size_t margin = (same_namespace1 && same_namespace2) ? 2 :
                                                            (same_namespace1 || same_namespace2) ? 1 : 0;
                                if (fst + margin >= fst_end) continue; */


                                const feature* fst = ec.atomics[fst_ns].begin;
                                const feature* fst_end = ec.atomics[fst_ns].end;
                                const feature* snd_end = (same_namespace1) ? fst_end : ec.atomics[snd_ns].end;
                                const feature* thr_end = (same_namespace2) ? snd_end : ec.atomics[thr_ns].end;

                                for (; fst < fst_end; ++fst)
                                {

                                    const feature* snd = (!same_namespace1) ? ec.atomics[snd_ns].begin :
                                                                              (fst->x != 1. && feature_self_interactions_for_weight_other_than_1) ? fst : fst+1;
                                    const uint32_t halfhash1 = FNV_prime * ((fst->weight_index + offset) >> stride_shift);

                                    for (; snd < snd_end; ++snd)
                                    { //f3 x k*(f2 x k*f1)
                                        const uint32_t halfhash2 = FNV_prime * (halfhash1 ^ ((snd->weight_index + offset) >> stride_shift));
                                        const float ft_weight1 = fst->x * snd->x;

                                        const feature* thr = (!same_namespace2) ? ec.atomics[thr_ns].begin :
                                                                                  (snd->x != 1. && feature_self_interactions_for_weight_other_than_1) ? snd : snd+1;

//                                        ec.num_features += thr_end - thr;

                                        for (; thr < thr_end; ++thr)
                                        {

                                            const float ft_weight2 = ft_weight1 * thr->x;

                                            const size_t ft_idx = (thr->weight_index ^ halfhash2) << stride_shift;
                                            call_T<R, T> (dat, weight_vector, weight_mask, ft_weight2, ft_idx);

//                                            sum_feat_sq += ft_weight2 * ft_weight2;

                                        } // end for thr
                                    } // end for snd
                                } // end for fst

                            } // end if atomics[thr] size > 0
                        } // end if atomics[snd] size > 0
                    } // end if atomics[fst] size > 0

                } else // generic case: quatriples, etc.

#endif
                {

                    // preparing state data

                    bool no_data_to_interact = false; // if any namespace has 0 features - whole interaction is skipped

                    for (size_t i = 0; i < len; ++i)
                    {
                        size_t ft_cnt = ec.atomics[(int32_t)ns[i]].size();
                        if (ft_cnt == 0)
                        {
                            no_data_to_interact = true;
                            break;
                        }

                        if (state_data.size() <= i)
                            state_data.push_back(empty_ns_data);
                        state_data[i].loop_end = ft_cnt-1; // saving number of features in each namespace
                    }

                    if (no_data_to_interact) continue;



                    if (!all.permutations)
                    {
                        // if permutations mode disabeled then namespaces in ns are already sorted and thus grouped
                        // let's go throw the list and calculate number of features to skip in namespaces which
                        // repeated more than once to ensure generation of only simple combinations

                        size_t margin = 0;  // number of features to ignore if namesapce has been seen before


                        bool impossible_without_permutations = false;

                        // iterate list backward as margin grows in this order
                        for (int i = len-1; i >= 1; --i)
                        {
                            if (ns[i] == ns[i-1])
                            {
                                size_t& loop_end = state_data[i-1].loop_end;

                                if (ec.atomics[(int32_t)ns[i-1]][loop_end-margin].x == 1. || // if special case at end of array then we can't exclude more than existing margin
                                        !feature_self_interactions_for_weight_other_than_1)  // and we have to
                                {
                                    ++margin; // otherwise margin can 't be increased

                                    if ( (impossible_without_permutations = (loop_end < margin)) ) break;
                                }
                                loop_end -= margin;               // skip some features and increase margin

                                state_data[i].same_ns = true;     // mark namespace as appearing more than once
                            } else
                                margin = 0;
                        }

                        // if impossible_without_permutations is true then we faced with case like interaction 'aaaa'
                        // where namespace 'a' contains less than 4 unique features. It's impossible to make simple
                        // combination of length 4 without repetitions from 3 or less elements.
                        if (impossible_without_permutations) continue;
                    }


                    size_t cur_ns_idx = 0;      // idx of current namespace
                    state_data[0].loop_idx = 0; // loop_idx contains current feature id for curently processed namespace.

                    const int32_t last_ns = (int32_t)ns[len-1];

                    // beware: micro-optimization. This block taken out from the while loop. Seems to be useful.
                    feature* features_begin = ec.atomics[last_ns].begin; // really constant in all cases. Left non constant to avoid type conversion
                    feature_gen_data* data = &state_data[len-1];
                    /* start & end are always point to features in last namespace of interaction.
                    for 'all.permutations == true' they are constant, so new_ft_cnt is constant too.*/
                    feature* start = features_begin;
                    feature* end = features_begin + data->loop_end + 1; // end is constant as data->loop_end is never chabges in the loop
//                    size_t new_ft_cnt = (size_t)(end-start);

                    uint include_itself = 0;
                    feature_gen_data* next_data;
                    feature* cur_feature;
                    // end of micro-optimization block

                    bool do_it = true;

                    while (do_it)
                    {
                        data = &state_data[cur_ns_idx];

                        if (cur_ns_idx < len-1) // can go further throw the list of namespaces
                        {
                            next_data = &state_data[cur_ns_idx+1];
                            cur_feature = &ec.atomics[(int32_t)ns[cur_ns_idx]][data->loop_idx];

                            if (next_data->same_ns)
                            {
                                // if next namespace is same, we should start with loop_idx + 1 to avoid feature interaction with itself
                                // unless feature has weight w and w != w*w. E.g. w != 0 and w != 1. Features with w == 0 are already
                                // filtered out in parce_args.cc::maybeFeature().

                                include_itself = ((cur_feature->x != 1.) && feature_self_interactions_for_weight_other_than_1) ? 0 : 1;
                                next_data->loop_idx = data->loop_idx + include_itself;
                            }
                            else
                                next_data->loop_idx = 0;


                            if (cur_ns_idx == 0)
                            {
                                next_data->hash = (cur_feature->weight_index  + offset) >> stride_shift;
                                next_data->x = cur_feature->x; // data->x == 1.
                            }
                            else
                            {
                                // feature2 xor (16777619*feature1)
                                next_data->hash = (FNV_prime * data->hash) ^ (( (cur_feature->weight_index  + offset ) >> stride_shift ) ); // not a real index yet as [<< stride_shift] wasn't done for performance reasons
                                next_data->x = cur_feature->x * data->x;
                            }

                            ++cur_ns_idx;

                        } else {

                            // last namespace - iterate its features and go back

                            if (!all.permutations) // values are not constant in this case
                            {
                                start = features_begin + data->loop_idx;
//                                new_ft_cnt = (size_t)(end-start);
                            }

//                             add new features to cache

                            float ft_weight; // micro-optimization. useless, i think

                            for (feature* f = start; f != end; ++f)
                            {
                                ft_weight = data->x * f->x;

                                const size_t ft_idx = ( (FNV_prime * data->hash) ^ (f->weight_index >> stride_shift) ) << stride_shift;
                                call_T<R, T> (dat, weight_vector, weight_mask, ft_weight, ft_idx);

//                                if (ft_weight != 1.) // another useless micro-optimization, i suppose.
//                                    sum_feat_sq += ft_weight * ft_weight;
//                                else ++sum_feat_sq;
                            }

//                            ec.num_features += new_ft_cnt;

                            // trying to go back increasing loop_idx of each namespace by the way

                            bool go_further = true;
                            feature_gen_data* cur_data; // beware: useless micro-optimization

                            do {
                                cur_data = &state_data[--cur_ns_idx];
                                go_further = (++cur_data->loop_idx > cur_data->loop_end);
                            } while (go_further && cur_ns_idx != 0);

                            do_it = !(cur_ns_idx == 0 && go_further);
                            //if false - we've reached 0 namespace but its 'cur_data.loop_idx > cur_data.loop_end' -> exit the while loop
                        }

                    } // while do_it

                }

        } // for all.interactions

//        ec.total_sum_feat_sq += sum_feat_sq;
        state_data.delete_v(); // moving state_data to shared emory seems to make things worse. So freeng memory every time.

//        // Some VW modes are sensitive to number of indexes. For ex. LabelDict::ec_is_label_definition()
//        // Thus can't add empty index as indiator. Or generate_interactions() shouldn't be called in such modes.
//        if (!cache.empty())
//            ec.indices.push_back(interactions_namespace);

//    } // if cache

}


// some helper functions

// expand namespace interactions if contain wildcards
const unsigned char printable_start = '!';
const unsigned char printable_end   = '~';
const uint valid_ns_size = printable_end - printable_start - 1; // will skip two characters

void expand_namespace_depth(const string& ns, vector<string>& res, string val,  size_t pos);
inline void expand_namespace(const string& ns, vector<string>& res)
{
    string temp;
    expand_namespace_depth(ns, res, temp, 0);
}


// exand all wildcard namespaces in vector<string>
// req_length must be 0 if interactions of any length are allowed, otherwise contains required length
// err_msg will be printed plus exception will be thrown if req_length != 0 and mismatch interaction length.
vector<string> expand_interactions(const vector<string> &vec, const size_t req_length, const string err_msg);

// remove duplicate interactions from vector<string>
size_t filter_duplicate_interactions(vector<string>& vec);
}


