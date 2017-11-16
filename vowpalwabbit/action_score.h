#pragma once
namespace ACTION_SCORE
{

struct action_score
{ uint32_t action;
  float score;
};

typedef v_array<action_score> action_scores;

inline int cmp(size_t a, size_t b)
{ if (a == b) return 0;
  if (a > b) return 1;
  return -1;
}

inline int score_comp(const void* p1, const void* p2)
{ action_score* s1 = (action_score*)p1;
  action_score* s2 = (action_score*)p2;
  // Most sorting algos do not guarantee the output order of elements that compare equal.
  // Tie-breaking on the index ensures that the result is deterministic across platforms.
  // However, this forces a strict ordering, rather than a weak ordering, which carries a performance cost.
  if(s2->score == s1->score) return cmp(s1->action, s2->action);
  else if(s2->score >= s1->score) return -1;
  else return 1;
}

inline int reverse_order(const void* p1, const void* p2)
{ return score_comp(p2,p1);
}

void print_action_score(int f, v_array<action_score>& a_s, v_array<char>&);

void delete_action_scores(void* v);
}
