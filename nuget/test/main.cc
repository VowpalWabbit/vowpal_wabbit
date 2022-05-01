#include "vw/core/vw.h"

int main()
{
  auto* workspace = VW::initialize("--quiet");
  VW::finish(*workspace);
}
