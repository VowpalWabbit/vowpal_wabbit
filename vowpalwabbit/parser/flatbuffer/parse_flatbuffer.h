
#pragma once

#include "vw.h"
#include "example.h"

namespace VW 
{
    void read_flatbuffer(vw* all, char* line, size_t len, v_array<example*>& examples);
    int flatbuffer_to_examples(vw* all, v_array<example*>& examples);
}