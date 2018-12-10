#pragma once

#include "arguments.h"

#include "vw_exception.h"

#include <sstream>

namespace VW {
  struct arguments_serializer_boost_po : arguments_serializer_i {
    arguments_serializer_boost_po(bool only_serialize_keep_args) :
      m_only_serialize_keep_args(only_serialize_keep_args)
    {}

    template <typename T>
    bool serialize_if_t(base_argument& base_argument) {
      if (base_argument.m_type_hash == typeid(T).hash_code()) {
        auto typed = dynamic_cast<typed_argument<T>&>(base_argument);
        serialize(typed);
        return true;
      }

      return false;
    }

    template <typename T>
    void serialize(typed_argument<T> typed_argument) {
      m_output_stream << " --" << typed_argument.m_name << " " << typed_argument.value();
    }

    template <typename T>
    void serialize(typed_argument<std::vector<T>> typed_argument) {
      auto vec = typed_argument.value();
      if (vec.size() > 0) {
        m_output_stream << " --" << typed_argument.m_name;
        for (auto const& value : vec) {
          m_output_stream << " " << value;
        }
      }
    }

    virtual void add(base_argument& argument) override;
    virtual std::string str() override;
    virtual const char* data() override;
    virtual size_t size() override;

  private:
    std::stringstream m_output_stream;
    bool m_only_serialize_keep_args;
  };

  template <>
  void arguments_serializer_boost_po::serialize<bool>(typed_argument<bool> typed_argument);
}
