#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <ranges>

// With help from
// https://en.wikipedia.org/w/index.php?title=Computation_of_cyclic_redundancy_checks&oldid=1334498958

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
        // Note: Wraps to zero after 248, but that is fine as it is only
        // called with bit_ceil which returns values up to 64.
        constexpr uint8_t roundUpTo8N(uint8_t x)
        {
            return static_cast<uint8_t>(((x + 7) / 8) * 8);
        }

        // Minimal uintX_t which can store up to N
        template<uint8_t N>
        using Uint = UintN<roundUpTo8N(std::bit_ceil(N))>::Type;

        // Bitshift ============================================================

        // A mask like 0x00FF/0x0F0F/0x3333/0x5555 used for bitswap
        template<uint8_t N>
        constexpr Uint<N> bitswapMask(uint8_t digits)
        {
            // Cast is ok, value is at least 1 (no uint0_t)
            Uint<N> mask = static_cast<Uint<N>>((Uint<N>(1) << digits) - 1);
            Uint<N> oldMask = 0;
            while (mask != oldMask)
            {
                oldMask = mask;
                const auto gap = mask << digits;
                // Cast is ok, we want RHS to fall off the edge to
                // stop changing to terminate the loop
                mask = static_cast<Uint<N>>(mask | (gap << digits));
            }
            return mask;
        }

        template<uint8_t N>
        constexpr Uint<N> bitswap(Uint<N> value)
        {
            // Handle non power-of-2 numbers, by bitswapping using the
            // nearest power-of-2 larger or equal to it, then shifting
            // right
            constexpr uint8_t digits = std::numeric_limits<Uint<N>>::digits;
            for (uint8_t digit = digits >> 1; digit != 0; digit = digit >> 1)
            {
                const Uint<N> rmask = bitswapMask<N>(digit);
                const Uint<N> lmask = rmask << digit;
                value = ((value & lmask) >> digit) | ((value & rmask) << digit);
            }
            return value >> (digits - N);
        }

        // Cyclic Redundancy Check (table driven/Sarwate algorithm) ===========

        // Also corresponds to the refin/refout parameters of CRC RevEng,
        // hence Msb=false and Lsb=true
        enum Bitorder
        {
            Msb = false,
            Lsb = true,
        };

        template<
            uint8_t width,
            Uint<width> polynomial,
            Uint<width> initial,
            Bitorder inorder,
            Bitorder outorder,
            Uint<width> xorout
        >
        struct Impl
        {
            static constexpr auto poly = (inorder == Msb) ? polynomial : bitswap<width>(polynomial);
            static constexpr Uint<width> msb = Uint<width>(1) << (width - 1);
            static constexpr Uint<width> lsb = 1;
            static constexpr Uint<width> mask = (msb << 1) - 1;
            Uint<width> checksum = (inorder == Msb) ? initial : bitswap<width>(initial);

            // The simple bitwise CRC implementation
            template<std::ranges::input_range Range>
            constexpr Impl & bitwise(Range && range)
            {
                for (uint8_t byte : range)
                {
                    if constexpr (inorder == Msb)
                    {
                        // Shift byte into the MSbit position
                        checksum ^= Uint<width>(byte) << (width - 8);
                        for (unsigned i = 0; i < 8; ++i)
                        {
                            // This is the core of CRC, XORing the
                            // polynomial if the bit is set
                            checksum = (checksum << 1) ^ ((checksum & msb) ? poly : 0);
                        }
                    }
                    else
                    {
                        checksum ^= byte;
                        for (unsigned i = 0; i < 8; ++i)
                        {
                            checksum = (checksum >> 1) ^ ((checksum & lsb) ? poly : 0);
                        }
                    }
                }
                return *this;
            }

            // The table-based implementation using the Sarwate algorithm
            static constexpr std::array<Uint<width>, 256> table = []
            {
                std::array<Uint<width>, 256> table = {0};
                // The table is computed as
                //
                // table[0] = 0
                // table[1] = CRC of 0x01 (LSbit) or 0x80 (MSbit)
                // table[2] = CRC of 0x02 (LSbit) or 0x40 (MSbit)
                // table[3] = CRC of 0x03 (LSbit) or 0x60 (MSbit)
                // ...
                //
                // We could just use the bitwise function to do this,
                // but note that table[3] = table[1] ^ table[2], so we
                // can just compute i = 0x01, 0x02, 0x04, 0x08, 0x10,
                // and so on; and for each i, fill in the rest using
                // the previously computed values.
                if constexpr (inorder == Msb)
                {
                    Uint<width> checksum = msb;
                    for (unsigned i = 0x01; i != 0x100; i = i << 1)
                    {
                        checksum = (checksum << 1) ^ ((checksum & msb) ? poly : 0);
                        // For e.g. i=0x04 we have j=0 to j=3, giving
                        // us i+j=4 to i+j=7
                        for (unsigned j = 0; j < i; ++j)
                        {
                            table[i + j] = checksum ^ table[j];
                        }
                    }
                }
                else
                {
                    Uint<width> checksum = lsb;
                    for (unsigned i = 0x80; i != 0x00; i = i >> 1)
                    {
                        checksum = (checksum >> 1) ^ ((checksum & lsb) ? poly : 0);
                        // This is like the 1,...,i above, except using
                        // the bits to the right of i
                        unsigned previousBit = i << 1;
                        for (unsigned j = 0; j < 256; j += previousBit)
                        {
                            table[i + j] = checksum ^ table[j];
                        }
                    }
                }
                return table;
            }();

            template<std::ranges::input_range Range>
            constexpr Impl & tabled(Range && range)
            {
                for (uint8_t byte : range)
                {
                    if constexpr (inorder == Msb)
                    {
                        // XORing a byte at a time, using the checksum XOR
                        // byte as the key into the table which has cached
                        // the `bit-set ? poly : 0` for a byte at a time
                        const auto leftmost = static_cast<uint8_t>(checksum >> (width - 8));
                        const uint8_t lookup = byte ^ leftmost;
                        checksum = (checksum << 8) ^ table[lookup];
                    }
                    else
                    {
                        const auto rightmost = static_cast<uint8_t>(checksum);
                        const uint8_t lookup = byte ^ rightmost;
                        checksum = (checksum >> 8) ^ table[lookup];
                    }
                }
                return *this;
            }

            constexpr operator Uint<width>() const
            {
                const Uint<width> value = checksum ^ xorout;
                const Uint<width> out = (inorder == outorder) ? value : bitswap<width>(value);
                // For non-exact widths (i.e. not 8/16/32/64), mask off
                // the overflow
                return out & mask;
            }
        };
    };

    using enum Detail::Bitorder;

    // With huge thanks to https://reveng.sourceforge.io/crc-catalogue/

    // CRC-32
    using Bzip2 = Detail::Impl<32, 0x04C11DB7, ~0u, Msb, Msb, ~0u>;
    // The classic one used in Ethernet and zlib
    using Pkzip = Detail::Impl<32, 0x04C11DB7, ~0u, Lsb, Lsb, ~0u>;
    using Cksum = Detail::Impl<32, 0x04C11DB7, 0, Msb, Msb, ~0u>;

    // CRC-64
    using Ecma182 = Detail::Impl<64, 0x42F0E1EBA9EA3693, 0, Msb, Msb, 0>;
    // Often misidentified as "ECMA" apparently
    using Crc64Xz = Detail::Impl<64, 0x42F0E1EBA9EA3693, ~0ull, Lsb, Lsb, ~0ull>;
};
