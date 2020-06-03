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
