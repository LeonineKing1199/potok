#ifndef POTOK_HPACK_DECODE_HPP_
#define POTOK_HPACK_DECODE_HPP_

#include <potok/hpack/common.hpp>
#include <potok/stdint.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>

#include <boost/system/error_code.hpp>

#include <boost/assert.hpp>

namespace potok {
namespace hpack {

template <class ConstBufferSequence>
auto decode_integer(u8 const num_prefix_bits, ConstBufferSequence const_buf_seq, boost::system::error_code&) -> u64
{
  BOOST_ASSERT(num_prefix_bits >= 1 && num_prefix_bits <= 8);

  auto pos = boost::asio::buffers_begin(const_buf_seq);
  auto end = boost::asio::buffers_end(const_buf_seq);

  u64 const max_prefix_value = get_max_prefix_value(num_prefix_bits);

  auto I = *pos & max_prefix_value;
  if (I < max_prefix_value) { return I; }

  auto M = u64{0};
  ++pos;

  auto B = u8{128};
  while ((pos != end) && ((B & 128) == 128)) {
    B = *pos++;
    I = I + (B & 127) * (1 << M);
    M += 7;
  }

  return I;
}

}    // namespace hpack
}    // namespace potok

#endif    // POTOK_HPACK_ENCODE_HPP_