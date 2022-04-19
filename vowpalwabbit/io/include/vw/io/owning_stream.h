// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <memory>
#include <ostream>

namespace VW
{
namespace io
{
class owning_ostream : public std::ostream
{
public:
  owning_ostream(std::unique_ptr<std::streambuf>&& output)
      : std::ostream(output.get()), _output_buffer(std::move(output))
  {
  }

private:
  std::unique_ptr<std::streambuf> _output_buffer;
};

}  // namespace io
}  // namespace VW