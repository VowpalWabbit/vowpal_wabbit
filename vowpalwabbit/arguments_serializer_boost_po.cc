#include "arguments_serializer_boost_po.h"

using namespace VW;

std::string arguments_serializer_boost_po::str() {
  return m_output_stream.str();
}

const char* arguments_serializer_boost_po::data() {
  return m_output_stream.str().c_str();
}

size_t arguments_serializer_boost_po::size() {
  return m_output_stream.str().size();
}

void arguments_serializer_boost_po::add(base_argument& argument) {
  // If the serializer should only save keep args early exit here for non keep.
  if (m_only_serialize_keep_args && !argument.m_keep) {
    return;
  }

  if (serialize_if_t<int>(argument)) { return; }
  if (serialize_if_t<float>(argument)) { return; }
  if (serialize_if_t<char>(argument)) { return; }
  if (serialize_if_t<std::string>(argument)) { return; }
  if (serialize_if_t<bool>(argument)) { return; }
  if (serialize_if_t<std::vector<int>>(argument)) { return; }
  if (serialize_if_t<std::vector<float>>(argument)) { return; }
  if (serialize_if_t<std::vector<char>>(argument)) { return; }
  if (serialize_if_t<std::vector<std::string>>(argument)) { return; }

  THROW("That is an unsupported argument type.");
}

template <>
void arguments_serializer_boost_po::serialize<bool>(typed_argument<bool> typed_argument) {
  if (typed_argument.value()) {
    m_output_stream << " --" << typed_argument.m_name;
  }
}
