namespace ACTION_SCORE
{

  struct action_score
  { float score;
    uint32_t action;
  };

  typedef v_array<action_score> action_scores;

  void print_action_score(int f, v_array<action_score>& a_s, v_array<char>&);

  void delete_action_scores(void* v);
}
