#pragma once

#include "cpprc/crc.hpp"

#include <concepts>
#include <cstdint>

using namespace std::literals;
using namespace Crc::Detail;

static_assert(std::same_as<Uint<0>,  uint8_t>);
static_assert(std::same_as<Uint<8>,  uint8_t>);
static_assert(std::same_as<Uint<9>,  uint16_t>);
static_assert(std::same_as<Uint<16>, uint16_t>);
static_assert(std::same_as<Uint<17>, uint32_t>);
static_assert(std::same_as<Uint<32>, uint32_t>);
static_assert(std::same_as<Uint<33>, uint64_t>);
static_assert(std::same_as<Uint<64>, uint64_t>);

static_assert(bitswapMask<16>(8) == 0x00FF);
static_assert(bitswapMask<16>(4) == 0x0F0F);
static_assert(bitswapMask<16>(2) == 0x3333);
static_assert(bitswapMask<16>(1) == 0x5555);

static_assert(bitswap<4>(0x1) == 0x8);
static_assert(bitswap<8>(0x01) == 0x80);
static_assert(bitswap<12>(0x001) == 0x800);
static_assert(bitswap<32>(0x0000'0001) == 0x8000'0000);
static_assert(bitswap<32>(0x1234'5678) == 0x1E6A'2C48);

// CRC-32
static_assert(Crc::Bzip2{}.bitwise("123456789"sv) == 0xFC891918);
static_assert(Crc::Pkzip{}.bitwise("123456789"sv) == 0xCBF43926);
static_assert(Crc::Cksum{}.bitwise("123456789"sv) == 0x765e7680);

static_assert(Crc::Bzip2{}.tabled("123456789"sv) == 0xFC891918);
static_assert(Crc::Pkzip{}.tabled("123456789"sv) == 0xCBF43926);

// CRC-64
static_assert(Crc::Ecma182{}.bitwise("123456789"sv) == 0x6C40DF5F0B497347);
static_assert(Crc::Crc64Xz{}.bitwise("123456789"sv) == 0x995DC9BBDF1939FA);

static_assert(Crc::Ecma182{}.tabled("123456789"sv) == 0x6C40DF5F0B497347);
static_assert(Crc::Crc64Xz{}.tabled("123456789"sv) == 0x995DC9BBDF1939FA);
