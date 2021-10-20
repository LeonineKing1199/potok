#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include <potok/span.hpp>

#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>

#include <stdexcept>
#include <cstdint>
#include <limits>

namespace potok {

using u8  = std::uint8_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

namespace hpack {

// from the hpack rfc (https://datatracker.ietf.org/doc/html/rfc7541#section-5.1):
//  Pseudocode to represent an integer I is as follows:
//  if I < 2^N - 1, encode I on N bits
//  else
//      encode (2^N - 1) on N bits
//      I = I - (2^N - 1)
//      while I >= 128
//           encode (I % 128 + 128) on 8 bits
//           I = I / 128
//      encode I on 8 bits

// to get the required number of octets to encode an integer, we first check if the supplied value fits inside the
// prefix
//
// assuming it does, we know we can immediately return a value of 1
//
// if it does not, we know from the pseudocode algorithm that we need a number of octets N such that:
//     128^N >= (x - max_prefix_value)
// is true, minimizing for N
//
// solving the above exponential would be mathematically equivalent to:
//     floor(log(x - max_prefix_value) / log(128))
// so we use an inline while-loop as that's likely faster than a `std::log` or `log` call
//
constexpr auto get_num_required_octets(u64 const x, u8 const num_prefix_bits) -> u32
{
  u64 const max_prefix_value = (u64{1} << num_prefix_bits) - 1;

  if (x < max_prefix_value) {
    return 1;
  }

  auto I = x - max_prefix_value;

  auto count = 1;
  while (I >= 128) {
    ++count;
    I /= 128;
  }
  ++count;
  return count;
}

static_assert(get_num_required_octets(14, 5) == 1);
static_assert(get_num_required_octets(35, 5) == 2);
static_assert(get_num_required_octets(std::numeric_limits<u64>::max(), 1) == 11);

constexpr auto encode_integer(u8 const num_prefix_bits, u64 const x, potok::span<u8> buf) -> span<u8>
{
  BOOST_ASSERT(num_prefix_bits >= 1 && num_prefix_bits <= 8);
  BOOST_ASSERT(!buf.empty());

  auto const num_required_octets = get_num_required_octets(x, num_prefix_bits);
  if (buf.size() < num_required_octets) {
    BOOST_THROW_EXCEPTION(std::runtime_error("buf does not have enough capacity"));
  }

  u64 const max_prefix_value = (u64{1} << num_prefix_bits) - 1;

  auto* out = buf.data();

  if (x < max_prefix_value) {
    if (num_prefix_bits == 8) {
      *out++ = x;
    }
    else {
      *out++ += x;
    }

    return buf.subspan(num_required_octets);
  }

  if (num_prefix_bits == 8) {
    *out++ = max_prefix_value;
  }
  else {
    *out++ += max_prefix_value;
  }

  auto I = x - max_prefix_value;
  while (I >= 128) {
    auto const v = (I % 128 + 128);

    *out++ = v;
    I /= 128;
  }

  *out++ = I;

  return buf.subspan(num_required_octets);
}

}    // namespace hpack
}    // namespace potok

using potok::u64;
using potok::u8;

// https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.1
//
TEST_CASE("C.1.1. Example 1: Encoding 10 Using a 5-Bit Prefix")
{
  auto const num_prefix_bits = 5;
  auto const value           = 10;

  {
    auto storage = u8{0b11100000};

    auto buf = potok::span<u8>(&storage, 1);

    potok::hpack::encode_integer(num_prefix_bits, value, buf);

    REQUIRE(storage == u8{0b11101010});
  }

  {
    auto storage = u8{0b00000000};

    auto buf = potok::span<u8>(&storage, 1);

    potok::hpack::encode_integer(num_prefix_bits, value, buf);

    REQUIRE(storage == u8{0b00001010});
  }
}

// https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.2
//
TEST_CASE("C.1.2. Example 2: Encoding 1337 Using a 5-Bit Prefix")
{
  auto const num_prefix_bits = 5;
  auto const value           = 1337;

  {
    u8 storage[] = {0, 0, 0};

    potok::hpack::encode_integer(num_prefix_bits, value, storage);

    CHECK(storage[0] == 0b00011111);
    CHECK(storage[1] == 0b10011010);
    CHECK(storage[2] == 0b00001010);
  }

  {
    u8 storage[] = {0b11100000, 1, 7};

    potok::hpack::encode_integer(num_prefix_bits, value, storage);

    CHECK(storage[0] == 0b11111111);
    CHECK(storage[1] == 0b10011010);
    CHECK(storage[2] == 0b00001010);
  }
}

// https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.3
//
TEST_CASE("C.1.3. Example 3: Encoding 42 Starting at an Octet Boundary")
{
  {
    auto const num_prefix_bits = 8;
    auto const value           = 42;

    auto storage = u8{0b10011001};
    auto buf     = potok::span<u8>(&storage, 1);

    potok::hpack::encode_integer(num_prefix_bits, value, buf);

    REQUIRE(storage == 0b00101010);
  }

  {
    auto const num_prefix_bits = 8;
    auto const value           = 325;

    auto storage = u64{0b10011001};
    auto buf     = potok::span<u8>(reinterpret_cast<u8*>(&storage), 4);

    potok::hpack::encode_integer(num_prefix_bits, value, buf);

    REQUIRE(buf[0] == 0b11111111);
    REQUIRE(buf[1] == 0b01000110);
  }
}

TEST_CASE("Not enough space")
{
  auto const num_prefix_bits = 4;
  auto const value           = u64{123423423};

  u8 storage[] = {0x00};
  REQUIRE_THROWS(potok::hpack::encode_integer(num_prefix_bits, value, storage));
}
