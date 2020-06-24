#pragma once

#include "../../vw.h"
#include "../../example.h"
#include "generated/example_generated.h"

namespace VW {
namespace parsers {
namespace flatbuffer {
void parse_examples(vw* all, v_array<example*>& examples, const ExampleCollection* ec);
void parse_example(vw* all, example* ae, const Example* eg);
void parse_namespaces(vw* all, example* ae, const Namespace* ns);
void parse_features(vw* all, example* ae, features& fs, const Feature* feature);
void read_flatbuffer(vw* all, char* line, size_t len, v_array<example*>& examples);
void parse_flat_label(vw* all, example* ae, const Example* eg);
int flatbuffer_to_examples(vw* all, v_array<example*>& examples);
static int example_index=0;
static uint64_t c_hash;
} // flatbuffer
} // parsers
} // VW


class VW::parsers::flatbuffer::parse 
{

public:
    void parse_examples(vw* all, v_array<example*>& examples, const ExampleCollection* ec);
    int flatbuffer_to_examples(vw* all, v_array<example*>& examples);
    void init();
    parse(std::string filename);
    parse(uint8_t *buffer_pointer);
    const VW::parsers::flatbuffer::ExampleCollection* data;

private:
    // Add underscore
    std::string _filename;
    std::ifstream infile;
    uint8_t* flatbuffer_pointer;
    bool to_flat;
    int example_index=0;
    uint64_t c_hash;
    
    void parse_example(vw* all, example* ae, const Example* eg);
    void parse_namespaces(vw* all, example* ae, const Namespace* ns);
    void parse_features(vw* all, example* ae, features& fs, const Feature* feature);
    void read_flatbuffer(vw* all, char* line, size_t len, v_array<example*>& examples);
    void parse_flat_label(vw* all, example* ae, const Example* eg);
};
