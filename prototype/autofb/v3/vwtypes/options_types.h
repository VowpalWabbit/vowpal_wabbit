#pragma once

#include "../base.h"

namespace pseudo_vw
{
  struct untyped_option
  {
  };

  template <typename T>
  struct typed_option : untyped_option
  {
  };

  template <typename T>
  struct typed_option_with_location
  {
  };

  template <typename T>
  struct option_builder_t
  {
    template <typename...Args>
    option_builder_t(Args&&...args)
    {
    }

    option_builder_t& short_name(char short_name)
    {
      return *this;
    }

    option_builder_t& short_name(const std::string& short_name)
    {
      return *this;
    }

    option_builder_t& help(const std::string& help)
    {
      return *this;
    }

    option_builder_t& keep(bool hidden = true)
    {
      return *this;
    }

    option_builder_t& necessary(bool necessary = true)
    {
      return *this;
    }

    // option_builder_t& one_of(std::vector<typename T::value_type> args)
    // {
    //   return *this;
    // }

    option_builder_t& allow_override(bool allow_override = true)
    {
      return *this;
    }

    static std::shared_ptr<untyped_option> finalize(option_builder_t&& option)
    {
      return std::make_shared<typed_option<T>>(std::move(option.m_option_obj));
    }
  private:
  };

  template <typename T>
  option_builder_t<typed_option_with_location<T>> make_option(const std::string& name, T& location)
  {
    return {typed_option_with_location<T>()};
  }

  struct option_group_definition
  {
    option_group_definition(const std::string& name)
    {}

    template <typename T>
    option_group_definition& add(option_builder_t<T>&& op)
    {
      return *this;
    }

    option_group_definition& add(option_builder_t<untyped_option>&& op)
    {
      return *this;
    }

  private:
  };

  struct options_builder_t
  {
    void add_and_parse(const option_group_definition& group);
  };

  namespace VW { namespace config
  {
    using options_i = ::pseudo_vw::options_builder_t;

    using option_group_definition = ::pseudo_vw::option_group_definition;
    using base_option = ::pseudo_vw::untyped_option;

    template <typename T>
    using typed_option = ::pseudo_vw::typed_option<T>;

    template <typename T>
    using typed_option_with_location = ::pseudo_vw::typed_option_with_location<T>;

    template <typename T>
    using option_builder = ::pseudo_vw::option_builder_t<T>;
  }};

  //using namespace VW;
}
