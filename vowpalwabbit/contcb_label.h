#pragma once

#include "label_parser.h"

namespace VW
{
namespace continuous_cb
{
  struct label
  {
    float action;
    float cost;
  };

  extern label_parser contcb_label;

} // namespace continuous_cb
}
