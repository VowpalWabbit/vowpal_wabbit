#include "endian.h"

namespace VW {
namespace parsers {
namespace flatbuffer {

    bool endian::is_big_endian(void)
    {
        const union {
            std::uint32_t i;
            char c[4];
        } b_int{0x01000000};
        return b_int.c[0] == 1;
    }

    std::uint32_t endian::htonl(std::uint32_t host_l)
    {
        if (is_big_endian())
        {
            return host_l;
        }
        std::uint32_t ret_val;
        std::uint8_t *p_ret_raw = (std::uint8_t *)&ret_val;
        std::uint8_t *p_raw = (std::uint8_t *)&host_l;
        p_ret_raw[0] = p_raw[3];
        p_ret_raw[1] = p_raw[2];
        p_ret_raw[2] = p_raw[1];
        p_ret_raw[3] = p_raw[0];
        return ret_val;
    }

    std::uint16_t endian::htons(std::uint16_t host_l)
    {
        if (is_big_endian())
        {
            return host_l;
        }
        std::uint16_t ret_val;
        std::uint8_t *p_ret_raw = (std::uint8_t *)&ret_val;
        std::uint8_t *p_raw = (std::uint8_t *)&host_l;
        p_ret_raw[0] = p_raw[1];
        p_ret_raw[1] = p_raw[0];
        return ret_val;
    }

    std::uint32_t endian::ntohl(std::uint32_t net_l)
    {
        if (is_big_endian())
        {
            return net_l;
        }
        std::uint32_t ret_val;
        std::uint8_t *p_ret_raw = (std::uint8_t *)&ret_val;
        std::uint8_t *p_raw = (std::uint8_t *)&net_l;
        p_ret_raw[0] = p_raw[3];
        p_ret_raw[1] = p_raw[2];
        p_ret_raw[2] = p_raw[1];
        p_ret_raw[3] = p_raw[0];
        return ret_val;
    }

    std::uint16_t endian::ntohs(std::uint16_t net_s)
    {
        if (is_big_endian())
        {
            return net_s;
        }
        std::uint16_t ret_val;
        std::uint8_t *p_ret_raw = (std::uint8_t *)&ret_val;
        std::uint8_t *p_raw = (std::uint8_t *)&net_s;
        p_ret_raw[0] = p_raw[1];
        p_ret_raw[1] = p_raw[0];
        return ret_val;
    }
}
}
} // namespace VW
