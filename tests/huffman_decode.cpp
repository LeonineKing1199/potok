#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include <potok/span.hpp>
#include <potok/stdint.hpp>

#include <boost/assert.hpp>

#include <memory_resource>
#include <optional>
#include <vector>

namespace potok {
namespace huffman {

// u32 const codewords[257] = {
//     0x1ff8,     0x7fffd8,  0xfffffe2,  0xfffffe3,  0xfffffe4, 0xfffffe5, 0xfffffe6, 0xfffffe7, 0xfffffe8, 0xffffea,
//     0x3ffffffc, 0xfffffe9, 0xfffffea,  0x3ffffffd, 0xfffffeb, 0xfffffec, 0xfffffed, 0xfffffee, 0xfffffef, 0xffffff0,
//     0xffffff1,  0xffffff2, 0x3ffffffe, 0xffffff3,  0xffffff4, 0xffffff5, 0xffffff6, 0xffffff7, 0xffffff8, 0xffffff9,
//     0xffffffa,  0xffffffb, 0x14,       0x3f8,      0x3f9,     0xffa,     0x1ff9,    0x15,      0xf8,      0x7fa,
//     0x3fa,      0x3fb,     0xf9,       0x7fb,      0xfa,      0x16,      0x17,      0x18,      0x0,       0x1,
//     0x2,        0x19,      0x1a,       0x1b,       0x1c,      0x1d,      0x1e,      0x1f,      0x5c,      0xfb,
//     0x7ffc,     0x20,      0xffb,      0x3fc,      0x1ffa,    0x21,      0x5d,      0x5e,      0x5f,      0x60,
//     0x61,       0x62,      0x63,       0x64,       0x65,      0x66,      0x67,      0x68,      0x69,      0x6a,
//     0x6b,       0x6c,      0x6d,       0x6e,       0x6f,      0x70,      0x71,      0x72,      0xfc,      0x73,
//     0xfd,       0x1ffb,    0x7fff0,    0x1ffc,     0x3ffc,    0x22,      0x7ffd,    0x3,       0x23,      0x4,
//     0x24,       0x5,       0x25,       0x26,       0x27,      0x6,       0x74,      0x75,      0x28,      0x29,
//     0x2a,       0x7,       0x2b,       0x76,       0x2c,      0x8,       0x9,       0x2d,      0x77,      0x78,
//     0x79,       0x7a,      0x7b,       0x7ffe,     0x7fc,     0x3ffd,    0x1ffd,    0xffffffc, 0xfffe6,   0x3fffd2,
//     0xfffe7,    0xfffe8,   0x3fffd3,   0x3fffd4,   0x3fffd5,  0x7fffd9,  0x3fffd6,  0x7fffda,  0x7fffdb,  0x7fffdc,
//     0x7fffdd,   0x7fffde,  0xffffeb,   0x7fffdf,   0xffffec,  0xffffed,  0x3fffd7,  0x7fffe0,  0xffffee,  0x7fffe1,
//     0x7fffe2,   0x7fffe3,  0x7fffe4,   0x1fffdc,   0x3fffd8,  0x7fffe5,  0x3fffd9,  0x7fffe6,  0x7fffe7,  0xffffef,
//     0x3fffda,   0x1fffdd,  0xfffe9,    0x3fffdb,   0x3fffdc,  0x7fffe8,  0x7fffe9,  0x1fffde,  0x7fffea,  0x3fffdd,
//     0x3fffde,   0xfffff0,  0x1fffdf,   0x3fffdf,   0x7fffeb,  0x7fffec,  0x1fffe0,  0x1fffe1,  0x3fffe0,  0x1fffe2,
//     0x7fffed,   0x3fffe1,  0x7fffee,   0x7fffef,   0xfffea,   0x3fffe2,  0x3fffe3,  0x3fffe4,  0x7ffff0,  0x3fffe5,
//     0x3fffe6,   0x7ffff1,  0x3ffffe0,  0x3ffffe1,  0xfffeb,   0x7fff1,   0x3fffe7,  0x7ffff2,  0x3fffe8,  0x1ffffec,
//     0x3ffffe2,  0x3ffffe3, 0x3ffffe4,  0x7ffffde,  0x7ffffdf, 0x3ffffe5, 0xfffff1,  0x1ffffed, 0x7fff2,   0x1fffe3,
//     0x3ffffe6,  0x7ffffe0, 0x7ffffe1,  0x3ffffe7,  0x7ffffe2, 0xfffff2,  0x1fffe4,  0x1fffe5,  0x3ffffe8, 0x3ffffe9,
//     0xffffffd,  0x7ffffe3, 0x7ffffe4,  0x7ffffe5,  0xfffec,   0xfffff3,  0xfffed,   0x1fffe6,  0x3fffe9,  0x1fffe7,
//     0x1fffe8,   0x7ffff3,  0x3fffea,   0x3fffeb,   0x1ffffee, 0x1ffffef, 0xfffff4,  0xfffff5,  0x3ffffea, 0x7ffff4,
//     0x3ffffeb,  0x7ffffe6, 0x3ffffec,  0x3ffffed,  0x7ffffe7, 0x7ffffe8, 0x7ffffe9, 0x7ffffea, 0x7ffffeb, 0xffffffe,
//     0x7ffffec,  0x7ffffed, 0x7ffffee,  0x7ffffef,  0x7fffff0, 0x3ffffee, 0x3fffffff};

// u32 const bit_lens[257] = {
//     13, 23, 28, 28, 28, 28, 28, 28, 28, 24, 30, 28, 28, 30, 28, 28, 28, 28, 28, 28, 28, 28, 30, 28, 28, 28, 28, 28,
//     28, 28, 28, 28, 6,  10, 10, 12, 13, 6,  8,  11, 10, 10, 8,  11, 8,  6,  6,  6,  5,  5,  5,  6,  6,  6,  6,  6, 6,
//     6, 7,  8,  15, 6,  12, 10, 13, 6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
//     7, 7,  8,  7,  8,  13, 19, 13, 14, 6,  15, 5,  6,  5,  6,  5,  6,  6,  6,  5,  7,  7,  6,  6,  6,  5,  6,  7,  6,
//     5, 5,  6,  7,  7,  7,  7,  7,  15, 11, 14, 13, 28, 20, 22, 20, 20, 22, 22, 22, 23, 22, 23, 23, 23, 23, 23, 24,
//     23, 24, 24, 22, 23, 24, 23, 23, 23, 23, 21, 22, 23, 22, 23, 23, 24, 22, 21, 20, 22, 22, 23, 23, 21, 23, 22, 22,
//     24, 21, 22, 23, 23, 21, 21, 22, 21, 23, 22, 23, 23, 20, 22, 22, 22, 23, 22, 22, 23, 26, 26, 20, 19, 22, 23, 22,
//     25, 26, 26, 26, 27, 27, 26, 24, 25, 19, 21, 26, 27, 27, 26, 27, 24, 21, 21, 26, 26, 28, 27, 27, 27, 20, 24, 20,
//     21, 22, 21, 21, 23, 22, 22, 25, 25, 24, 24, 26, 23, 26, 27, 26, 26, 27, 27, 27, 27, 27, 28, 27, 27, 27, 27, 27,
//     26, 30,
// };

// u32 const table[257][2] = {
//     {0x1ff8, 13},    {0x7fffd8, 23},   {0xfffffe2, 28}, {0xfffffe3, 28}, {0xfffffe4, 28},  {0xfffffe5, 28},
//     {0xfffffe6, 28}, {0xfffffe7, 28},  {0xfffffe8, 28}, {0xffffea, 24},  {0x3ffffffc, 30}, {0xfffffe9, 28},
//     {0xfffffea, 28}, {0x3ffffffd, 30}, {0xfffffeb, 28}, {0xfffffec, 28}, {0xfffffed, 28},  {0xfffffee, 28},
//     {0xfffffef, 28}, {0xffffff0, 28},  {0xffffff1, 28}, {0xffffff2, 28}, {0x3ffffffe, 30}, {0xffffff3, 28},
//     {0xffffff4, 28}, {0xffffff5, 28},  {0xffffff6, 28}, {0xffffff7, 28}, {0xffffff8, 28},  {0xffffff9, 28},
//     {0xffffffa, 28}, {0xffffffb, 28},  {0x14, 6},       {0x3f8, 10},     {0x3f9, 10},      {0xffa, 12},
//     {0x1ff9, 13},    {0x15, 6},        {0xf8, 8},       {0x7fa, 11},     {0x3fa, 10},      {0x3fb, 10},
//     {0xf9, 8},       {0x7fb, 11},      {0xfa, 8},       {0x16, 6},       {0x17, 6},        {0x18, 6},
//     {0x0, 5},        {0x1, 5},         {0x2, 5},        {0x19, 6},       {0x1a, 6},        {0x1b, 6},
//     {0x1c, 6},       {0x1d, 6},        {0x1e, 6},       {0x1f, 6},       {0x5c, 7},        {0xfb, 8},
//     {0x7ffc, 15},    {0x20, 6},        {0xffb, 12},     {0x3fc, 10},     {0x1ffa, 13},     {0x21, 6},
//     {0x5d, 7},       {0x5e, 7},        {0x5f, 7},       {0x60, 7},       {0x61, 7},        {0x62, 7},
//     {0x63, 7},       {0x64, 7},        {0x65, 7},       {0x66, 7},       {0x67, 7},        {0x68, 7},
//     {0x69, 7},       {0x6a, 7},        {0x6b, 7},       {0x6c, 7},       {0x6d, 7},        {0x6e, 7},
//     {0x6f, 7},       {0x70, 7},        {0x71, 7},       {0x72, 7},       {0xfc, 8},        {0x73, 7},
//     {0xfd, 8},       {0x1ffb, 13},     {0x7fff0, 19},   {0x1ffc, 13},    {0x3ffc, 14},     {0x22, 6},
//     {0x7ffd, 15},    {0x3, 5},         {0x23, 6},       {0x4, 5},        {0x24, 6},        {0x5, 5},
//     {0x25, 6},       {0x26, 6},        {0x27, 6},       {0x6, 5},        {0x74, 7},        {0x75, 7},
//     {0x28, 6},       {0x29, 6},        {0x2a, 6},       {0x7, 5},        {0x2b, 6},        {0x76, 7},
//     {0x2c, 6},       {0x8, 5},         {0x9, 5},        {0x2d, 6},       {0x77, 7},        {0x78, 7},
//     {0x79, 7},       {0x7a, 7},        {0x7b, 7},       {0x7ffe, 15},    {0x7fc, 11},      {0x3ffd, 14},
//     {0x1ffd, 13},    {0xffffffc, 28},  {0xfffe6, 20},   {0x3fffd2, 22},  {0xfffe7, 20},    {0xfffe8, 20},
//     {0x3fffd3, 22},  {0x3fffd4, 22},   {0x3fffd5, 22},  {0x7fffd9, 23},  {0x3fffd6, 22},   {0x7fffda, 23},
//     {0x7fffdb, 23},  {0x7fffdc, 23},   {0x7fffdd, 23},  {0x7fffde, 23},  {0xffffeb, 24},   {0x7fffdf, 23},
//     {0xffffec, 24},  {0xffffed, 24},   {0x3fffd7, 22},  {0x7fffe0, 23},  {0xffffee, 24},   {0x7fffe1, 23},
//     {0x7fffe2, 23},  {0x7fffe3, 23},   {0x7fffe4, 23},  {0x1fffdc, 21},  {0x3fffd8, 22},   {0x7fffe5, 23},
//     {0x3fffd9, 22},  {0x7fffe6, 23},   {0x7fffe7, 23},  {0xffffef, 24},  {0x3fffda, 22},   {0x1fffdd, 21},
//     {0xfffe9, 20},   {0x3fffdb, 22},   {0x3fffdc, 22},  {0x7fffe8, 23},  {0x7fffe9, 23},   {0x1fffde, 21},
//     {0x7fffea, 23},  {0x3fffdd, 22},   {0x3fffde, 22},  {0xfffff0, 24},  {0x1fffdf, 21},   {0x3fffdf, 22},
//     {0x7fffeb, 23},  {0x7fffec, 23},   {0x1fffe0, 21},  {0x1fffe1, 21},  {0x3fffe0, 22},   {0x1fffe2, 21},
//     {0x7fffed, 23},  {0x3fffe1, 22},   {0x7fffee, 23},  {0x7fffef, 23},  {0xfffea, 20},    {0x3fffe2, 22},
//     {0x3fffe3, 22},  {0x3fffe4, 22},   {0x7ffff0, 23},  {0x3fffe5, 22},  {0x3fffe6, 22},   {0x7ffff1, 23},
//     {0x3ffffe0, 26}, {0x3ffffe1, 26},  {0xfffeb, 20},   {0x7fff1, 19},   {0x3fffe7, 22},   {0x7ffff2, 23},
//     {0x3fffe8, 22},  {0x1ffffec, 25},  {0x3ffffe2, 26}, {0x3ffffe3, 26}, {0x3ffffe4, 26},  {0x7ffffde, 27},
//     {0x7ffffdf, 27}, {0x3ffffe5, 26},  {0xfffff1, 24},  {0x1ffffed, 25}, {0x7fff2, 19},    {0x1fffe3, 21},
//     {0x3ffffe6, 26}, {0x7ffffe0, 27},  {0x7ffffe1, 27}, {0x3ffffe7, 26}, {0x7ffffe2, 27},  {0xfffff2, 24},
//     {0x1fffe4, 21},  {0x1fffe5, 21},   {0x3ffffe8, 26}, {0x3ffffe9, 26}, {0xffffffd, 28},  {0x7ffffe3, 27},
//     {0x7ffffe4, 27}, {0x7ffffe5, 27},  {0xfffec, 20},   {0xfffff3, 24},  {0xfffed, 20},    {0x1fffe6, 21},
//     {0x3fffe9, 22},  {0x1fffe7, 21},   {0x1fffe8, 21},  {0x7ffff3, 23},  {0x3fffea, 22},   {0x3fffeb, 22},
//     {0x1ffffee, 25}, {0x1ffffef, 25},  {0xfffff4, 24},  {0xfffff5, 24},  {0x3ffffea, 26},  {0x7ffff4, 23},
//     {0x3ffffeb, 26}, {0x7ffffe6, 27},  {0x3ffffec, 26}, {0x3ffffed, 26}, {0x7ffffe7, 27},  {0x7ffffe8, 27},
//     {0x7ffffe9, 27}, {0x7ffffea, 27},  {0x7ffffeb, 27}, {0xffffffe, 28}, {0x7ffffec, 27},  {0x7ffffed, 27},
//     {0x7ffffee, 27}, {0x7ffffef, 27},  {0x7fffff0, 27}, {0x3ffffee, 26}, {0x3fffffff, 30},
// };

// struct decoder {
//   std::pmr::vector<u32> codewords_;
//   std::pmr::vector<u32> bit_lens_;
// };

}    // namespace huffman
}    // namespace potok

using namespace potok::ints;

auto codeword_match(u32 const* codewords, u32 const* masks, usize const len, u32 const v) -> std::optional<usize>
{
  for (usize i = 0; i < len; ++i) {
    if ((masks[i] & v) == codewords[i]) { return {i}; }
  }

  return {};
}

TEST_CASE("Codeword matching!")
{
  //    'i' (105)  |00110                                         6  [ 5]
  //    'o' (111)  |00111                                         7  [ 5]

  u32 const masks[]     = {u32{0xf8} << 24, u32{0xf8} << 24};
  u32 const codewords[] = {0x06 << 27, 0x07 << 27};
  u32 const bit_lens[]  = {5, 5};

  u32 v = u32{0b00110'00111'00110'00111'00110'00111'11};

  REQUIRE((masks[0] & codewords[0]) == codewords[0]);
  REQUIRE((masks[0] & v) == codewords[0]);

  auto maybe_idx = std::optional<usize>();
  auto idx       = usize{0};

  {
    maybe_idx = codeword_match(codewords, masks, 2, v);
    REQUIRE(maybe_idx);

    idx = *maybe_idx;
    REQUIRE(idx == 0);

    v <<= bit_lens[idx];
  }

  {
    maybe_idx = codeword_match(codewords, masks, 2, v);
    REQUIRE(maybe_idx);

    idx = *maybe_idx;
    REQUIRE(idx == 1);

    v <<= bit_lens[idx];
  }

  {
    maybe_idx = codeword_match(codewords, masks, 2, v);
    REQUIRE(maybe_idx);

    idx = *maybe_idx;
    REQUIRE(idx == 0);

    v <<= bit_lens[idx];
  }

  {
    maybe_idx = codeword_match(codewords, masks, 2, v);
    REQUIRE(maybe_idx);

    idx = *maybe_idx;
    REQUIRE(idx == 1);

    v <<= bit_lens[idx];
  }

  {
    maybe_idx = codeword_match(codewords, masks, 2, v);
    REQUIRE(maybe_idx);

    idx = *maybe_idx;
    REQUIRE(idx == 0);

    v <<= bit_lens[idx];
  }

  {
    maybe_idx = codeword_match(codewords, masks, 2, v);
    REQUIRE(maybe_idx);

    idx = *maybe_idx;
    REQUIRE(idx == 1);

    v <<= bit_lens[idx];
  }

  {
    maybe_idx = codeword_match(codewords, masks, 2, v);
    REQUIRE(!maybe_idx);
  }
}
