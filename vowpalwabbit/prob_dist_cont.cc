#include <float.h>
#include <vector>

#include "reductions.h"
#include "cb_algs.h"
#include "rand48.h"
#include "bs.h"
#include "vw.h"
#include "hash.h"
#include "explore.h"
#include "v_array.h"
#include "prob_dist_cont.h"
#include "io_buf.h"

using namespace std;
// TODO: below check
namespace PDF
{
void print_prob_dist(int f, v_array<prob_dist>& p_d, v_array<char>&)
{
  if (f >= 0)
  {
    std::stringstream ss;

    for (size_t i = 0; i < p_d.size(); i++)
    {
      if (i > 0)
        ss << ',';
      ss << p_d[i].action << ':' << p_d[i].value;
    }
    ss << '\n';
    ssize_t len = ss.str().size();
    ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
    if (t != len)
      cerr << "write error: " << strerror(errno) << endl;
  }
}

void delete_prob_dist(void* v)
{
  v_array<prob_dist>* cs = (v_array<prob_dist>*)v;
  cs->delete_v();
}

} // namespase PDF


