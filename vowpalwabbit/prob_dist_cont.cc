#include "vw.h"
#include "v_array.h"
#include "prob_dist_cont.h"
#include "io_buf.h"

using namespace std;
// TODO: below check
namespace VW { namespace actions_pdf
{
  // Convert pdf to string of form 'begin-end:pdf_value, ... '
  std::string to_string(const v_array<pdf_segment>& p_d, bool newline)
  {
    std::stringstream ss;
    for (size_t i = 0; i < p_d.size(); i++)
    {
      if (i > 0)
        ss << ',';
      ss << p_d[i].begin << '-' << p_d[i].end << ':' << p_d[i].pdf_value;
    }

    if (newline) ss << endl;

    return ss.str();
  }

  // Print out to_string(pdf) to given file descriptor
  void print_prob_dist(int f, v_array<pdf_segment>& p_d, v_array<char>&)
  {
    if (f >= 0)
    {
      std::stringstream ss;
      ss << to_string(p_d);
      ss << '\n';
      const ssize_t len = ss.str().size();
      const ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
      if (t != len)
        cerr << "write error: " << strerror(errno) << endl;
    }
  }

  void delete_prob_dist(void* v)
  {
    v_array<pdf_segment>* cs = (v_array<pdf_segment>*)v;
    cs->delete_v();
  }

  float get_pdf_value(VW::actions_pdf::pdf& prob_dist, float chosen_action)
  {
    int begin = -1;
    int end = (int)prob_dist.size();
    while (end - begin > 1)
    {
      int mid = (begin + end) / 2;
      if (prob_dist[mid].end <= chosen_action)
      {
        begin = mid;
      }
      else
      {
        end = mid;
      }
    }

    // temporary fix for now
    if (begin == (int)prob_dist.size() - 1 && prob_dist[begin].pdf_value == 0)
      return prob_dist[begin - 1].pdf_value;
    return prob_dist[begin].pdf_value;
  }

  std::string to_string(const action_pdf_value& a_p, bool newline)
  {
    std::stringstream strm;
    strm << a_p.action << "," << a_p.pdf_value;
    if (newline)
      strm << endl;
    return strm.str();
  }
}} // namespace vw::pdf


