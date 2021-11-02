#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include <potok/hpack/decode.hpp>
#include <potok/hpack/encode.hpp>

#include <boost/asio/buffer.hpp>

#include <vector>

static_assert(potok::hpack::get_num_required_octets(14, 5) == 1);
static_assert(potok::hpack::get_num_required_octets(35, 5) == 2);
static_assert(potok::hpack::get_num_required_octets(std::numeric_limits<potok::u64>::max(), 1) == 11);

using potok::u64;
using potok::u8;

// https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.1
//
TEST_CASE("C.1.1. Example 1: Encoding 10 Using a 5-Bit Prefix")
{
  auto const num_prefix_bits = 5;
  auto const value           = 10;

  {
    auto storage = std::vector<u8>{0b11100000};
    auto buf     = boost::asio::dynamic_vector_buffer(storage);
    auto pos     = 0;

    REQUIRE(1 == potok::hpack::encode_integer(num_prefix_bits, value, buf, pos));
    REQUIRE(storage[0] == u8{0b11101010});

    auto ec = boost::system::error_code();
    REQUIRE(value == potok::hpack::decode_integer(num_prefix_bits, buf.data(0, buf.size()), ec));
  }

  {
    auto storage = std::vector<u8>{0b00011111};
    auto buf     = boost::asio::dynamic_vector_buffer(storage);
    auto pos     = 0;

    REQUIRE(1 == potok::hpack::encode_integer(num_prefix_bits, value, buf, pos));
    REQUIRE(storage[0] == u8{0b00001010});

    auto ec = boost::system::error_code();
    REQUIRE(value == potok::hpack::decode_integer(num_prefix_bits, buf.data(0, buf.size()), ec));
  }
}

// https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.2
//
TEST_CASE("C.1.2. Example 2: Encoding 1337 Using a 5-Bit Prefix")
{
  auto const num_prefix_bits = 5;
  auto const value           = 1337;

  {
    auto storage = std::vector<u8>{};
    auto buf     = boost::asio::dynamic_vector_buffer(storage);
    auto pos     = 0;

    REQUIRE(3 == potok::hpack::encode_integer(num_prefix_bits, value, buf, pos));
    CHECK(storage[0] == 0b00011111);
    CHECK(storage[1] == 0b10011010);
    CHECK(storage[2] == 0b00001010);

    auto ec = boost::system::error_code();
    REQUIRE(value == potok::hpack::decode_integer(num_prefix_bits, buf.data(0, buf.size()), ec));
  }

  {
    auto storage = std::vector<u8>{0b11100000, 1, 7};
    auto buf     = boost::asio::dynamic_vector_buffer(storage);
    auto pos     = 0;

    REQUIRE(3 == potok::hpack::encode_integer(num_prefix_bits, value, buf, pos));

    CHECK(storage[0] == 0b11111111);
    CHECK(storage[1] == 0b10011010);
    CHECK(storage[2] == 0b00001010);

    auto ec = boost::system::error_code();
    REQUIRE(value == potok::hpack::decode_integer(num_prefix_bits, buf.data(0, buf.size()), ec));
  }
}

// https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.3
//
TEST_CASE("C.1.3. Example 3: Encoding 42 Starting at an Octet Boundary")
{
  {
    auto const num_prefix_bits = 8;
    auto const value           = 42;

    auto storage = std::vector<u8>{0b10011001};
    auto buf     = boost::asio::dynamic_vector_buffer(storage);
    auto pos     = 0;

    REQUIRE(1 == potok::hpack::encode_integer(num_prefix_bits, value, buf, pos));
    REQUIRE(storage[0] == 0b00101010);

    auto ec = boost::system::error_code();
    REQUIRE(value == potok::hpack::decode_integer(num_prefix_bits, buf.data(0, buf.size()), ec));
  }

  {
    auto const num_prefix_bits = 8;
    auto const value           = 325;

    auto storage = std::vector<u8>{0b10011001};
    auto buf     = boost::asio::dynamic_vector_buffer(storage);
    auto pos     = 0;

    REQUIRE(2 == potok::hpack::encode_integer(num_prefix_bits, value, buf, pos));

    REQUIRE(storage[0] == 0b11111111);
    REQUIRE(storage[1] == 0b01000110);

    auto ec = boost::system::error_code();
    REQUIRE(value == potok::hpack::decode_integer(num_prefix_bits, buf.data(0, buf.size()), ec));
  }
}
