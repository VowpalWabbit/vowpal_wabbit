#include "type_registry.h"

namespace type_system {
  type_registry& type_registry::instance() 
  {
    static type_registry instance;
    return instance;
  }

  type_registry::type_registry()
  {
    #define REGISTER_BUILTIN(_type) this->register_type_internal(#_type, type< ## _type ## >::erase(), true);
    REGISTER_BUILTIN(int8_t);
    REGISTER_BUILTIN(int16_t);
    REGISTER_BUILTIN(int32_t);
    REGISTER_BUILTIN(int64_t);
    REGISTER_BUILTIN(uint8_t);
    REGISTER_BUILTIN(uint16_t);
    REGISTER_BUILTIN(uint32_t);
    REGISTER_BUILTIN(uint64_t);
    REGISTER_BUILTIN(float);
    REGISTER_BUILTIN(double);
    REGISTER_BUILTIN(bool);
    REGISTER_BUILTIN(std::string);
    #undef REGISTER_BUILTIN
  }
}