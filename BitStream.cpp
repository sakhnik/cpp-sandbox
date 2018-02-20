#include "doctest.h"
#include <cstdint>

namespace BitStream {

// Read 'count' number of bits from a memory starting at bit 'offset'
// The compiler will easily optimizize the implementation.
template <unsigned offset, unsigned count>
inline uint32_t Read(const uint8_t *src, uint32_t accum = 0)
{
    // Base of recursion
    if (count == 0)
        return accum;

    // Look at the source, how many bits there left in the current byte
    const uint8_t *cur_src = src + offset / 8;
    constexpr unsigned bits_left = 8 - offset % 8;
    const uint8_t cur_data = *cur_src << (8 - bits_left);

    // How many bits we need and can write now
    constexpr unsigned bits_to_use = bits_left < count ? bits_left : count;

    // Write the desired bits to the accumulator
    accum <<= bits_to_use;
    uint8_t mask = (1 << bits_to_use) - 1;
    constexpr unsigned off = 8 - bits_to_use;
    accum |= (cur_data & (mask << off)) >> off;

    // Tail-recurse into the rest of required bits
    return Read<offset + bits_to_use, count - bits_to_use>(src, accum);
}

// Write given value to the memory starting at 'offset' bit spanning 'count' bits
template <unsigned offset, unsigned count>
inline void Write(uint8_t *dst, uint32_t value)
{
    // Recursion base
    if (count == 0)
        return;

    // How many bits there left in the current target byte
    uint8_t *cur_dst = dst + offset / 8;
    constexpr unsigned bits_left = 8 - offset % 8;

    // How many bits are necessary and the mask for that count of bits
    constexpr unsigned bits_to_use = bits_left < count ? bits_left : count;
    const uint8_t mask = (1 << bits_to_use) - 1;

    // The desired range of bits from the source value
    constexpr unsigned value_off = count - bits_to_use;
    const uint8_t value_bits = (value >> value_off) & mask;

    // Writing the bits to the destination
    uint8_t cur_data = *cur_dst;
    constexpr unsigned off = bits_left - bits_to_use;
    cur_data &= ~(mask << off);
    cur_data |= value_bits << off;
    *cur_dst = cur_data;

    // Tail recursion to the rest of data
    return Write<offset + bits_to_use, count - bits_to_use>(dst, value);
}

} //namespace BitStream;

TEST_CASE("read")
{
    const uint8_t data[] = {0x89, 0xab, 0xcd, 0xef};

    CHECK(BitStream::Read<0,1>(data) == 0x1);
    CHECK(BitStream::Read<1,11>(data) == 0x9a);
    CHECK(BitStream::Read<12,12>(data) == 0xbcd);
}

TEST_CASE("write")
{
    uint8_t data[4] = {};

    BitStream::Write<0,1>(data, 0x1);
    CHECK(data[0] == 0x80);
    CHECK(data[1] == 0x00);
    CHECK(data[2] == 0x00);
    CHECK(data[3] == 0x00);

    BitStream::Write<1,11>(data, 0x9a);
    CHECK(data[0] == 0x89);
    CHECK(data[1] == 0xa0);
    CHECK(data[2] == 0x00);
    CHECK(data[3] == 0x00);

    BitStream::Write<12,12>(data, 0xbcd);
    CHECK(data[0] == 0x89);
    CHECK(data[1] == 0xab);
    CHECK(data[2] == 0xcd);
    CHECK(data[3] == 0x00);
}
