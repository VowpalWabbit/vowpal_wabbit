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


void generate_interactions(vw& all, example& ec);

// some helper functions

// expand namespace interactions if contain wildcards
const unsigned char printable_start = '!';
const unsigned char printable_end   = '~';
const uint valid_ns_size = printable_end - printable_start - 1; // will skip two characters

void expand_namespace_depth(string& ns, vector<string>& res, string val,  size_t pos);
inline void expand_namespace(string ns, vector<string>& res)
{
    string temp;
    expand_namespace_depth(ns, res, temp, 0);
}

// remove duplicate interactions from vector<string>
void filter_duplicate_interactions(vector<string>& vec, bool leave_elements_sorted = false);

}

