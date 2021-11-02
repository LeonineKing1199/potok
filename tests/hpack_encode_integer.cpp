#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include <potok/hpack/common.hpp>
#include <potok/hpack/encode.hpp>

#include <limits>
#include <vector>

using potok::u64;
using potok::u8;

TEST_CASE("C.1.1. Example 1: Encoding 10 Using a 5-Bit Prefix")
{
  // https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.1
  //
  auto const num_prefix_bits = 5;
  auto const value           = 10;

  auto storage = std::vector<u8>{0x00};
  auto buf     = boost::asio::dynamic_vector_buffer(storage);
  auto pos     = 0;

  REQUIRE(1 == potok::hpack::encode_integer(num_prefix_bits, value, buf, pos));
  REQUIRE(storage[0] == u8{0b00001010});
}

TEST_CASE("C.1.2. Example 2: Encoding 1337 Using a 5-Bit Prefix")
{
  // https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.2
  //
  auto const num_prefix_bits = 5;
  auto const value           = 1337;

  auto storage = std::vector<u8>{};
  auto buf     = boost::asio::dynamic_vector_buffer(storage);
  auto pos     = 0;

  REQUIRE(3 == potok::hpack::encode_integer(num_prefix_bits, value, buf, pos));
  CHECK(storage[0] == 0b00011111);
  CHECK(storage[1] == 0b10011010);
  CHECK(storage[2] == 0b00001010);
}

TEST_CASE("C.1.3. Example 3: Encoding 42 Starting at an Octet Boundary")
{
  // https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.3
  //
  auto const num_prefix_bits = 8;
  auto const value           = 42;

  auto storage = std::vector<u8>{};
  auto buf     = boost::asio::dynamic_vector_buffer(storage);
  auto pos     = 0;

  REQUIRE(1 == potok::hpack::encode_integer(num_prefix_bits, value, buf, pos));
  REQUIRE(storage[0] == 0b00101010);
}

TEST_CASE("Encoding an integer shouldn't cause the underlying storage to reallocate if capacity is sufficient")
{
  auto const num_prefix_bits = 5;

  auto const x = 10;

  auto const  size    = u64{10};
  auto        storage = std::vector<u8>(size);
  auto const* ptr     = storage.data();

  REQUIRE(1 == potok::hpack::encode_integer(num_prefix_bits, x, boost::asio::dynamic_vector_buffer(storage), 0));
  REQUIRE(ptr == storage.data());
}

TEST_CASE("Encoding an integer should automatically reallocate for the caller if capacity is unsufficient")
{
  auto const num_prefix_bits = 5;

  auto const x = 1234;

  auto const  size    = u64{2};
  auto        storage = std::vector<u8>(size);
  auto const* ptr     = storage.data();

  REQUIRE(3 == potok::hpack::encode_integer(num_prefix_bits, x, boost::asio::dynamic_vector_buffer(storage), 0));
  REQUIRE(ptr != storage.data());
}

TEST_CASE("Non-prefix bits in the leading octet should not be affected by encoding an integer")
{
  for (unsigned i = 0; i < 8; ++i) {
    auto s = std::vector<u8>{0xff};

    auto const x               = 1337;
    auto const num_prefix_bits = 1 + i;
    auto const buf             = boost::asio::dynamic_vector_buffer(s);

    auto const n = potok::hpack::encode_integer(num_prefix_bits, x, buf, 0);
    REQUIRE(n > 0);

    auto const mask = static_cast<u8>(~((u64{1} << num_prefix_bits) - 1));
    REQUIRE((s[0] & mask) == mask);
  }

  for (unsigned i = 0; i < 8; ++i) {
    auto s = std::vector<u8>{0x00};

    auto const x               = 1337;
    auto const num_prefix_bits = 1 + i;
    auto const buf             = boost::asio::dynamic_vector_buffer(s);

    auto const n = potok::hpack::encode_integer(num_prefix_bits, x, buf, 0);
    REQUIRE(n > 0);

    auto const mask = static_cast<u8>(~((u64{1} << num_prefix_bits) - 1));
    REQUIRE((s[0] & mask) == 0);
  }
}

TEST_CASE("If a value can fit entirely in the prefix bits, it must be written solely in the leading octet")
{
  for (unsigned i = 0; i < 8; ++i) {
    auto s = std::vector<u8>{0x00};

    auto const buf = boost::asio::dynamic_vector_buffer(s);

    auto const num_prefix_bits  = 1 + i;
    auto const max_prefix_value = potok::hpack::get_max_prefix_value(num_prefix_bits);

    for (unsigned x = 0; x < max_prefix_value; ++x) {
      auto const n = potok::hpack::encode_integer(num_prefix_bits, x, buf, 0);
      REQUIRE(n == 1);
      REQUIRE(s[0] == x);
    }
  }
}

TEST_CASE("We should be able to encode the largest u64 possible")
{
  auto const num_prefix_bits = 5;
  auto const value           = std::numeric_limits<u64>::max();

  auto s   = std::vector<u8>();
  auto buf = boost::asio::dynamic_vector_buffer(s);

  auto const max = potok::hpack::get_max_prefix_value(num_prefix_bits);

  auto const n = potok::hpack::encode_integer(num_prefix_bits, value, buf, 0);
  REQUIRE(n == 11);
  REQUIRE(s.size() == n);

  REQUIRE(s[0] == max);

  REQUIRE((s[1] & 128) == 128);
  REQUIRE(s[1] == static_cast<u8>(~max));

  for (unsigned i = 2; i < 10; ++i) {
    REQUIRE(s[i] == 255);
  }

  REQUIRE(s[10] == 1);
}
