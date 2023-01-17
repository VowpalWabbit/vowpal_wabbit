#include "vw/core/example.h"
#include "vw/core/multi_ex.h"

namespace VW
{
// Polymorphic wrapper around VW::example* and VW::multi_ex*
class polymorphic_ex
{
public:
  polymorphic_ex(VW::example* ex) : _example(ex), _is_multiline(false) {}
  polymorphic_ex(VW::example& ex) : _example(&ex), _is_multiline(false) {}
  polymorphic_ex(VW::multi_ex* ex) : _multi_ex(ex), _is_multiline(true) {}
  polymorphic_ex(VW::multi_ex& ex) : _multi_ex(&ex), _is_multiline(true) {}

  // We need to be able to convert const example/multi_ex into polymorphic_ex
  polymorphic_ex(const VW::example* ex) : _example(const_cast<VW::example*>(ex)), _is_multiline(false) {}
  polymorphic_ex(const VW::example& ex) : _example(const_cast<VW::example*>(&ex)), _is_multiline(false) {}
  polymorphic_ex(const VW::multi_ex* ex) : _multi_ex(const_cast<VW::multi_ex*>(ex)), _is_multiline(true) {}
  polymorphic_ex(const VW::multi_ex& ex) : _multi_ex(const_cast<VW::multi_ex*>(&ex)), _is_multiline(true) {}

  operator VW::example*() { if(_is_multiline) { THROW("Cannot convert example to multi_ex"); } return _example; }
  operator VW::example&() { if(_is_multiline) { THROW("Cannot convert example to multi_ex"); } return *_example; }
  operator VW::multi_ex*() { if(!_is_multiline) { THROW("Cannot convert multi_ex to example"); } return _multi_ex; }
  operator VW::multi_ex&() { if(!_is_multiline) { THROW("Cannot convert multi_ex to example"); } return *_multi_ex; }

  // Can convert back to const example/multi_ex only if the polymorphic_ex class itself is const
  operator const VW::example*() const { if(_is_multiline) { THROW("Cannot convert example to multi_ex"); } return _example; }
  operator const VW::example&() const { if(_is_multiline) { THROW("Cannot convert example to multi_ex"); } return *_example; }
  operator const VW::multi_ex*() const { if(!_is_multiline) { THROW("Cannot convert multi_ex to example"); } return _multi_ex; }
  operator const VW::multi_ex&() const { if(!_is_multiline) { THROW("Cannot convert multi_ex to example"); } return *_multi_ex; }

  bool is_multiline() const { return _is_multiline; }

private:
  VW::example* _example;
  VW::multi_ex* _multi_ex;
  bool _is_multiline = false;
};
}
