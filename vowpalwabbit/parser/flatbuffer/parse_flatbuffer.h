#pragma once

#include "vw.h"
#include "example.h"
#include "test_generated.h"

namespace VW 
{
    void parse_flatbuf_examples(vw* all, v_array<example*>& examples, const Data* data);
    void parse_flatbuf_example(vw* all, example* ae, const Datapoint* datapoint);
    void parse_flatbuf_features(vw* all, example* ae, features& fs, const Feature* feature, unsigned char _index);
    void read_flatbuffer(vw* all, char* line, size_t len, v_array<example*>& examples);
    int flatbuffer_to_examples(vw* all, v_array<example*>& examples);
    static int index=0;
}