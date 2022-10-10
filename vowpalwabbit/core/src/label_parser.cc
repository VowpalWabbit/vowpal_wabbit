// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/label_parser.h"

#include "vw/common/vw_exception.h"
#include "vw/core/cb.h"
#include "vw/core/cb_continuous_label.h"
#include "vw/core/ccb_label.h"
#include "vw/core/cost_sensitive.h"
#include "vw/core/multiclass.h"
#include "vw/core/multilabel.h"
#include "vw/core/no_label.h"
#include "vw/core/simple_label_parser.h"
#include "vw/core/slates_label.h"

VW::label_parser VW::get_label_parser(VW::label_type_t label_type)
{
  switch (label_type)
  {
    case VW::label_type_t::simple:
      return VW::simple_label_parser_global;

    case VW::label_type_t::cb:
      return CB::cb_label;

    case VW::label_type_t::cb_eval:
      return CB_EVAL::cb_eval;

    case VW::label_type_t::cs:
      return VW::cs_label_parser_global;

    case VW::label_type_t::multilabel:
      return MULTILABEL::multilabel;

    case VW::label_type_t::multiclass:
      return VW::multiclass_label_parser_global;

    case VW::label_type_t::ccb:
      return VW::ccb_label_parser_global;

    case VW::label_type_t::slates:
      return VW::slates::slates_label_parser;

    case VW::label_type_t::nolabel:
      return VW::no_label_parser_global;

    case VW::label_type_t::continuous:
      return VW::cb_continuous::the_label_parser;
  }

  THROW("Unknown label type in get_label_parser. This should be unreachable code.")
}
