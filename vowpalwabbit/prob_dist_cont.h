#pragma once
namespace VW { namespace actions_pdf
{
struct action_pdf_value
{
  float action;      //continuous action
  float pdf_value;   //probability density value
};

struct pdf_segment
{
  float begin;
  float end;
  float pdf_value;
};

typedef v_array<pdf_segment> pdf;

// TODO: below check

void print_prob_dist(int f, v_array<pdf_segment>& a_s, v_array<char>&);

void delete_prob_dist(void* v);

float get_pdf_value(pdf& prob_dist, float chosen_action);

//TODO: do we need below?

class prob_iterator : public virtual std::iterator<std::random_access_iterator_tag,  // iterator_cateogry
                           float,                                                     // value_type
                           long,                                                      // difference_type
                           float*,                                                    // pointer
                           float                                                      // reference
                           >
{
  pdf_segment* _p;

 public:
  prob_iterator(pdf_segment* p) : _p(p) {}

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

  float& operator*() { return _p->pdf_value; }
};

inline prob_iterator begin_probs(pdf& p_d) { return prob_iterator(p_d.begin()); }

inline prob_iterator end_probs(pdf& p_d) { return prob_iterator(p_d.end()); }

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
  pdf_segment* s1 = (pdf_segment*)p1;
  pdf_segment* s2 = (pdf_segment*)p2;
  // Most sorting algos do not guarantee the output order of elements that compare equal.
  // Tie-breaking on the index ensures that the result is deterministic across platforms.
  // However, this forces a strict ordering, rather than a weak ordering, which carries a performance cost.
  if (s2->pdf_value == s1->pdf_value)
    return cmp(s1->pdf_value, s2->pdf_value);
  else if (s2->pdf_value >= s1->pdf_value)
    return -1;
  else
    return 1;
}

inline int reverse_order(const void* p1, const void* p2) { return prob_comp(p2, p1); }

std::string to_string(const action_pdf_value& seg, bool print_newline = false);
std::string to_string(const v_array<pdf_segment>& pdf, bool print_newline = false);
}}  // namespace VW::actions
