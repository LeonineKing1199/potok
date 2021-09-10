#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <potok/span.hpp>

#include <boost/assert.hpp>

#include <cstdint>

namespace potok {

using u8  = std::uint8_t;
using u64 = std::uint64_t;

namespace hpack {

constexpr void encode_integer(u8 const prefix, u64 const value, potok::span<u8> buf)
{
  BOOST_ASSERT(prefix >= 1 && prefix <= 8);

  auto const max = u64{(u64{1} << (prefix)) - 1};
  if (value < max) {
    buf[0] = reinterpret_cast<u8 const*>(&value)[0] << (8 - prefix);
  }
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

  auto storage = u8{0};
  auto buf     = potok::span<u8>(&storage, 1);

  potok::hpack::encode_integer(prefix, value, buf);

  REQUIRE(storage == u8{0b01010000});
  REQUIRE(storage == u8{80});
}
