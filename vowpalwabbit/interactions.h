#pragma once

#include "global_data.h"
#include "constant.h"

namespace INTERACTIONS
{

// By default include interactions of feature with itself when its weight != weight^2.
const bool feature_self_interactions_for_weight_other_than_1 = true;

// Default interaction hash constants
// It's a variation of Bernstein hash http://www.eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
// Suggested by Josh Bloch in Effective Java http://stackoverflow.com/a/1646913/841424

const int bernstein_c1 = 31;
const int bernstein_c0 = 17 * bernstein_c1; // 17*31

// All hashes could be represented in form f(x,a,b) = a*x + b
// for pairs it's f(hash1 + offset, quadratic_constant, hash2)
// for triples it's f( f(hash1 + offset, cubic_constant, hash2) + offset, cubic_constant2, hash3)
// for bernshtain it's f(bernstein_c1, hash1 + offset + bernstein_c0, hash2)

inline uint32_t interact(const uint32_t hash1, const uint32_t hash2, const int coef, const uint32_t offset)
{
    return coef*(hash1+offset) + hash2;
}


// state data for non-recursive iteration algorithm
// consists on N feature_gen_data records
// where N is length of interaction

struct feature_gen_data
{
    size_t loop_idx; // current feature id in namespace
    size_t hash;     // hash of feature interactions of previous namespaces in the list
    double x;        // weight of feature interactions of previous namespaces in the list
    size_t loop_end; // last feature id. May be less than number of features if namespace involved in interaction more than once
                     // calculated at preprocessing together with same_ns
    size_t same_ns;  // true if same namespace as previous is interaction list
    feature_gen_data(): loop_idx(0), hash(1), x(1.), loop_end(0), same_ns(false) {}
};


// checks if allready allocated memory is enough to store cnt items. Resize if not enough.
template<class T>
inline void ensure_could_push(v_array<T>& v, size_t cnt)
{
    if (v.end + cnt > v.end_array)
        v.resize(2*(v.size()+cnt)+3);
}


void generate_interactions(vw& all, example& ec);

// some helper functions

// expand namespace interactions if contain wildcards
const unsigned char printable_start = '!';
const unsigned char printable_end   = '~';
const uint valid_ns_size = printable_end - printable_start - 1; //will skip two characters

void expand_namespace_depth(string& ns, vector<string>& res, string val,  size_t pos);
inline void expand_namespace(string ns, vector<string>& res)
{
    string temp;
    expand_namespace_depth(ns, res, temp, 0);
}

// remove duplicate interactions from vector<string>
void filter_duplicate_interactions(vector<string>& vec, bool leave_elements_sorted = false);

}

