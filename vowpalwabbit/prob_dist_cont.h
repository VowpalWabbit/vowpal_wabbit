#pragma once
namespace VW { namespace pdf
{
struct prob_dist
{
  float action;  //starting point
  float value; //height
};

typedef v_array<prob_dist> prob_dists;

// TODO: below check

void print_prob_dist(int f, v_array<prob_dist>& a_s, v_array<char>&);

void delete_prob_dist(void* v);

//TODO: do we need below?

class prob_iterator : public virtual std::iterator<std::random_access_iterator_tag,  // iterator_cateogry
                           float,                                                     // value_type
                           long,                                                      // difference_type
                           float*,                                                    // pointer
                           float                                                      // reference
                           >
{
  prob_dist* _p;

 public:
  prob_iterator(prob_dist* p) : _p(p) {}

  prob_iterator& operator++()
  {
    ++_p;
    return *this;
  }

  prob_iterator operator+(size_t n) { return prob_iterator(_p + n); }

  bool operator==(const prob_iterator& other) const { return _p == other._p; }

  bool operator!=(const prob_iterator& other) const { return _p != other._p; }

  bool operator<(const prob_iterator& other) const { return _p < other._p; }

  size_t operator-(const prob_iterator& other) const { return _p - other._p; }

  float& operator*() { return _p->value; }
};

inline prob_iterator begin_probs(prob_dists& p_d) { return prob_iterator(p_d.begin()); }

inline prob_iterator end_probs(prob_dists& p_d) { return prob_iterator(p_d.end()); }

inline int cmp(float a, float b)
{
  if (a == b)
    return 0;
  if (a > b)
    return 1;
  return -1;
}

inline int prob_comp(const void* p1, const void* p2)
{
  prob_dist* s1 = (prob_dist*)p1;
  prob_dist* s2 = (prob_dist*)p2;
  // Most sorting algos do not guarantee the output order of elements that compare equal.
  // Tie-breaking on the index ensures that the result is deterministic across platforms.
  // However, this forces a strict ordering, rather than a weak ordering, which carries a performance cost.
  if (s2->value == s1->value)
    return cmp(s1->action, s2->action);
  else if (s2->value >= s1->value)
    return -1;
  else
    return 1;
}

inline int reverse_order(const void* p1, const void* p2) { return prob_comp(p2, p1); }


}}  // namespace vw::pdf
