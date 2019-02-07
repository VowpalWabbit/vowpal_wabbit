#include "v_array.h"
#include "action_score.h"
#include "io_buf.h"
using namespace std;
namespace ACTION_SCORE
{
void print_action_score(int f, v_array<action_score>& a_s, v_array<char>&)
{
  if (f >= 0)
  {
    std::stringstream ss;

    for (size_t i = 0; i < a_s.size(); i++)
    {
      if (i > 0)
        ss << ',';
      ss << a_s[i].action << ':' << a_s[i].score;
    }
    ss << '\n';
    ssize_t len = ss.str().size();
    ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
    if (t != len)
      cerr << "write error: " << strerror(errno) << endl;
  }
}

void delete_action_scores(void* v)
{
  v_array<action_score>* cs = (v_array<action_score>*)v;
  cs->delete_v();
}

}  // namespace ACTION_SCORE
