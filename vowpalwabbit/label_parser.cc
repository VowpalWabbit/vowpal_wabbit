// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "label_parser.h"

#include "simple_label_parser.h"
#include "cb.h"
#include "cost_sensitive.h"
#include "multilabel.h"
#include "multiclass.h"
#include "ccb_label.h"
#include "slates_label.h"
#include "no_label.h"
#include "cb_continuous_label.h"
#include "vw_exception.h"

label_parser VW::get_label_parser(VW::label_type_t label_type)
{
  switch (label_type)
  {
    case VW::label_type_t::simple:
      return simple_label_parser;

    case VW::label_type_t::cb:
      return CB::cb_label;

    case VW::label_type_t::cb_eval:
      return CB_EVAL::cb_eval;

    case VW::label_type_t::cs:
      return COST_SENSITIVE::cs_label;

    case VW::label_type_t::multilabel:
      return MULTILABEL::multilabel;

    case VW::label_type_t::multiclass:
      return MULTICLASS::mc_label;

    case VW::label_type_t::ccb:
      return CCB::ccb_label_parser;

    case VW::label_type_t::slates:
      return VW::slates::slates_label_parser;

    case VW::label_type_t::nolabel:
      return no_label::no_label_parser;

    case VW::label_type_t::continuous:
      return VW::cb_continuous::the_label_parser;
  }

  THROW("Unreachable code reached!")
}
