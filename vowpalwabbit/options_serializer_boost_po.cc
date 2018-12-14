#include "options_serializer_boost_po.h"

using namespace VW;

std::string options_serializer_boost_po::str() {
  return m_output_stream.str();
}

const char* options_serializer_boost_po::data() {
  return m_output_stream.str().c_str();
}

size_t options_serializer_boost_po::size() {
  return m_output_stream.str().size();
}

void options_serializer_boost_po::add(base_option& option) {
  // If the serializer should only save keep args early exit here for non keep.
  if (m_only_serialize_keep_args && !option.m_keep) {
    return;
  }

  if (serialize_if_t<int>(option)) { return; }
  if (serialize_if_t<float>(option)) { return; }
  if (serialize_if_t<char>(option)) { return; }
  if (serialize_if_t<std::string>(option)) { return; }
  if (serialize_if_t<bool>(option)) { return; }
  if (serialize_if_t<std::vector<int>>(option)) { return; }
  if (serialize_if_t<std::vector<float>>(option)) { return; }
  if (serialize_if_t<std::vector<char>>(option)) { return; }
  if (serialize_if_t<std::vector<std::string>>(option)) { return; }

  THROW("That is an unsupported option type.");
}

template <>
void options_serializer_boost_po::serialize<bool>(typed_option<bool> typed_option) {
  if (typed_option.value()) {
    m_output_stream << " --" << typed_option.m_name;
  }
}
