// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "../../vw.h"
#include "../../example.h"
#include "generated/example_generated.h"

namespace VW {
namespace parsers {
namespace flatbuffer {
void read_flatbuffer(vw* all, char* line, size_t len, v_array<example*>& examples);
int flatbuffer_to_examples(vw* all, v_array<example*>& examples);
} // flatbuffer
} // parsers
} // VW

class VW::parsers::flatbuffer::parser 
{

public:
    bool parse_examples(vw* all, v_array<example*>& examples);
    void init();
    parser(std::string filename);
    parser(uint8_t *buffer_pointer);
    const VW::parsers::flatbuffer::ExampleCollection* data();

private:
    const VW::parsers::flatbuffer::ExampleCollection* _data;
    std::string _filename;
    uint8_t* _flatbuffer_pointer;
    std::vector<char> buffer;
    size_t _example_index;
    uint64_t _c_hash;
    
    void parse_example(vw* all, example* ae, const Example* eg);
    void parse_namespaces(vw* all, example* ae, const Namespace* ns);
    void parse_features(vw* all, features& fs, const Feature* feature);
    void parse_flat_label(shared_data* sd, example* ae, const Example* eg);

    void parse_simple_label(shared_data* sd, polylabel* l, const SimpleLabel* label);
    void parse_cb_label(polylabel* l, const CBLabel* label);
    void parse_ccb_label(polylabel* l, const CCBLabel* label);
    void parse_cs_label(polylabel* l, const CS_Label* label);
    void parse_cb_eval_label(polylabel* l, const CB_EVAL_Label* label);
    void parse_mc_label(shared_data* sd, polylabel* l, const MultiClass* label);
    void parse_multi_label(polylabel* l, const MultiLabel* label);
    void parse_no_label();
    void parse_slates_label(polylabel* l, const Slates_Label* label);
};
