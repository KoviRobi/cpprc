#pragma once

#include <bit>
#include <cstdint>
#include <limits>

namespace Crc
{
    namespace Detail
    {
        // Size-templated types ================================================

        template<uint8_t width> struct UintN;
        template<> struct UintN<8>  { using Type = uint8_t;  };
        template<> struct UintN<16> { using Type = uint16_t; };
        template<> struct UintN<32> { using Type = uint32_t; };
        template<> struct UintN<64> { using Type = uint64_t; };

        // Rounds up to the nearest multiple of 8
        constexpr uint8_t roundUpTo8N(uint8_t x)
        {
            return ((x + 7) / 8) * 8;
        }

        // Minimal uintX_t which can store up to N
        template<uint8_t N>
        using Uint = UintN<roundUpTo8N(std::bit_ceil(N))>::Type;

        // Bitshift ============================================================

        // A mask like 0x00FF/0x0F0F/0x3333/0x5555 used for bitswap
        template<uint8_t N>
        constexpr Uint<N> bitswapMask(uint8_t digits)
        {
            Uint<N> mask = (1 << digits) - 1;
            Uint<N> oldMask = 0;
            while (mask != oldMask)
            {
                oldMask = mask;
                const auto gap = mask << digits;
                mask = mask | (gap << digits);
            }
            return mask;
        }

        template<uint8_t N>
        constexpr Uint<N> bitswap(Uint<N> value)
        {
            constexpr uint8_t digits = std::numeric_limits<Uint<N>>::digits;
            for (uint8_t digit = digits >> 1; digit != 0; digit = digit >> 1)
            {
                const Uint<N> rmask = bitswapMask<N>(digit);
                const Uint<N> lmask = rmask << digit;
                value = ((value & lmask) >> digit) | ((value & rmask) << digit);
            }
            return value;
        }

        // Cyclic Redundancy Check (table driven/Sarwate algorithm) ===========

        enum Bitorder
        {
            Msb,
            Lsb,
        };

        template<uint8_t width, Uint<width> polynomial, Bitorder bitorder>
        struct Impl
        {
            static constexpr auto poly = bitorder == Msb ? polynomial : bitswap(polynomial);
            Uint<width> checksum;
        };
    };
};
