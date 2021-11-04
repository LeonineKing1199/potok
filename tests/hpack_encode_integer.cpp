#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include <potok/hpack/common.hpp>
#include <potok/hpack/encode.hpp>

#include <boost/system/error_code.hpp>

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
  auto const size            = potok::hpack::get_num_required_octets(value, num_prefix_bits);

  auto e = potok::hpack::integer_encoder(value, num_prefix_bits);

  auto storage = std::vector<u8>(size);
  auto buf     = boost::asio::dynamic_vector_buffer(storage);
  auto ec      = boost::system::error_code();

  REQUIRE(1 == e(buf.data(0, buf.size()), ec));
  REQUIRE(storage[0] == u8{0b00001010});
  REQUIRE(!ec);
}

TEST_CASE("C.1.2. Example 2: Encoding 1337 Using a 5-Bit Prefix")
{
  // https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.2
  //
  auto const num_prefix_bits = 5;
  auto const value           = 1337;
  auto const size            = potok::hpack::get_num_required_octets(value, num_prefix_bits);

  auto e = potok::hpack::integer_encoder(value, num_prefix_bits);

  auto storage = std::vector<u8>(size);
  auto buf     = boost::asio::dynamic_vector_buffer(storage);
  auto ec      = boost::system::error_code();

  REQUIRE(3 == e(buf.data(0, buf.size()), ec));
  CHECK(storage[0] == 0b00011111);
  CHECK(storage[1] == 0b10011010);
  CHECK(storage[2] == 0b00001010);
  REQUIRE(!ec);
}

TEST_CASE("C.1.3. Example 3: Encoding 42 Starting at an Octet Boundary")
{
  // https://datatracker.ietf.org/doc/html/rfc7541#appendix-C.1.3
  //
  auto const num_prefix_bits = 8;
  auto const value           = 42;
  auto const size            = potok::hpack::get_num_required_octets(value, num_prefix_bits);

  auto e = potok::hpack::integer_encoder(value, num_prefix_bits);

  auto storage = std::vector<u8>(size);
  auto buf     = boost::asio::dynamic_vector_buffer(storage);
  auto ec      = boost::system::error_code();

  REQUIRE(1 == e(buf.data(0, buf.size()), ec));
  REQUIRE(storage[0] == 0b00101010);
  REQUIRE(!ec);
}

TEST_CASE("Encoding into an empty buffer sequence should return an error code and 0 bytes written")
{
  auto const num_prefix_bits = 5;
  auto const value           = 10;

  auto e = potok::hpack::integer_encoder(value, num_prefix_bits);

  auto storage = std::vector<u8>();
  REQUIRE(storage.empty());

  auto buf = boost::asio::dynamic_vector_buffer(storage);
  REQUIRE(buf.size() == 0);

  auto mut_buf_seq = buf.data(0, buf.size());
  auto ec          = boost::system::error_code();

  auto const n = e(mut_buf_seq, ec);
  REQUIRE(n == 0);
  REQUIRE(ec == potok::hpack::error::needs_more);
}

TEST_CASE("Encoding into a buffer sequence with insufficient capacity should write some bytes and return an error code")
{
  auto const num_prefix_bits = 5;
  auto const value           = 1337;

  auto e = potok::hpack::integer_encoder(value, num_prefix_bits);

  auto storage     = std::vector<u8>(2);
  auto buf         = boost::asio::dynamic_vector_buffer(storage);
  auto mut_buf_seq = buf.data(0, buf.size());
  auto ec          = boost::system::error_code();

  auto n = e(mut_buf_seq, ec);
  REQUIRE(n == 2);
  REQUIRE(ec == potok::hpack::error::needs_more);

  storage.resize(3);

  mut_buf_seq = buf.data(n, buf.size() - n);
  ec          = {};
  n           = e(mut_buf_seq, ec);

  REQUIRE(n == 1);
  REQUIRE(!ec);

  CHECK(storage[0] == 0b00011111);
  CHECK(storage[1] == 0b10011010);
  CHECK(storage[2] == 0b00001010);
}

TEST_CASE("Re-using a completed encoder should be harmless, i.e. write no bytes, generate no errors")
{
  auto const num_prefix_bits = 5;
  auto const value           = 10;
  auto const size            = potok::hpack::get_num_required_octets(value, num_prefix_bits);

  auto e = potok::hpack::integer_encoder(value, num_prefix_bits);

  auto storage = std::vector<u8>(size);
  auto buf     = boost::asio::dynamic_vector_buffer(storage);
  auto ec      = boost::system::error_code();

  REQUIRE(1 == e(buf.data(0, buf.size()), ec));
  REQUIRE(storage[0] == u8{0b00001010});
  REQUIRE(!ec);

  REQUIRE(0 == e(buf.data(0, buf.size()), ec));
  REQUIRE(storage[0] == u8{0b00001010});
  REQUIRE(!ec);
}

TEST_CASE("Non-prefix bits in the leading octet should not be affected by encoding an integer")
{
  for (unsigned i = 0; i < 8; ++i) {
    auto const x               = 1337;
    auto const num_prefix_bits = 1 + i;
    auto const size            = potok::hpack::get_num_required_octets(x, num_prefix_bits);

    auto s = std::vector<u8>(size);
    s[0]   = 0xff;

    auto buf = boost::asio::dynamic_vector_buffer(s);

    auto e  = potok::hpack::integer_encoder(x, num_prefix_bits);
    auto ec = boost::system::error_code();

    auto const n = e(buf.data(0, buf.size()), ec);
    REQUIRE(n > 0);
    REQUIRE(!ec);

    auto const mask = static_cast<u8>(~((u64{1} << num_prefix_bits) - 1));
    REQUIRE((s[0] & mask) == mask);
  }

  for (unsigned i = 0; i < 8; ++i) {
    auto const x               = 1337;
    auto const num_prefix_bits = 1 + i;
    auto const size            = potok::hpack::get_num_required_octets(x, num_prefix_bits);

    auto s = std::vector<u8>(size);
    s[0]   = 0x00;

    auto buf = boost::asio::dynamic_vector_buffer(s);
    auto e   = potok::hpack::integer_encoder(x, num_prefix_bits);
    auto ec  = boost::system::error_code();

    auto const n = e(buf.data(0, buf.size()), ec);
    REQUIRE(n > 0);
    REQUIRE(!ec);

    auto const mask = static_cast<u8>(~((u64{1} << num_prefix_bits) - 1));
    REQUIRE((s[0] & mask) == 0);
  }
}

TEST_CASE("If a value can fit entirely in the prefix bits, it must be written solely in the leading octet")
{
  for (unsigned i = 0; i < 8; ++i) {
    auto s = std::vector<u8>{0x00};

    auto buf = boost::asio::dynamic_vector_buffer(s);

    auto const num_prefix_bits  = 1 + i;
    auto const max_prefix_value = potok::hpack::get_max_prefix_value(num_prefix_bits);

    for (unsigned x = 0; x < max_prefix_value; ++x) {
      auto e  = potok::hpack::integer_encoder(x, num_prefix_bits);
      auto ec = boost::system::error_code();

      auto const n = e(buf.data(0, buf.size()), ec);
      REQUIRE(n == 1);
      REQUIRE(s[0] == x);
      REQUIRE(!ec);
    }
  }
}

TEST_CASE("We should be able to encode the largest u64 possible")
{
  // value to encode is: 18446744073709551615
  // prefix bits = 5
  // max prefix value = 31
  //
  // using the iterative algo, we first encode 31 (1)
  //
  // then see I trasnform as:
  // I = 18446744073709551584 (2)
  // I /= 128 => 144115188075855871 (3)
  // I /= 128 => 1125899906842623 (4)
  // I /= 128 => 8796093022207 (5)
  // I /= 128 => 68719476735 (6)
  // I /= 128 => 536870911 (7)
  // I /= 128 => 4194303 (8)
  // I /= 128 => 32767 (9)
  // I /= 128 => 255 (10)
  // I /= 128 => 1
  //
  // I finally <= 128
  // encode final value, 1 (11)
  //

  auto const num_prefix_bits = 5;
  auto const value           = std::numeric_limits<u64>::max();
  auto const size            = potok::hpack::get_num_required_octets(value, num_prefix_bits);

  auto s   = std::vector<u8>(size);
  auto buf = boost::asio::dynamic_vector_buffer(s);

  auto const max = potok::hpack::get_max_prefix_value(num_prefix_bits);

  auto e  = potok::hpack::integer_encoder(value, num_prefix_bits);
  auto ec = boost::system::error_code();

  auto const n = e(buf.data(0, buf.size()), ec);
  REQUIRE(!ec);

  REQUIRE(n == 11);
  REQUIRE(s.size() == n);

  REQUIRE(s[0] == max);

  REQUIRE((s[1] & 128) == 128);
  REQUIRE(s[1] == ((255 - max) | 128));

  for (unsigned i = 2; i < 10; ++i) {
    REQUIRE(s[i] == 255);
  }

  REQUIRE(s[10] == 1);

  REQUIRE(s == std::vector<u8>{max, ((255 - max) | 128), 255, 255, 255, 255, 255, 255, 255, 255, 1});
}

TEST_CASE("We should be able to encode multiple integers in a row, using the same buffer")
{
  auto const num_prefix_bits = 5;

  auto storage = std::vector<u8>{0x00};
  auto buf     = boost::asio::dynamic_vector_buffer(storage);
  auto pos     = u64{0};

  auto ec = boost::system::error_code();

  {
    auto const value = 10;

    auto e = potok::hpack::integer_encoder(value, num_prefix_bits);

    auto const n = e(buf.data(pos, buf.size() - pos), ec);
    REQUIRE(n == 1);
    REQUIRE(storage[pos] == value);
    REQUIRE(!ec);

    pos += n;
  }

  {
    auto const value = 1337;

    auto e = potok::hpack::integer_encoder(value, num_prefix_bits);
    auto n = e(buf.data(pos, buf.size() - pos), ec);

    REQUIRE(n == 0);
    REQUIRE(ec == potok::hpack::error::needs_more);

    buf.grow(3);

    n = e(buf.data(pos, buf.size() - pos), ec);
    REQUIRE(n == 3);
    REQUIRE(!ec);
    REQUIRE(storage[pos + 0] == 0b00011111);
    REQUIRE(storage[pos + 1] == 0b10011010);
    REQUIRE(storage[pos + 2] == 0b00001010);

    pos += n;
  }

  {
    auto const value = 1337;

    buf.grow(2);

    auto e = potok::hpack::integer_encoder(value, num_prefix_bits);
    auto n = e(buf.data(pos, buf.size() - pos), ec);

    REQUIRE(n == 2);
    REQUIRE(ec == potok::hpack::error::needs_more);

    buf.grow(1);
    n = e(buf.data(pos + n, buf.size() - pos - n), ec);

    REQUIRE(n == 1);
    REQUIRE(!ec);

    REQUIRE(storage[pos + 0] == 0b00011111);
    REQUIRE(storage[pos + 1] == 0b10011010);
    REQUIRE(storage[pos + 2] == 0b00001010);

    pos += 3;
  }

  {
    auto const value = 10;

    buf.grow(1);

    auto       e = potok::hpack::integer_encoder(value, num_prefix_bits);
    auto const n = e(buf.data(pos, buf.size() - pos), ec);

    REQUIRE(n == 1);
    REQUIRE(!ec);
    REQUIRE(storage[pos] == value);

    pos += n;
  }

  REQUIRE(storage.size() == pos);

  REQUIRE(storage == std::vector<u8>{0b00001010, 0b00011111, 0b10011010, 0b00001010, 0b00011111, 0b10011010, 0b00001010,
                                     0b00001010});
}
