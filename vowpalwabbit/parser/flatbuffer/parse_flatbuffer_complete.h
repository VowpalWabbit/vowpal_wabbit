#pragma once

#include "vw.h"
#include "example.h"
#include "input_generated.h"

namespace VW 
{
    void parse_flatbuf_examples(vw* all, v_array<example*>& examples, const ExampleCollection* ec);
    void parse_flatbuf_example(vw* all, example* ae, const Example* eg);
    void parse_flatbuf_namespaces(vw* all, example* ae, const Namespace* ns);
    void parse_flatbuf_features(vw* all, example* ae, features& fs, const Feature* feature, unsigned char _index, uint64_t _channel_hash);
    void read_flatbuffer(vw* all, char* line, size_t len, v_array<example*>& examples);
    int flatbuffer_to_examples(vw* all, v_array<example*>& examples);
    static int example_index=0;
}