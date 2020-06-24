#pragma once

#include "../../vw.h"
#include "../../example.h"
#include "generated/example_generated.h"

namespace VW {
namespace parsers {
namespace flatbuffer {
    void parse_simple_label(shared_data* sd, polylabel* l, const SimpleLabel* label);
    void parse_cb_label(polylabel* l, const CBLabel* label);
    void parse_ccb_label(polylabel* l, const CCBLabel* label);
    void parse_cs_label(polylabel* l, const CS_Label* label);
    void parse_cb_eval_label(polylabel* l, const CB_EVAL_Label* label);
    void parse_mc_label(shared_data* sd, polylabel* l, const MultiClass* label);
    void parse_multi_label(polylabel* l, const MultiLabel* label);
    void parse_no_label();
    void parse_slates_label(polylabel* l, const Slates_Label* label);
} // f;atbuffer
} // parsers
} // VW
