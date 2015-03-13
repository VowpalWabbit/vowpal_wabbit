#include "interactions.h"

namespace INTERACTIONS
{

// generates features for all.interations and store them in cache ( ec.atomics[interactions_namespace] )
void generate_interactions(vw& all, example& ec)
{
  if (!all.interactions.empty() && !v_array_contains(ec.indices, (unsigned char)interactions_namespace) )
    {   // if cache wasn't created yet

        v_array<feature>& cache = ec.atomics[interactions_namespace];
        float& sum_feat_sq = ec.sum_feat_sq[interactions_namespace];

        uint32_t offset = ec.ft_offset;

        v_array<feature_gen_data> state_data = v_init<feature_gen_data>(); // statedata for non-recursive iteration
        feature_gen_data empty_ns_data;  // befare: micro-optimization. don't want to call its constructor each time in loop.

        for (vector<string>::const_iterator it = all.interactions.begin(); it != all.interactions.end(); ++it)
        {
            const string ns = (*it);  // current list of namespaces to interact.
            // 'const string::operator[]' seems to be faster (http://stackoverflow.com/a/19920071/841424)

            // preparing state data

            bool no_data_to_interact = false; // if any namespace has 0 features - whole interaction is skipped

            const size_t len = ns.length();
            if (len > state_data.size())
                ensure_could_push(state_data, len);


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
                        if ( (impossible_without_permutations = (loop_end < ++margin)) ) break;
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

            // a small trick to keep backward compatibility with hash functions and good performance

            int coef1; int coef2;

            switch(len)
            {
            case 2: { // pairs
                coef1 = quadratic_constant;
                coef2 = quadratic_constant;
            } break;
            case 3: { // triples
                coef1 = cubic_constant;
                coef2 = cubic_constant2;
            } break;
            default: {// all other
                coef1 = bernstein_c0;
                coef2 = bernstein_c0;
                offset += bernstein_c1 << all.reg.stride_shift;
            } break;
            }

            // as coef2 != coef1 only for triples, its use will be hardcoded for hash calculation for last namespace in interaction


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
            size_t new_ft_cnt = (size_t)(end-start);

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
                    { // small micro-optimization
                        next_data->hash = cur_feature->weight_index;
                        next_data->x = cur_feature->x; // data->x == 1.
                    }
                    else
                    {
                        next_data->hash = interact( data->hash, cur_feature->weight_index, coef1, offset); // always coef1, as we are not in last namespace yet
                        next_data->x = cur_feature->x * data->x;
                    }



                    ++cur_ns_idx;

                } else {

                    // last namespace - iterate its features and go back

                    if (!all.permutations) // values are not constant in this case
                    {
                        start = features_begin + data->loop_idx;
                        new_ft_cnt = (size_t)(end-start);
                    }

                    // add new features to cache

                    ensure_could_push(cache, new_ft_cnt); // it's better than just resize(size()+new_ft_cnt)

                    float ft_weight; // micro-optimization. useless, i think

                    for (feature* f = start; f != end; ++f)
                    {
                        ft_weight = data->x * f->x;

                        cache.push_back(feature {
                                            ft_weight,
                                            interact(data->hash, f->weight_index, coef2, offset)
                                            }); /* coef2 here. If triples then we are in namespace #3.
                                                   In other cases coef1==coef2 anyway, so coef2 is hardcoded here
                                                   and no check of 'len' and 'cur_ns_idx' is done. */

                        if (ft_weight != 1.) // another useless micro-optimization, i suppose.
                            sum_feat_sq += ft_weight * ft_weight;
                        else ++sum_feat_sq;
                    }

                    ec.num_features += new_ft_cnt;

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

        } // for all.interactions

        ec.total_sum_feat_sq += sum_feat_sq;
        state_data.delete_v(); // moving state_data to shared emory seems to make things worse. So freeng memory every time.

        // Some VW modes are sensitive to number of indexes. For ex. LabelDict::ec_is_label_definition()
        // Thus can't add empty index as indiator. Or generate_interactions() shouldn't be called in such modes.
        if (!cache.empty())
            ec.indices.push_back(interactions_namespace);

    } // if cache

}

}
