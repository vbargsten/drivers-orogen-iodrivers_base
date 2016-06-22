#ifndef IODRIVERS_BASE_RAW_PACKET_HPP
#define IODRIVERS_BASE_RAW_PACKET_HPP

#include <vector>
#include <base/Time.hpp>

namespace iodrivers_base
{
    struct RawPacket
    {
        base::Time time;
        std::vector<uint8_t> data;
    };
}

#endif

