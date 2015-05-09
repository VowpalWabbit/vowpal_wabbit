#pragma once

#include "global_data.h"
#include "constant.h"

/*
 *  Interactions preprocessing and feature combinations generation
*/

namespace INTERACTIONS
{


/*
 *  Interactions preprocessing
 */


const unsigned char printable_start = '!';
const unsigned char printable_end   = '~';
const uint valid_ns_size = printable_end - printable_start - 1; // -1 to skip characters ':' and '|' excluded in is_valid_ns()

// exand all wildcard namespaces in vector<string>
// req_length must be 0 if interactions of any length are allowed, otherwise contains required length
// err_msg will be printed plus exception will be thrown if req_length != 0 and mismatch interaction length.
v_array<v_string> expand_interactions(const vector<string> &vec, const size_t required_length, const string &err_msg);

// remove duplicate interactions and sort namespaces in them (if required)
void sort_and_filter_duplicate_interactions(v_array<v_string> &vec, bool filter_duplicates, size_t &removed_cnt, size_t &sorted_cnt);


/*
 *  Feature combinations generation
 */


/*
* By default include interactions of feature with itself if its weight != weight^2.
* For ex. interaction 'aa' with 3 features in namespace a: {1,2,3} generates
* simple combinations of features {12,13,23}. But if weight of feature 2 != 1.0 then
* features {12, 13, 22, 23} will be generated. For 'aaa' it will be {123, 222, 223}
*/
const bool feature_self_interactions_for_weight_other_than_1 = true;



// FNV-like hash constant for 32bit
// http://www.isthe.com/chongo/tech/comp/fnv/#FNV-param
const uint32_t FNV_prime = 16777619;



// function estimates how many new features will be generated for example and ther sum(weight^2).
void eval_count_of_generated_ft(vw& all, example& ec, size_t& new_features_cnt, float& new_features_weight);



// 2 template functions to pass T() proper argument (feature idx in regressor, or its coefficient)

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



// state data used in non-recursive feature generation algorithm
// contains N feature_gen_data records (where N is length of interaction)

struct feature_gen_data
{
    size_t loop_idx;          // current feature id in namespace
    uint32_t hash;            // hash of feature interactions of previous namespaces in the list
    double x;                 // weight of feature interactions of previous namespaces in the list
    size_t loop_end;          // last feature id. May be less than number of features if namespace involved in interaction more than once
                              // calculated at preprocessing together with same_ns
    size_t self_interaction;  // namespace interacting with itself    
    v_array<feature>* ft_arr;
//    feature_gen_data(): loop_idx(0), x(1.), loop_end(0), self_interaction(false) {}
};


// uncomment line below to disable usage of inner 'for' loops for pair and triple interactions
// end switch to usage of non-recursive feature generation algorithm for interactions of any length

// #define GEN_INTER_LOOP

// this templated function generates new features for given example and set of interactions
// and passes each of them to given function T()
// it must be in header file to avoid compilation problems

template <class R, class S, void (*T)(R&, float, S)>
inline void generate_interactions(vw& all, example& ec, R& dat)
{
    // often used values
    const uint32_t offset = ec.ft_offset;
//    const uint32_t stride_shift = all.reg.stride_shift; // it seems we don't need stride shift in FTRL-like hash
    weight* weight_vector = all.reg.weight_vector;
    const size_t  weight_mask   = all.reg.weight_mask;

    // statedata for generic non-recursive iteration
    v_array<feature_gen_data> state_data = v_init<feature_gen_data>();

    feature_gen_data empty_ns_data;  // micro-optimization. don't want to call its constructor each time in loop.
    empty_ns_data.loop_idx = 0;
    empty_ns_data.x = 1.;
    empty_ns_data.loop_end = 0;
    empty_ns_data.self_interaction = false;

    // loop throw the set of possible interactions
    for (v_string* it = all.interactions.begin; it != all.interactions.end; ++it)
    {
        v_string& ns = (*it);         // current list of namespaces to interact.

#ifndef GEN_INTER_LOOP

        // unless GEN_INTER_LOOP is defined we use nested 'for' loops for interactions length 2 (pairs) and 3 (triples)
        // and generic non-recursive algorythm for all other cases.
        // nested 'for' loops approach is faster, but can't be used for interation of any length.

        const size_t len = ns.size();

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
                        const uint32_t halfhash = FNV_prime * fst->weight_index;
                        // next index differs for permutations and simple combinations
                        const feature* snd = (!same_namespace) ? ec.atomics[snd_ns].begin :
                                                                 (fst->x != 1. && feature_self_interactions_for_weight_other_than_1) ? fst : fst+1;
                        const float& ft_weight = fst->x;
                        for (; snd < snd_end; ++snd)
                        {                            
                            //  const size_t ft_idx = ((snd->weight_index /*>> stride_shift*/) ^ halfhash) /*<< stride_shift*/;
                            call_T<R, T> (dat, weight_vector, weight_mask, ft_weight*snd->x, (snd->weight_index^halfhash) + offset);
                        } // end for(snd)
                    } // end for(fst)

                } // end if (atomics[snd] size > 0)
            } // end if (atomics[fst] size > 0)

        } else

            if (len == 3) // special case for triples
            {
                const size_t fst_ns = ns[0];
                if (ec.atomics[fst_ns].size() > 0) {

                    const size_t snd_ns = ns[1];
                    if (ec.atomics[snd_ns].size() > 0) {

                        const size_t thr_ns = ns[2];
                        if (ec.atomics[thr_ns].size() > 0) {


                            // don't compare 1 and 3 as interaction is sorted
                            const bool same_namespace1 = ( !all.permutations && ( fst_ns == snd_ns ) );
                            const bool same_namespace2 = ( !all.permutations && ( snd_ns == thr_ns ) );

                            const feature* fst = ec.atomics[fst_ns].begin;
                            const feature* fst_end = ec.atomics[fst_ns].end;
                            const feature* snd_end = (same_namespace1) ? fst_end : ec.atomics[snd_ns].end;
                            const feature* thr_end = (same_namespace2) ? snd_end : ec.atomics[thr_ns].end;

                            for (; fst < fst_end; ++fst)
                            {
                                // next index differs for permutations and simple combinations
                                const feature* snd = (!same_namespace1) ? ec.atomics[snd_ns].begin :
                                                                          (fst->x != 1. && feature_self_interactions_for_weight_other_than_1) ? fst : fst+1;

                                const uint32_t halfhash1 = FNV_prime * fst->weight_index;
                                const float& ft_weight = fst->x;

                                for (; snd < snd_end; ++snd)
                                {
                                    //f3 x k*(f2 x k*f1)
                                    const uint32_t halfhash2 = FNV_prime * (halfhash1 ^ snd->weight_index);
                                    const float ft_weight1 =  ft_weight * snd->x;

                                    // next index differs for permutations and simple combinations
                                    const feature* thr = (!same_namespace2) ? ec.atomics[thr_ns].begin :
                                                                              (snd->x != 1. && feature_self_interactions_for_weight_other_than_1) ? snd : snd+1;

                                    for (; thr < thr_end; ++thr)
                                    {
//                                        const size_t ft_idx = ((thr->weight_index /*>> stride_shift*/)^ halfhash2) /*<< stride_shift*/;
                                        call_T<R, T> (dat, weight_vector, weight_mask, ft_weight1 * thr->x, (thr->weight_index^halfhash2) + offset);
                                    } // end for (thr)
                                } // end for (snd)
                            } // end for (fst)

                        } // end if (atomics[thr] size > 0)
                    } // end if (atomics[snd] size > 0)
                } // end if (atomics[fst] size > 0)

            } else // generic case: quatriples, etc.

#endif
            {

                bool must_skip_interaction = false;

                // preparing state data
                feature_gen_data* fgd = state_data.begin;
                feature_gen_data* fgd2; // for further use
                for (unsigned char* n = ns.begin; n != ns.end; ++n)
                {                    
                    v_array<feature>* ft = &ec.atomics[(int32_t)*n];
                    const size_t ft_cnt = ft->size();

                    if (ft_cnt == 0)
                    {
                        must_skip_interaction = true;
                        break;
                    }

                    if (fgd == state_data.end)
                    {
                        state_data.push_back(empty_ns_data);
                        fgd = state_data.end-1; // reassign as memory could be realloced
                    }

                    fgd->loop_end = ft_cnt-1; // saving number of features for each namespace
                    fgd->ft_arr = ft;
                    ++fgd;
                }

                // if any of interacting namespace has 0 features - whole interaction is skipped
                if (must_skip_interaction) continue; //no_data_to_interact



                if (!all.permutations) // adjust state_data for simple combinations
                {
                    // if permutations mode is disabeled then namespaces in ns are already sorted and thus grouped
                    // (in fact, currently they are sorted even for enabled permutations mode)
                    // let's go throw the list and calculate number of features to skip in namespaces which
                    // repeated more than once to generate only simple combinations of features

                    size_t margin = 0;  // number of features to ignore if namesapce has been seen before                    

                    // iterate list backward as margin grows in this order                    

                    for (fgd = state_data.end-1; fgd > state_data.begin; --fgd)
                    {
                        fgd2 = fgd-1;
                        fgd->self_interaction = (fgd->ft_arr == fgd2->ft_arr); //state_data.begin.self_interaction is always false
                        if (fgd->self_interaction)
                        {                            
                            size_t& loop_end = fgd2->loop_end;

                            if ((*fgd2->ft_arr)[loop_end-margin].x == 1. || // if special case at end of array then we can't exclude more than existing margin
                                    !feature_self_interactions_for_weight_other_than_1)  // and we have to
                            {
                                ++margin; // otherwise margin can 't be increased
                                if ( (must_skip_interaction = (loop_end < margin)) ) break;
                            }

                            if (margin != 0)
                                loop_end -= margin;               // skip some features and increase margin
                        } else
                            if (margin != 0) margin = 0;

                    }

                    // if impossible_without_permutations == true then we faced with case like interaction 'aaaa'
                    // where namespace 'a' contains less than 4 unique features. It's impossible to make simple
                    // combination of length 4 without repetitions from 3 or less elements.
                    if (must_skip_interaction) continue; // impossible_without_permutations
                } // end of state_data adjustment


                fgd = state_data.begin;  // always equal to first ns
                fgd2 = state_data.end-1; // always equal to last ns
                fgd->loop_idx = 0; // loop_idx contains current feature id for curently processed namespace.

                // beware: micro-optimization.

                feature* features_begin = fgd2->ft_arr->begin; // in fact, constant in all cases. Left non constant to avoid type conversion
                /* start & end are always point to features in last namespace of interaction.
                    for 'all.permutations == true' they are constant.*/
                feature* start = features_begin;
                feature* end = features_begin + fgd2->loop_end + 1; // end is constant as data->loop_end is never changed in the loop

                feature_gen_data* data = fgd;
                feature_gen_data* next_data;
                feature* cur_feature;
                // end of micro-optimization block


                // generic feature generation cycle for interactions of any length
                bool do_it = true;

                while (do_it)
                {                    

                    if (data < fgd2) // can go further throw the list of namespaces in interaction
                    {
                        next_data = data+1;
                        cur_feature = data->ft_arr->begin + data->loop_idx;

                        if (next_data->self_interaction)
                        {
                            // if next namespace is same, we should start with loop_idx + 1 to avoid feature interaction with itself
                            // unless feature has weight w and w != w*w. E.g. w != 0 and w != 1. Features with w == 0 are already
                            // filtered out in parce_args.cc::maybeFeature().

                            next_data->loop_idx = ((cur_feature->x != 1.) && feature_self_interactions_for_weight_other_than_1) ? data->loop_idx : data->loop_idx + 1;
                        }
                        else
                            next_data->loop_idx = 0;


                        if (data == fgd)
                        {
                            next_data->hash = FNV_prime * cur_feature->weight_index;
                            next_data->x = cur_feature->x; // data->x == 1.
                        }
                        else
                        {
                            // feature2 xor (16777619*feature1)
                            next_data->hash = FNV_prime * (data->hash ^ cur_feature->weight_index);
                            next_data->x = cur_feature->x * data->x;
                        }

                        ++data;

                    } else {

                        // last namespace - iterate its features and go back

                        if (!all.permutations) // start value is not a constant in this case
                            start = features_begin + fgd2->loop_idx;



                        for (feature* f = start; f != end; ++f)
                        {
                            // const size_t ft_idx = ( data->hash ^ (f->weight_index /*>> stride_shift*/) ) /*<< stride_shift*/;
                            call_T<R, T> (dat, weight_vector, weight_mask, fgd2->x * f->x /*ft_weight*/, (fgd2->hash^f->weight_index) + offset );
                        }

                        // trying to go back increasing loop_idx of each namespace by the way

                        bool go_further = true;

                        do {

//                            feature_gen_data* cur_data = &state_data[--cur_ns_idx];
                            --data;
                            go_further = (++data->loop_idx > data->loop_end);
                        } while (go_further && data != fgd);

                        do_it = !(data == fgd && go_further);
                        //if do_it==false - we've reached 0 namespace but its 'cur_data.loop_idx > cur_data.loop_end' -> exit the while loop

                    } // if last namespace

                } // while do_it

            }

    } // foreach interaction in all.interactions

    state_data.delete_v();
}


} // end of namespace


