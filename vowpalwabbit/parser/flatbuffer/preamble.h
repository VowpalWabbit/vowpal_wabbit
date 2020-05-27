#pragma once
#include <cstddef>
#include <cstdint>

namespace VW {
namespace parsers {
namespace flatbuffer {
struct preamble {
  uint8_t reserved = 0;
  uint8_t version = 0;
  uint16_t msg_type = 0;
  uint32_t msg_size = 0;

  bool write_to_bytes(uint8_t* buffer, size_t buffersz);
  bool read_from_bytes(uint8_t* buffer, size_t buffersz);
  constexpr static uint32_t size() { return 8; };
};
}
}
}
