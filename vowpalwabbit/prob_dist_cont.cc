#include "vw.h"
#include "v_array.h"
#include "prob_dist_cont.h"
#include "io_buf.h"

using namespace std;
// TODO: below check
namespace VW { namespace actions_pdf
{
  void delete_prob_dist(void* v)
  {
    v_array<pdf_segment>* cs = (v_array<pdf_segment>*)v;
    cs->delete_v();
  }

  std::string to_string(const action_pdf_value& a_p, bool newline)
  {
    std::stringstream strm;
    strm << a_p.action << "," << a_p.pdf_value;
    if (newline)
      strm << endl;
    return strm.str();
  }

  std::string to_string(const pdf_segment& seg)
  {
    std::stringstream strm;
    strm << "{" << seg.left << "-" << seg.right
         << "," << seg.pdf_value << "}";
    return strm.str();
  }

  // Convert pdf to string of form 'begin-end:pdf_value, ... '
  std::string to_string(const v_array<pdf_segment>& p_d, bool newline)
  {
    std::stringstream ss;
    for (size_t i = 0; i < p_d.size(); i++)
    {
      if (i > 0)
        ss << ',';
      ss << p_d[i].left << '-' << p_d[i].right << ':' << p_d[i].pdf_value;
    }

    if (newline)
      ss << endl;

    return ss.str();
  }

  }  // namespace actions_pdf
  } // namespace vw::pdf


