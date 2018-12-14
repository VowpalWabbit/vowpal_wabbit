#pragma once

#include "options.h"

#include "vw_exception.h"

#include <sstream>

namespace VW {
  struct options_serializer_boost_po : options_serializer_i {
    options_serializer_boost_po(bool only_serialize_keep_opts) :
      m_only_serialize_keep_args(only_serialize_keep_opts)
    {}

    template <typename T>
    bool serialize_if_t(base_option& base_option) {
      if (base_option.m_type_hash == typeid(T).hash_code()) {
        auto typed = dynamic_cast<typed_option<T>&>(base_option);
        serialize(typed);
        return true;
      }

      return false;
    }

    template <typename T>
    void serialize(typed_option<T> typed_option) {
      m_output_stream << " --" << typed_option.m_name << " " << typed_option.value();
    }

    template <typename T>
    void serialize(typed_option<std::vector<T>> typed_option) {
      auto vec = typed_option.value();
      if (vec.size() > 0) {
        m_output_stream << " --" << typed_option.m_name;
        for (auto const& value : vec) {
          m_output_stream << " " << value;
        }
      }
    }

    virtual void add(base_option& option) override;
    virtual std::string str() override;
    virtual const char* data() override;
    virtual size_t size() override;

  private:
    std::stringstream m_output_stream;
    bool m_only_serialize_keep_args;
  };

  template <>
  void options_serializer_boost_po::serialize<bool>(typed_option<bool> typed_argument);
}
