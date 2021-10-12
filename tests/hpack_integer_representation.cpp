#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include <potok/span.hpp>

#include <boost/assert.hpp>

#include <cstdint>

namespace potok {

using u8  = std::uint8_t;
using u64 = std::uint64_t;

namespace hpack {

constexpr void encode_integer(u8 const num_prefix_bits, u64 const x, potok::span<u8> buf)
{
  BOOST_ASSERT(num_prefix_bits >= 1 && num_prefix_bits <= 8);

  auto* out = buf.data();

  u64 const max_prefix_value = (u64{1} << num_prefix_bits) - 1;

  if (x < max_prefix_value) {
    *out += x;
    return;
  }

  *out += max_prefix_value;
  ++out;

  auto I = x - max_prefix_value;
  while (I >= 128) {
    auto const v = (I % 128 + 128);

    *out = v;
    ++out;

    I /= 128;
  }

  *out = I;
}

}    // namespace hpack
}    // namespace potok

using potok::u8;

// https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.1
//
TEST_CASE("C.1.1. Example 1: Encoding 10 Using a 5-Bit Prefix")
{
  auto const num_prefix_bits = 5;
  auto const value           = 10;

  auto storage = u8{0b11100000};

  auto buf = potok::span<u8>(&storage, 1);

  potok::hpack::encode_integer(num_prefix_bits, value, buf);

  REQUIRE(storage == u8{0b11101010});
}

// https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.2
//
TEST_CASE("C.1.2. Example 2: Encoding 1337 Using a 5-Bit Prefix")
{
  auto const num_prefix_bits = 5;
  auto const value           = 1337;

  u8 storage[] = {0, 0, 0};

  potok::hpack::encode_integer(num_prefix_bits, value, storage);

  CHECK(storage[0] == 0b00011111);
  CHECK(storage[1] == 0b10011010);
  CHECK(storage[2] == 0b00001010);
}

// https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.3
//
TEST_CASE("C.1.3. Example 3: Encoding 42 Starting at an Octet Boundary")
{
  auto const num_prefix_bits = 8;
  auto const value           = 42;

  auto storage = u8{0};
  auto buf     = potok::span<u8>(&storage, 1);

  potok::hpack::encode_integer(num_prefix_bits, value, buf);

  REQUIRE(storage == 0b00101010);
}
