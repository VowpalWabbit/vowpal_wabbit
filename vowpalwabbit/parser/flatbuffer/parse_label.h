#pragma once

#include "../../vw.h"
#include "../../example.h"
#include "generated/example_generated.h"

namespace VW {
namespace parsers {
namespace flatbuffer {
    void parse_simple_label(vw*all, example* ae, const SimpleLabel* label);
    void parse_cb_label(example* ae, const CBLabel* label);
    void parse_ccb_label(example* ae, const CCBLabel* label);
    void parse_cs_label(example* ae, const CS_Label* label);
    void parse_cb_eval_label(example* ae, const CB_EVAL_Label* label);
    void parse_mc_label(vw* all, example* ae, const MultiClass* label);
    void parse_multi_label(example* ae, const MultiLabel* label);
    void parse_no_label();
    void parse_slates_label(example* ae, const Slates_Label* label);
} // f;atbuffer
} // parsers
} // VW
