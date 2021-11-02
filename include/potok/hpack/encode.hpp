#ifndef POTOK_HPACK_ENCODE_HPP_
#define POTOK_HPACK_ENCODE_HPP_

#include <potok/hpack/common.hpp>
#include <potok/stdint.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>

#include <boost/assert.hpp>

#include <boost/throw_exception.hpp>

#include <stdexcept>

namespace potok {
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

template <class DynamicBuffer_V2>
auto encode_integer(u8 const         num_prefix_bits,    //
                    u64 const        x,                  //
                    DynamicBuffer_V2 buf,                //
                    usize const      offset) -> usize
{
  BOOST_ASSERT(num_prefix_bits >= 1 && num_prefix_bits <= 8);

  auto const num_required_octets = get_num_required_octets(x, num_prefix_bits);

  auto mutable_buf_seq = buf.data(offset, num_required_octets);
  if (boost::asio::buffer_size(mutable_buf_seq) < num_required_octets) { buf.grow(num_required_octets); }

  mutable_buf_seq = buf.data(offset, num_required_octets);
  if (boost::asio::buffer_size(mutable_buf_seq) < num_required_octets) {
    boost::throw_exception(std::runtime_error("Unable to extend underlying DynamicBuffer_V2"));
  }

  u64 const max_prefix_value = get_max_prefix_value(num_prefix_bits);

  auto out = boost::asio::buffers_begin(mutable_buf_seq);

  auto bytes_written = usize{0};

  *out &= ~max_prefix_value;

  if (x < max_prefix_value) {
    *out++ |= x;
    ++bytes_written;
    return bytes_written;
  }

  *out++ |= max_prefix_value;
  ++bytes_written;

  auto I = x - max_prefix_value;
  while (I >= 128) {
    auto const v = (I % 128 + 128);

    *out++ = v;
    ++bytes_written;
    I /= 128;
  }

  *out++ = I;
  ++bytes_written;

  return bytes_written;
}

}    // namespace hpack
}    // namespace potok

#endif    // POTOK_HPACK_ENCODE_HPP_