#include "v_array.h"
#include "action_score.h"
#include "io_buf.h"
#include "global_data.h"
using namespace std;
namespace ACTION_SCORE
{
void print_action_score(io_adapter* f, v_array<action_score>& a_s, v_array<char>& tag)
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
    print_tag(ss, tag);
    ss << '\n';
    ssize_t len = ss.str().size();
    ssize_t t = f->write(ss.str().c_str(), (unsigned int)len);
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
