#include "vw.h"
#include "v_array.h"
#include "prob_dist_cont.h"
#include "io_buf.h"

using namespace std;
// TODO: below check
namespace VW { namespace actions_pdf
{
void print_prob_dist(int f, v_array<pdf_segment>& p_d, v_array<char>&)
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
    if (prob_dist[mid].action <= chosen_action)
    {
      begin = mid;
    }
    else
    {
      end = mid;
    }
  }
  return prob_dist[begin].value;
}

std::string to_string(const pdf_segment& seg)
{
  std::stringstream strm;
  strm << "{" << seg.action << "," << seg.value << "}";
  return strm.str();
}
}} // namespace vw::pdf


