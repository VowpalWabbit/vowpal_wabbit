#include "vw/core/example.h"
#include "vw/core/multi_ex.h"

namespace VW
{
// Polymorphic wrapper around VW::example* and VW::multi_ex*
class polymorphic_ex
{
public:
  // This will implicitly convert both `example` and `const example` pointers and references into polymorphic_ex
  polymorphic_ex(const VW::example* ex) : _example(ex), _is_multiline(false) {}
  polymorphic_ex(const VW::example& ex) : _example(&ex), _is_multiline(false) {}
  polymorphic_ex(const VW::multi_ex* ex) : _multi_ex(ex), _is_multiline(true) {}
  polymorphic_ex(const VW::multi_ex& ex) : _multi_ex(&ex), _is_multiline(true) {}

  // Can always implicitly convert back to `const example`
  operator const VW::example*() const
  {
    if (_is_multiline) { THROW("Cannot convert example to multi_ex"); }
    return _example;
  }
  operator const VW::example&() const
  {
    if (_is_multiline) { THROW("Cannot convert example to multi_ex"); }
    return *_example;
  }
  operator const VW::multi_ex*() const
  {
    if (!_is_multiline) { THROW("Cannot convert multi_ex to example"); }
    return _multi_ex;
  }
  operator const VW::multi_ex&() const
  {
    if (!_is_multiline) { THROW("Cannot convert multi_ex to example"); }
    return *_multi_ex;
  }

  // Can implicitly convert back to mutable `example` only if the polymorphic_ex object itself is not const
  operator VW::example*()
  {
    if (_is_multiline) { THROW("Cannot convert example to multi_ex"); }
    return const_cast<VW::example*>(_example);
  }
  operator VW::example&()
  {
    if (_is_multiline) { THROW("Cannot convert example to multi_ex"); }
    return const_cast<VW::example&>(*_example);
  }
  operator VW::multi_ex*()
  {
    if (!_is_multiline) { THROW("Cannot convert multi_ex to example"); }
    return const_cast<VW::multi_ex*>(_multi_ex);
  }
  operator VW::multi_ex&()
  {
    if (!_is_multiline) { THROW("Cannot convert multi_ex to example"); }
    return const_cast<VW::multi_ex&>(*_multi_ex);
  }

  bool is_multiline() const { return _is_multiline; }

private:
  const VW::example* _example;
  const VW::multi_ex* _multi_ex;
  const bool _is_multiline = false;
};
}  // namespace VW
