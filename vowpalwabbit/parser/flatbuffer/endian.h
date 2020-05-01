#pragma once
#include <cstdint>

namespace VW { 
  struct endian {
    static bool is_big_endian(void);
    static std::uint32_t htonl(uint32_t host_l);
    static std::uint16_t htons(uint16_t host_l);
    static std::uint32_t ntohl(uint32_t net_l);
    static std::uint16_t ntohs(uint16_t net_s);
  };
}