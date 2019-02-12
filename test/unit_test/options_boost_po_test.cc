#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include "options_boost_po.h"
#include "options_serializer_boost_po.h"

#include <memory>
#include <vector>

using namespace VW::config;


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

BOOST_AUTO_TEST_CASE(typed_options_parsing) {
  std::string str_arg;
  int int_opt;
  bool bool_opt;
  char char_opt;
  float float_opt;

  char command_line[] = "exe --str_opt test_str -i 5 --bool_opt yes --char_opt f --float_opt 4.3";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<options_i> options = std::unique_ptr<options_boost_po>(
    new options_boost_po(argc, argv.get()));

  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_arg));
  arg_group.add(make_option("int_opt", int_opt).short_name("i"));
  arg_group.add(make_option("bool_opt", bool_opt));
  arg_group.add(make_option("char_opt", char_opt));
  arg_group.add(make_option("float_opt", float_opt));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(str_arg, "test_str");
  BOOST_CHECK_EQUAL(int_opt, 5);
  BOOST_CHECK_EQUAL(bool_opt, true);
  BOOST_CHECK_EQUAL(char_opt, 'f');
  BOOST_CHECK_CLOSE(float_opt, 4.3f, 0.001f);
}

BOOST_AUTO_TEST_CASE(typed_option_collection_parsing) {
  std::vector<std::string> str_opt;
  std::vector<int> int_opt;
  std::vector<char> char_opt;
  std::vector<float> float_opt;

  char command_line[] = "exe --str_opt test_str another -i 5 --char_opt f --char_opt f g --float_opt 4.3 --str_opt at_end";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<options_i> options = std::unique_ptr<options_boost_po>(
    new options_boost_po(argc, argv.get()));

  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_opt));
  arg_group.add(make_option("int_opt", int_opt).short_name("i"));
  arg_group.add(make_option("char_opt", char_opt));
  arg_group.add(make_option("float_opt", float_opt));

  options->add_and_parse(arg_group);

  check_vectors(str_opt, { "test_str", "another", "at_end" });
  check_vectors(int_opt, { 5 });
  check_vectors(char_opt, { 'f', 'f', 'g' });
  check_float_vectors(float_opt, { 4.3f }, 0.001f);
}

BOOST_AUTO_TEST_CASE(bool_implicit_and_explicit_options) {
  bool bool_switch;
  bool bool_switch_unspecified;

  char command_line[] = "exe --bool_switch";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<options_i> options = std::unique_ptr<options_boost_po>(
    new options_boost_po(argc, argv.get()));

  option_group_definition arg_group("group");
  arg_group.add(make_option("bool_switch", bool_switch));
  arg_group.add(make_option("bool_switch_unspecified", bool_switch_unspecified));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(bool_switch, true);
  BOOST_CHECK_EQUAL(bool_switch_unspecified, false);
}

BOOST_AUTO_TEST_CASE(incorrect_option_type) {
  int int_opt;

  char command_line[] = "exe --int_opt str";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<options_i> options = std::unique_ptr<options_boost_po>(
    new options_boost_po(argc, argv.get()));

  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  BOOST_CHECK_THROW(options->add_and_parse(arg_group), VW::vw_argument_invalid_value_exception);
}

BOOST_AUTO_TEST_CASE(multiple_locations_one_option) {
  std::string str_opt_1;
  std::string str_opt_2;

  char command_line[] = "exe --str_opt value";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<options_i> options = std::unique_ptr<options_boost_po>(
    new options_boost_po(argc, argv.get()));

  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_opt_1));
  arg_group.add(make_option("str_opt", str_opt_2));

  BOOST_CHECK_THROW(options->add_and_parse(arg_group), VW::vw_exception);
}

BOOST_AUTO_TEST_CASE(duplicate_option_clash) {
  int int_opt;
  char char_opt;

  char command_line[] = "exe --the_opt s";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<options_i> options = std::unique_ptr<options_boost_po>(
    new options_boost_po(argc, argv.get()));

  option_group_definition arg_group("group");
  arg_group.add(make_option("the_opt", int_opt));
  arg_group.add(make_option("the_opt", char_opt));

  BOOST_CHECK_THROW(options->add_and_parse(arg_group), VW::vw_exception);
}

BOOST_AUTO_TEST_CASE(mismatched_values_duplicate_command_line) {
  int int_opt;

  char command_line[] = "exe --int_opt 3 --int_opt 5";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<options_i> options = std::unique_ptr<options_boost_po>(
    new options_boost_po(argc, argv.get()));

  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  BOOST_CHECK_THROW(options->add_and_parse(arg_group), VW::vw_argument_disagreement_exception);
}

//BOOST_AUTO_TEST_CASE(positional_data_value) {
//  std::string data;
//
//  char command_line[] = "exe data_file";
//  int argc;
//  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
//  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));
//
//  std::unique_ptr<options_i> options = std::unique_ptr<options_boost_po>(
//    new options_boost_po(argc, argv.get()));
//
//  option_group_definition arg_group("group");
//  arg_group.add(make_option("data", data));
//
//  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));
//  BOOST_CHECK_EQUAL(data, "data_file");
//}


BOOST_AUTO_TEST_CASE(matching_values_duplicate_command_line) {
  int int_opt;

  char command_line[] = "exe --int_opt 3 --int_opt 3";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<options_i> options = std::unique_ptr<options_boost_po>(
    new options_boost_po(argc, argv.get()));

  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));
  BOOST_CHECK_EQUAL(int_opt, 3);
}

BOOST_AUTO_TEST_CASE(add_two_groups) {
  int int_opt;
  std::string str_opt;

  char command_line[] = "exe --int_opt 3 --str_opt test";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<options_i> options = std::unique_ptr<options_boost_po>(
    new options_boost_po(argc, argv.get()));

  option_group_definition arg_group1("group1");
  arg_group1.add(make_option("int_opt", int_opt));

  option_group_definition arg_group2("group2");
  arg_group2.add(make_option("str_opt", str_opt));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group1));
  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group2));
  BOOST_CHECK_EQUAL(int_opt, 3);
  BOOST_CHECK_EQUAL(str_opt, "test");
}

BOOST_AUTO_TEST_CASE(was_supplied_test) {
  int int_opt;
  std::string str_opt;
  bool bool_opt;

  char command_line[] = "exe --int_opt 3 --str_opt test";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<options_i> options = std::unique_ptr<options_boost_po>(
    new options_boost_po(argc, argv.get()));

  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt));
  arg_group.add(make_option("str_opt", str_opt));
  arg_group.add(make_option("bool_opt", bool_opt));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));
  BOOST_CHECK_EQUAL(int_opt, 3);
  BOOST_CHECK_EQUAL(str_opt, "test");
  BOOST_CHECK_EQUAL(bool_opt, false);

  BOOST_CHECK_EQUAL(options->was_supplied("int_opt"), true);
  BOOST_CHECK_EQUAL(options->was_supplied("str_opt"), true);
  BOOST_CHECK_EQUAL(options->was_supplied("bool_opt"), false);
  BOOST_CHECK_EQUAL(options->was_supplied("other_opt"), false);
}

BOOST_AUTO_TEST_CASE(kept_command_line) {
  int int_opt;
  std::string str_opt;
  bool bool_opt;
  bool other_bool_opt;
  std::vector<char> char_opt_option;

  char command_line[] = "exe --int_opt 3 --str_opt test --other_bool_opt --char_opt_option a c --char_opt_option d";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<options_i> options = std::unique_ptr<options_boost_po>(
    new options_boost_po(argc, argv.get()));

  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt));
  arg_group.add(make_option("str_opt", str_opt).keep());
  arg_group.add(make_option("bool_opt", bool_opt).keep());
  arg_group.add(make_option("other_bool_opt", other_bool_opt).keep());
  arg_group.add(make_option("char_opt_option", char_opt_option).keep());

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));
  BOOST_CHECK_EQUAL(int_opt, 3);
  BOOST_CHECK_EQUAL(str_opt, "test");
  BOOST_CHECK_EQUAL(bool_opt, false);
  BOOST_CHECK_EQUAL(other_bool_opt, true);

  options_serializer_boost_po serializer;
  for (auto& opt : options->get_all_options()) {
    if (opt->m_keep) {
      serializer.add(*opt);
    }
  }

  auto serialized_string = serializer.str();

  BOOST_CHECK_NE(serialized_string.find("--str_opt test"), std::string::npos);
  BOOST_CHECK_NE(serialized_string.find("--other_bool_opt"), std::string::npos);
  BOOST_CHECK_NE(serialized_string.find("--char_opt_option a --char_opt_option c --char_opt_option d"), std::string::npos);
  BOOST_CHECK_EQUAL(serialized_string.find("--bool_opt"), std::string::npos);
  BOOST_CHECK_EQUAL(serialized_string.find("--int_opt"), std::string::npos);
}

BOOST_AUTO_TEST_CASE(unregistered_options) {
  int int_opt;

  char command_line[] = "exe --int_opt 3 --str_opt test";
  int argc;
  // Only the returned char* needs to be deleted as the individual pointers simply point into command_line.
  std::unique_ptr<char*> argv(convert_to_command_args(command_line, argc));

  std::unique_ptr<options_i> options = std::unique_ptr<options_boost_po>(
    new options_boost_po(argc, argv.get()));

  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));
  BOOST_CHECK_EQUAL(int_opt, 3);

  BOOST_CHECK_THROW(options->check_unregistered(), VW::vw_exception);
}
