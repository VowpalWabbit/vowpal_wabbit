#include "example_predict.h"
#include "vw_predict.h"

#include <iostream>
#include <fstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/bind.hpp>

enum Features
{
  // Shared
  Modality_Audio = 0,
  Modality_Video = 1,
  CallType_P2P = 2,
  CallType_Server = 3,
  NetworkType_Wired = 4,
  NetworkType_Wifi = 5,
  // ADF
  Action1 = 6,
  Action2 = 7,
  Action3 = 8
};

enum Namespaces
{
  SharedA = 0,
  SharedB = 10,
  SharedC = 20,
  ActionDependentX = 30,
  ActionDependentY = 40,
};

struct example_features_push_back_impl
{
  struct result
  {
    typedef void type;
  };

  void operator()(safe_example_predict& c, namespace_index const& ns, feature_index const& idx, feature_value const& x) const
  {
    c.feature_space[ns].push_back(x, idx);
  }
};

boost::phoenix::function<example_features_push_back_impl> const example_features_push_back = example_features_push_back_impl();

template <typename Iterator>
bool parse_complex(Iterator first, Iterator last)
{
  using boost::spirit::ascii::space;

  namespace phx = boost::phoenix;
  namespace qi = boost::spirit::qi;

  double label = 0.0;
  namespace_index current_ns;
  safe_example_predict ex;

  bool r = qi::parse(first, last,
    (
      qi::double_[phx::ref(label) = qi::_1]
      // namespaces
      >> +( +space
        >> '|' >> qi::int_[phx::push_back(phx::ref(ex.indices), qi::_1), phx::ref(current_ns) = qi::_1]
        // features
        >> +( +space
          >> (qi::ulong_long >> ':' >> qi::double_)[example_features_push_back(phx::ref(ex), phx::ref(current_ns), qi::_1, qi::_2)]
          )
        )
      )
    );

  std::cout << "label: " << label << std::endl;
  for (auto& ns : ex.indices)
  {
    std::cout << "ns: " << (uint64_t)ns << std::endl;
    for (auto& fi : ex.feature_space[ns])
      std::cout << "\tidx: " << fi.index() << " " << fi.value() << std::endl;
  }

  if (!r || first != last) // fail if we did not get a full match
    return false;
  //c = std::complex<double>(rN, iN);
  return r;
}

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    std::cout << "usage: " << argv[0] << " <model> <dataset> <prediction-output>" << std::endl;
    return -1;
  }

  // setup shared context
  safe_example_predict shared;

  shared.indices.push_back(SharedA);
  shared.indices.push_back(SharedB);
  shared.indices.push_back(SharedC);

  // 1-hot encoded feature
  shared.feature_space[SharedA].push_back(1, Features::Modality_Audio);
  shared.feature_space[SharedB].push_back(1, Features::CallType_P2P);
  shared.feature_space[SharedC].push_back(1, Features::NetworkType_Wired);

  // setup actions
  safe_example_predict action1, action2, action3;
  action1.indices.push_back(Namespaces::ActionDependentX);
  action2.indices.push_back(Namespaces::ActionDependentX);
  action3.indices.push_back(Namespaces::ActionDependentY);

  action1.feature_space[Namespaces::ActionDependentX].push_back(1, Features::Action1);
  action1.feature_space[Namespaces::ActionDependentX].push_back(1, Features::Action2);
  action1.feature_space[Namespaces::ActionDependentX].push_back(1, Features::Action3);

  // read from file
  std::ifstream ifs(argv[1], std::ios::binary | std::ios::ate);
  std::ifstream::pos_type model_size = ifs.tellg();

  std::vector<char> model_file(model_size);

  ifs.seekg(0, std::ios::beg);
  ifs.read(&model_file[0], model_size);

  vw_predict predictor(&model_file[0], model_size);

  std::string str = "1 |0 25:1 28:4.2 |5 26:1 27:3.4";
  parse_complex(str.begin(), str.end());

   // TODO: load model
  // TODO: slim parser
  // TODO: score
  /*
  inline_predict<dense_parameters>
    inline float inline_predict(W& weights, bool ignore_some_linear, bool ignore_linear[256], v_array<v_string>& interactions,
      bool permutations, example_predict& ec, float initial = 0.f)
      */

  return 0;
}
