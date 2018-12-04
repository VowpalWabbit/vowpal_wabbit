#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include "arguments_boost_po.h"

#include <memory>
#include <vector>


char** convert_to_command_args(char* command_line, int& argc, int max_args = 64) {
  char** argv = new char*[max_args];
  argc = 0;

  char* current_arg = strtok(command_line, " ");
  while (current_arg)
  {
    argv[argc++] = current_arg;
    current_arg = strtok(0, " ");
  }
  argv[argc] = 0;

  return argv;
}

BOOST_AUTO_TEST_CASE(typed_arguments_parsing) {
  std::string str_arg;
  int int_arg;
  bool bool_arg;
  char char_arg;
  float float_arg;

  char command_line[] = "exe --str_arg test_str -i 5 --bool_arg yes --char_arg f --float_arg 4.3";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<arguments_i> arguments = std::unique_ptr<arguments_boost_po>(
    new arguments_boost_po(argc, argv.get()));

  argument_group_definition arg_group("group");
  arg_group.add(make_typed_arg("str_arg", &str_arg));
  arg_group.add(make_typed_arg("int_arg", &int_arg).short_name("i"));
  arg_group.add(make_typed_arg("bool_arg", &bool_arg));
  arg_group.add(make_typed_arg("char_arg", &char_arg));
  arg_group.add(make_typed_arg("float_arg", &float_arg));

  arguments->add_and_parse(arg_group);

  BOOST_CHECK_EQUAL(str_arg, "test_str");
  BOOST_CHECK_EQUAL(int_arg, 5);
  BOOST_CHECK_EQUAL(bool_arg, true);
  BOOST_CHECK_EQUAL(char_arg, 'f');
  BOOST_CHECK_CLOSE(float_arg, 4.3f, 0.001f);
}

BOOST_AUTO_TEST_CASE(typed_arguments_collection_parsing) {
  std::vector<std::string> str_arg;
  std::vector<int> int_arg;
  std::vector<char> char_arg;
  std::vector<float> float_arg;

  char command_line[] = "exe --str_arg test_str another -i 5 --char_arg f --char_arg f g --float_arg 4.3 --str_arg at_end";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<arguments_i> arguments = std::unique_ptr<arguments_boost_po>(
    new arguments_boost_po(argc, argv.get()));

  argument_group_definition arg_group("group");
  arg_group.add(make_typed_arg("str_arg", &str_arg));
  arg_group.add(make_typed_arg("int_arg", &int_arg).short_name("i"));
  arg_group.add(make_typed_arg("char_arg", &char_arg));
  arg_group.add(make_typed_arg("float_arg", &float_arg));

  arguments->add_and_parse(arg_group);

  check_vectors(str_arg, { "test_str", "another", "at_end" });
  check_vectors(int_arg, { 5 });
  check_vectors(char_arg, { 'f', 'f', 'g' });
  check_float_vectors(float_arg, { 4.3f }, 0.001f);
}

BOOST_AUTO_TEST_CASE(bool_implicit_and_explicit_args) {
  bool bool_switch;
  bool bool_switch_unspecified;

  char command_line[] = "exe --bool_switch";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<arguments_i> arguments = std::unique_ptr<arguments_boost_po>(
    new arguments_boost_po(argc, argv.get()));

  argument_group_definition arg_group("group");
  arg_group.add(make_typed_arg("bool_switch", &bool_switch));
  arg_group.add(make_typed_arg("bool_switch_unspecified", &bool_switch_unspecified));

  arguments->add_and_parse(arg_group);

  BOOST_CHECK_EQUAL(bool_switch, true);
  BOOST_CHECK_EQUAL(bool_switch_unspecified, false);
}
BOOST_AUTO_TEST_CASE(incorrect_arg_type) {
  int int_arg;

  char command_line[] = "exe --int_arg str";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<arguments_i> arguments = std::unique_ptr<arguments_boost_po>(
    new arguments_boost_po(argc, argv.get()));

  argument_group_definition arg_group("group");
  arg_group.add(make_typed_arg("int_arg", &int_arg));

  BOOST_CHECK_THROW(arguments->add_and_parse(arg_group), VW::vw_argument_invalid_value_exception);
}
BOOST_AUTO_TEST_CASE(multiple_locations_one_arg) {
  std::string str_arg_1;
  std::string str_arg_2;

  char command_line[] = "exe --str_arg value";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<arguments_i> arguments = std::unique_ptr<arguments_boost_po>(
    new arguments_boost_po(argc, argv.get()));

  argument_group_definition arg_group("group");
  arg_group.add(make_typed_arg("str_arg", &str_arg_1));
  arg_group.add(make_typed_arg("str_arg", &str_arg_2));

  arguments->add_and_parse(arg_group);

  BOOST_CHECK_EQUAL(str_arg_1, "value");
  BOOST_CHECK_EQUAL(str_arg_2, "value");
}
BOOST_AUTO_TEST_CASE(duplicate_arg_clash) {}
BOOST_AUTO_TEST_CASE(mismatched_values_duplicate_command_line) {}
BOOST_AUTO_TEST_CASE(matching_values_duplicate_command_line) {}
BOOST_AUTO_TEST_CASE(merge_two_groups) {}
BOOST_AUTO_TEST_CASE(was_supplied_test) {}

