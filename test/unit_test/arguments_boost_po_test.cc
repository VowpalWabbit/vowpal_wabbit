#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include "arguments_boost_po.h"

#include <memory>
#include <vector>

using namespace VW;


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

  BOOST_CHECK_NO_THROW(arguments->add_and_parse(arg_group));

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

  BOOST_CHECK_NO_THROW(arguments->add_and_parse(arg_group));

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

  BOOST_CHECK_NO_THROW(arguments->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(str_arg_1, "value");
  BOOST_CHECK_EQUAL(str_arg_2, "value");
}

BOOST_AUTO_TEST_CASE(duplicate_arg_clash) {
  int int_arg;
  char char_arg;

  char command_line[] = "exe --the_arg s";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<arguments_i> arguments = std::unique_ptr<arguments_boost_po>(
    new arguments_boost_po(argc, argv.get()));

  argument_group_definition arg_group("group");
  arg_group.add(make_typed_arg("the_arg", &int_arg));
  arg_group.add(make_typed_arg("the_arg", &char_arg));

  BOOST_CHECK_THROW(arguments->add_and_parse(arg_group), VW::vw_exception);
}

BOOST_AUTO_TEST_CASE(mismatched_values_duplicate_command_line) {
  int int_arg;

  char command_line[] = "exe --int_arg 3 --int_arg 5";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<arguments_i> arguments = std::unique_ptr<arguments_boost_po>(
    new arguments_boost_po(argc, argv.get()));

  argument_group_definition arg_group("group");
  arg_group.add(make_typed_arg("int_arg", &int_arg));

  BOOST_CHECK_THROW(arguments->add_and_parse(arg_group), VW::vw_argument_disagreement_exception);
}

BOOST_AUTO_TEST_CASE(matching_values_duplicate_command_line) {
  int int_arg;

  char command_line[] = "exe --int_arg 3 --int_arg 3";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<arguments_i> arguments = std::unique_ptr<arguments_boost_po>(
    new arguments_boost_po(argc, argv.get()));

  argument_group_definition arg_group("group");
  arg_group.add(make_typed_arg("int_arg", &int_arg));

  BOOST_CHECK_NO_THROW(arguments->add_and_parse(arg_group));
  BOOST_CHECK_EQUAL(int_arg, 3);
}

BOOST_AUTO_TEST_CASE(merge_two_groups) {
  int int_arg;
  std::string str_arg;

  char command_line[] = "exe --int_arg 3 --str_arg test";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<arguments_i> arguments = std::unique_ptr<arguments_boost_po>(
    new arguments_boost_po(argc, argv.get()));

  argument_group_definition arg_group1("group1");
  arg_group1.add(make_typed_arg("int_arg", &int_arg));

  argument_group_definition arg_group2("group2");
  arg_group2.add(make_typed_arg("str_arg", &str_arg));

  BOOST_CHECK_NO_THROW(arguments->add_and_parse(arg_group1));
  BOOST_CHECK_NO_THROW(arguments->add_and_parse(arg_group2));
  BOOST_CHECK_EQUAL(int_arg, 3);
  BOOST_CHECK_EQUAL(str_arg, "test");
}

BOOST_AUTO_TEST_CASE(was_supplied_test) {
  int int_arg;
  std::string str_arg;
  bool bool_arg;

  char command_line[] = "exe --int_arg 3 --str_arg test";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<arguments_i> arguments = std::unique_ptr<arguments_boost_po>(
    new arguments_boost_po(argc, argv.get()));

  argument_group_definition arg_group("group1");
  arg_group.add(make_typed_arg("int_arg", &int_arg));
  arg_group.add(make_typed_arg("str_arg", &str_arg));
  arg_group.add(make_typed_arg("bool_arg", &bool_arg));

  BOOST_CHECK_NO_THROW(arguments->add_and_parse(arg_group));
  BOOST_CHECK_EQUAL(int_arg, 3);
  BOOST_CHECK_EQUAL(str_arg, "test");
  BOOST_CHECK_EQUAL(bool_arg, false);

  BOOST_CHECK_EQUAL(arguments->was_supplied("int_arg"), true);
  BOOST_CHECK_EQUAL(arguments->was_supplied("str_arg"), true);
  BOOST_CHECK_EQUAL(arguments->was_supplied("bool_arg"), false);
  BOOST_CHECK_EQUAL(arguments->was_supplied("other_arg"), false);
}

BOOST_AUTO_TEST_CASE(kept_command_line) {
  int int_arg;
  std::string str_arg;
  bool bool_arg;
  bool other_bool_arg;
  std::vector<char> char_vec_arg;

  char command_line[] = "exe --int_arg 3 --str_arg test --other_bool_arg --char_vec_arg a c --char_vec_arg d";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<arguments_i> arguments = std::unique_ptr<arguments_boost_po>(
    new arguments_boost_po(argc, argv.get()));

  argument_group_definition arg_group("group1");
  arg_group.add(make_typed_arg("int_arg", &int_arg));
  arg_group.add(make_typed_arg("str_arg", &str_arg).keep());
  arg_group.add(make_typed_arg("bool_arg", &bool_arg).keep());
  arg_group.add(make_typed_arg("other_bool_arg", &other_bool_arg).keep());
  arg_group.add(make_typed_arg("char_vec_arg", &char_vec_arg).keep());

  BOOST_CHECK_NO_THROW(arguments->add_and_parse(arg_group));
  BOOST_CHECK_EQUAL(int_arg, 3);
  BOOST_CHECK_EQUAL(str_arg, "test");
  BOOST_CHECK_EQUAL(bool_arg, false);
  BOOST_CHECK_EQUAL(other_bool_arg, true);
  BOOST_CHECK_NE(arguments->get_kept().find("--str_arg test"), std::string::npos);
  BOOST_CHECK_NE(arguments->get_kept().find("--other_bool_arg"), std::string::npos);
  BOOST_CHECK_NE(arguments->get_kept().find("--char_vec_arg a c d"), std::string::npos);
}

BOOST_AUTO_TEST_CASE(unregistered_arguments) {
  int int_arg;

  char command_line[] = "exe --int_arg 3 --str_arg test";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<arguments_i> arguments = std::unique_ptr<arguments_boost_po>(
    new arguments_boost_po(argc, argv.get()));

  argument_group_definition arg_group("group1");
  arg_group.add(make_typed_arg("int_arg", &int_arg));

  BOOST_CHECK_NO_THROW(arguments->add_and_parse(arg_group));
  BOOST_CHECK_EQUAL(int_arg, 3);

  BOOST_CHECK_THROW(arguments->check_unregistered(), VW::vw_exception);
}
