#include "test_common.h"
#include "parse_example_json.h"

multi_ex parse_json(vw& all, const std::string& line)
{
  auto examples = v_init<example*>();
  examples.push_back(&VW::get_unused_example(&all));
  VW::read_line_json<true>(
      all, examples, (char*)line.c_str(), (VW::example_factory_t)&VW::get_unused_example, (void*)&all);

  multi_ex result;
  for (size_t i = 0; i < examples.size(); ++i)
  {
    result.push_back(examples[i]);
  }
  examples.delete_v();
  return result;
}

bool is_invoked_with(const std::string& arg)
{
  for (size_t i = 0; i < boost::unit_test::framework::master_test_suite().argc; i++)
  {
    if (VW::string_view(boost::unit_test::framework::master_test_suite().argv[i]).find(arg) != std::string::npos)
    {
      return true;
    }
  }
  return false;
}