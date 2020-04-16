#pragma once

#include "label_parser.h"

namespace CONTCB
{
  struct label
  {
    float action;
    float cost;
  };

  extern label_parser contcb_label;

} // namespace CONTCB
