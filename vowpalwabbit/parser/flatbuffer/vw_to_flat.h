#pragma once

#include "vw.h"
#include "example_predict.h"
#include "flatbuffers/flatbuffers.h"
#include "generated/example_generated.h"

namespace VW{
  void convert_txt_to_flat(vw& all);
  namespace parsers {
    namespace flatbuffer {
      flatbuffers::Offset<VW::parsers::flatbuffer::Label>
      parse_flat_label(vw& all, flatbuffers::FlatBufferBuilder builder, example* v);
    }
  }
}