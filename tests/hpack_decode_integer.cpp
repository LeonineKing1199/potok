#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include <potok/hpack/common.hpp>
#include <potok/hpack/decode.hpp>

#include <boost/asio/buffer.hpp>

using namespace potok::ints;

TEST_CASE("C.1.1. Example 1: Encoding 10 Using a 5-Bit Prefix")
{
  // https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.1
  //
  auto const num_prefix_bits = 5;

  auto d = potok::hpack::integer_decoder(num_prefix_bits);

  auto       storage = std::vector<u8>{0b00001010};
  auto const buf     = boost::asio::dynamic_vector_buffer(storage);
  auto       ec      = boost::system::error_code();
  auto       v       = u64{0};

  REQUIRE(1 == d(buf.data(0, buf.size()), v, ec));
  REQUIRE(v == u64{0b00001010});
  REQUIRE(v == 10);
  REQUIRE(!ec);
}

TEST_CASE("C.1.2. Example 2: Encoding 1337 Using a 5-Bit Prefix")
{
  // https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.2
  //
  auto const num_prefix_bits = 5;

  auto d = potok::hpack::integer_decoder(num_prefix_bits);

  auto       storage = std::vector<u8>{0b00011111, 0b10011010, 0b00001010};
  auto const buf     = boost::asio::dynamic_vector_buffer(storage);
  auto       ec      = boost::system::error_code();
  auto       v       = u64{0};

  CHECK(3 == d(buf.data(0, buf.size()), v, ec));
  CHECK(v == 1337);
  CHECK(!ec);
}

TEST_CASE("C.1.3. Example 3: Encoding 42 Starting at an Octet Boundary")
{
  // https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.3
  //
  auto const num_prefix_bits = 8;

  auto d = potok::hpack::integer_decoder(num_prefix_bits);

  auto       storage = std::vector<u8>{42};
  auto const buf     = boost::asio::dynamic_vector_buffer(storage);
  auto       ec      = boost::system::error_code();
  auto       v       = u64{0};

  CHECK(1 == d(buf.data(0, buf.size()), v, ec));
  CHECK(v == 42);
  CHECK(!ec);
}

TEST_CASE("We should support incremental parsing of an integer")
{
  auto const num_prefix_bits = 5;

  {
    auto d = potok::hpack::integer_decoder(num_prefix_bits);

    auto       storage = std::vector<u8>{0b00011111};
    auto const buf     = boost::asio::dynamic_vector_buffer(storage);
    auto       ec      = boost::system::error_code();
    auto       v       = u64{0};

    CHECK(1 == d(buf.data(0, buf.size()), v, ec));
    CHECK(v == 0);
    REQUIRE(ec == potok::hpack::error::needs_more);

    storage.clear();
    storage = std::vector<u8>{0b10011010, 0b00001010};

    CHECK(2 == d(buf.data(0, buf.size()), v, ec));
    CHECK(v == 1337);
    REQUIRE(!ec);
  }

  {
    auto d = potok::hpack::integer_decoder(num_prefix_bits);

    auto       storage = std::vector<u8>{0b00011111, 0b10011010};
    auto const buf     = boost::asio::dynamic_vector_buffer(storage);
    auto       ec      = boost::system::error_code();
    auto       v       = u64{0};

    CHECK(2 == d(buf.data(0, buf.size()), v, ec));
    CHECK(v == 0);
    REQUIRE(ec == potok::hpack::error::needs_more);

    storage.clear();
    storage = std::vector<u8>{0b00001010};

    CHECK(1 == d(buf.data(0, buf.size()), v, ec));
    CHECK(v == 1337);
    REQUIRE(!ec);
  }

  {
    auto d = potok::hpack::integer_decoder(num_prefix_bits);

    auto       storage = std::vector<u8>{};
    auto const buf     = boost::asio::dynamic_vector_buffer(storage);
    auto       ec      = boost::system::error_code();
    auto       v       = u64{0};

    CHECK(0 == d(buf.data(0, buf.size()), v, ec));
    CHECK(v == 0);
    REQUIRE(ec == potok::hpack::error::needs_more);

    storage.clear();
    storage = std::vector<u8>{0b00011111, 0b10011010, 0b00001010};

    CHECK(3 == d(buf.data(0, buf.size()), v, ec));
    CHECK(v == 1337);
    REQUIRE(!ec);
  }
}

TEST_CASE("We should catch possible cases of u64 overflow")
{
  auto const num_prefix_bits = 4;

  auto d = potok::hpack::integer_decoder(num_prefix_bits);

  auto storage = std::vector<u8>{0b00001111, 255 - 0b1111, 255, 255, 255, 255, 255, 255, 255,
                                 255,        255,          255, 255, 255, 255, 255, 1};

  auto const buf = boost::asio::dynamic_vector_buffer(storage);
  auto       ec  = boost::system::error_code();
  auto       v   = u64{0};

  CHECK(11 == d(buf.data(0, buf.size()), v, ec));
  CHECK(v == 0);
  REQUIRE(ec == potok::hpack::error::too_large);
}

TEST_CASE("We should be able to decode the largest u64 correctly")
{
  auto const num_prefix_bits = 5;
  auto const max             = potok::hpack::get_max_prefix_value(num_prefix_bits);

  auto d = potok::hpack::integer_decoder(num_prefix_bits);

  auto       storage = std::vector<u8>{max, ((255 - max) | 128), 255, 255, 255, 255, 255, 255, 255, 255, 1};
  auto const buf     = boost::asio::dynamic_vector_buffer(storage);
  auto       ec      = boost::system::error_code();
  auto       v       = u64{0};

  CHECK(11 == d(buf.data(0, buf.size()), v, ec));
  CHECK(v == std::numeric_limits<u64>::max());
  CHECK(!ec);
}

TEST_CASE("We should handle empty octet sequences correctly")
{
  auto const num_prefix_bits = 5;

  auto d = potok::hpack::integer_decoder(num_prefix_bits);

  auto       storage = std::vector<u8>();
  auto const buf     = boost::asio::dynamic_vector_buffer(storage);
  auto       ec      = boost::system::error_code();
  auto       v       = u64{0};

  CHECK(0 == d(buf.data(0, buf.size()), v, ec));
  CHECK(v == 0);
  CHECK(ec == potok::hpack::error::needs_more);
}
