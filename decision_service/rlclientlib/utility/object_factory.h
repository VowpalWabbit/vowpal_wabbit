#pragma once

#include "config_collection.h"

#include <functional>
#include <memory>

namespace reinforcement_learning { namespace utility
{
    template<class I>
    struct object_factory
    {
      using create_fn = std::function<I*(const config_collection&)>;

      void register_type(const std::string& name, create_fn fptr) { _creators[name] = fptr; }

      std::unique_ptr<I> create(const std::string& name, const config_collection& cc)
      {
        auto it = _creators.find(name);

        if (it != _creators.end()) 
          return std::unique_ptr<I>((it->second)(cc));

        throw std::runtime_error("type not registered with class factory");
      }

    private:
      std::unordered_map<std::string, create_fn> _creators;
    };
}
}
