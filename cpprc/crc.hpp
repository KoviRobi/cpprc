#pragma once

#include <cstdint>

namespace Crc
{
    namespace Detail
    {
        template<size_t width> struct UintN;
        template<> struct UintN<8>  { using T = uint8_t;  };
        template<> struct UintN<16> { using T = uint16_t; };
        template<> struct UintN<32> { using T = uint32_t; };
        template<> struct UintN<64> { using T = uint64_t; };

        // Rounds up to the nearest multiple of 8
        constexpr uint8_t roundUpTo8N(uint8_t x)
        {
            return ((x + 7) / 8) * 8;
        }

        // Minimal uintX_t which can store up to N
        template<size_t N>
        using Uint = UintN<roundUpTo8N(N)>;
    };
};
