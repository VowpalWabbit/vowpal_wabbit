#include "vw_util.h" 
#include "hash.h"
#include "constant.h"

#ifdef BOOST_FOUND

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/bind.hpp>

#endif 

using namespace vw_slim;

#ifdef BOOST_FOUND

struct example_features_push_back_impl
{
  struct result { typedef void type; };

  void operator()(safe_example_predict& c, namespace_index const& ns, uint64_t namespace_hash, feature_index const& idx, feature_value const& x) const
  { c.feature_space[ns].push_back(x, namespace_hash + idx);  }
};

boost::phoenix::function<example_features_push_back_impl> const example_features_push_back = example_features_push_back_impl();

struct vw_hash_impl
{
	struct result { typedef uint64_t type; };

	uint64_t operator()(std::vector<char>& s, uint64_t seed = 0) const
	{ return uniform_hash((const void*)&s[0],s.size(), seed); }
};

boost::phoenix::function<vw_hash_impl> const vw_hash = vw_hash_impl();

struct first_char_impl
{
	struct result { typedef char type; };

	char operator()(std::vector<char>& s) const
	{ return s[0]; }
};

boost::phoenix::function<first_char_impl> const first_char = first_char_impl();

template <typename Iterator>
bool try_parse_examples(Iterator first, Iterator last, safe_example_predict& ex)
{
  namespace ascii = boost::spirit::ascii;
  namespace phx = boost::phoenix;
  namespace qi = boost::spirit::qi;

  double label = 0.0;
  namespace_index current_ns;
  uint64_t current_ns_hash;

  bool r = qi::parse(first, last,
    (
      qi::float_[phx::ref(label) = qi::_1]
      // namespaces
      >> +( +ascii::space
        >> '|' >> (
					qi::int_[phx::ref(current_ns_hash) = phx::ref(current_ns) = qi::_1] 
					| (+ascii::graph)[phx::ref(current_ns_hash) = vw_hash(qi::_1), phx::ref(current_ns) = first_char(qi::_1)]
			      )
				  [phx::push_back(phx::ref(ex.indices), phx::ref(current_ns))]
        // features
        >> +( +ascii::space
          >> (qi::ulong_long  >> ':' >> qi::float_)[example_features_push_back(phx::ref(ex), phx::ref(current_ns), phx::ref(current_ns_hash), qi::_1, qi::_2)]
		     //|((+ascii::graph) >> ':' >> qi::float_)[example_features_push_back(phx::ref(ex), phx::ref(current_ns), vw_hash(qi::_1, phx::ref(current_ns_hash)), qi::_2)]
			 //)
          )
        )
      )
    );

  //std::ofstream log("c:\\temp\\skype.txt");
  //log << "label: " << label << std::endl;
  //for (auto& ns : ex.indices)
  //{
	 // log << "ns: " << (uint64_t)ns << std::endl;
  //  for (auto& fi : ex.feature_space[ns])
		//log << "\tidx: " << fi.index() << " " << fi.value() << std::endl;
  //}

  if (!r || first != last) // fail if we did not get a full match
    return false;

  return r;
}

bool try_parse_examples(std::string line, safe_example_predict& ex)
{
  return try_parse_examples(std::begin(line), std::end(line), ex);
}

#endif 