#include "test.hpp"

#include "cpprc/crc.hpp"

#include <cstdint>
#include <span>

// It's all static_asserts!
// But we have a wrapper for Python to test against zlib

extern "C"
{
    uint32_t crc32Pkzip(const uint8_t * data, size_t len)
    {
        return Crc::Pkzip{}.tabled(std::span<const uint8_t>{data, len});
    }
}
