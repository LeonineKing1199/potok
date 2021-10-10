#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include <potok/span.hpp>

#include <boost/assert.hpp>

#include <cstdint>

namespace potok {

using u8  = std::uint8_t;
using u64 = std::uint64_t;

namespace hpack {

constexpr void encode_integer(u8 const prefix, u64 value, potok::span<u8> buf)
{
  BOOST_ASSERT(prefix >= 1 && prefix <= 8);

  auto* out = buf.data();

  auto const max = u64{(u64{1} << (prefix)) - 1};
  if (value < max) {
    *out = reinterpret_cast<u8 const*>(&value)[0] << (8 - prefix);
    return;
  }

  *out = u8{255} << (8 - prefix);
  ++out;

  value -= max;

  while (value >= 128) {
    auto const v = (value % 128 + 128);
    auto const e = v | (1 << 7);

    *out = e;
    ++out;

    value /= 128;
  }

  auto const v = (value % 128 + 128);
  auto const e = v & (0b01111111);

  *out = e;
}

}    // namespace hpack
}    // namespace potok

using potok::u8;

// https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.1
//
TEST_CASE("C.1.1. Example 1: Encoding 10 Using a 5-Bit Prefix")
{
  auto const prefix = 5;
  auto const value  = 10;

  REQUIRE(value == 0b01010);

  auto storage = u8{0};
  auto buf     = potok::span<u8>(&storage, 1);

  potok::hpack::encode_integer(prefix, value, buf);

  storage = storage & (0b11111000);

  REQUIRE(storage == u8{0b01010000});
  REQUIRE(storage == u8{80});
}

// https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.2
//
TEST_CASE("C.1.2. Example 2: Encoding 1337 Using a 5-Bit Prefix")
{
  auto const prefix = 5;
  auto const value  = 1337;

  u8 storage[] = {0, 0, 0};

  potok::hpack::encode_integer(prefix, value, storage);

  CHECK(storage[0] == 0b11111000);
  CHECK(storage[1] == 0b10011010);
  CHECK(storage[2] == 0b00001010);
}

// https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.3
//
TEST_CASE("C.1.3. Example 3: Encoding 42 Starting at an Octet Boundary")
{
  auto const prefix = 8;
  auto const value  = 42;

  auto storage = u8{0};
  auto buf     = potok::span<u8>(&storage, 1);

  potok::hpack::encode_integer(prefix, value, buf);

  REQUIRE(storage == 0b00101010);
}
