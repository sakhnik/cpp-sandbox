#include "doctest.h"
#include <cstdint>

namespace BitStream {
template <unsigned offset, unsigned count>
inline uint32_t Read(const uint8_t *src, uint32_t accum = 0)
{
    if (count == 0)
        return accum;

    const uint8_t *cur_src = src + offset / 8;
    constexpr unsigned bits_left = 8 - offset % 8;
    const uint8_t cur_data = *cur_src << (8 - bits_left);

    constexpr unsigned bits_to_use = bits_left < count ? bits_left : count;

    accum <<= bits_to_use;
    uint8_t mask = (1 << bits_to_use) - 1;
    constexpr unsigned off = 8 - bits_to_use;
    accum |= (cur_data & (mask << off)) >> off;

    return Read<offset + bits_to_use, count - bits_to_use>(src, accum);
}

template <unsigned offset, unsigned count>
inline void Write(uint8_t *dst, uint32_t value)
{
    if (count == 0)
        return;

    uint8_t *cur_dst = dst + offset / 8;
    constexpr unsigned bits_left = 8 - offset % 8;

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

    CHECK(BitStream::Read<0,1>(data) == 1);
    CHECK(BitStream::Read<1,11>(data) == 154);
    CHECK(BitStream::Read<12,12>(data) == 3021);
}

TEST_CASE("write")
{
    uint8_t data[4] = {};

    BitStream::Write<0,1>(data, 1);
    CHECK(data[0] == 0x80);
    CHECK(data[1] == 0x00);
    CHECK(data[2] == 0x00);
    CHECK(data[3] == 0x00);

    BitStream::Write<1,11>(data, 154);
    CHECK(data[0] == 0x89);
    CHECK(data[1] == 0xa0);
    CHECK(data[2] == 0x00);
    CHECK(data[3] == 0x00);

    BitStream::Write<12,12>(data, 3021);
    CHECK(data[0] == 0x89);
    CHECK(data[1] == 0xab);
    CHECK(data[2] == 0xcd);
    CHECK(data[3] == 0x00);
}
