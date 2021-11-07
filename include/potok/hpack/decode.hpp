#ifndef POTOK_HPACK_DECODE_HPP_
#define POTOK_HPACK_DECODE_HPP_

#include <potok/hpack/common.hpp>
#include <potok/hpack/error.hpp>

#include <potok/stdint.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>

#include <boost/system/error_code.hpp>

#include <boost/assert.hpp>

namespace potok {
namespace hpack {

//  decode I from the next N bits
//  if I < 2^N - 1, return I
//  else
//      M = 0
//      repeat
//          B = next octet
//          I = I + (B & 127) * 2^M
//          M = M + 7
//      while B & 128 == 128
//      return I

struct integer_decoder {
  enum class state { first, continuation, final, done };

  u64      v_               = 0;
  u64      M_               = 0;
  state    state_           = state::first;
  u8 const num_prefix_bits_ = 0;

  integer_decoder()                       = delete;
  integer_decoder(integer_decoder const&) = default;
  integer_decoder(integer_decoder&&)      = default;

  integer_decoder(u8 const num_prefix_bits)
      : num_prefix_bits_{num_prefix_bits}
  {
    BOOST_ASSERT(num_prefix_bits_ >= 1 && num_prefix_bits_ <= 8);
  }

  template <class ConstBufferSequence>
  auto operator()(ConstBufferSequence        const_buf_seq,    //
                  u64&                       v,                //
                  boost::system::error_code& ec) -> usize
  {
    constexpr auto const u64_max = u64{0xffffffff};

    auto bytes_read = usize{0};
    if (state_ == state::done) { return bytes_read; }

    auto pos = boost::asio::buffers_begin(const_buf_seq);
    auto end = boost::asio::buffers_end(const_buf_seq);

    if (pos == end) {
      ec = error::needs_more;
      return bytes_read;
    }

    switch (state_) {
      case state::first: {
        auto const max_prefix_value = get_max_prefix_value(num_prefix_bits_);

        v_ = *pos++ & max_prefix_value;
        ++bytes_read;

        if (v_ < max_prefix_value) {
          state_ = state::done;
          break;
        }

        state_ = state::continuation;
      }

      case state::continuation: {
        auto B = u64{128};
        for (; (pos != end) && ((B & 128) == 128); ++pos, ++bytes_read, M_ += 7) {
          B = *pos;

          auto const tmp = (B & u64{127}) * (u64{1} << M_);
          if (v_ > u64_max - tmp) {
            ec = error::too_large;
            break;
          }

          v_ += tmp;
        }

        if (!ec && ((B & 128) == 128)) {
          BOOST_ASSERT(pos == end);
          ec = error::needs_more;
          break;
        }

        state_ = state::done;
      }

      case state::done:
      default:
        break;
    }

    v = v_;
    return bytes_read;
  }
};

template <class ConstBufferSequence>
auto decode_integer(u8 const            num_prefix_bits,    //
                    ConstBufferSequence const_buf_seq,      //
                    u64&                out,                //
                    boost::system::error_code&) -> usize
{
  BOOST_ASSERT(num_prefix_bits >= 1 && num_prefix_bits <= 8);

  auto pos = boost::asio::buffers_begin(const_buf_seq);
  auto end = boost::asio::buffers_end(const_buf_seq);

  u64 const max_prefix_value = get_max_prefix_value(num_prefix_bits);

  auto num_parsed = u64{0};

  auto I = *pos & max_prefix_value;
  ++num_parsed;

  if (I < max_prefix_value) {
    out = I;
    return num_parsed;
  }

  auto M = u64{0};
  ++pos;

  auto B = u64{128};
  while ((pos != end) && ((B & 128) == 128)) {
    B = *pos++;
    I = I + (B & u64{127}) * (u64{1} << M);
    M += 7;
    ++num_parsed;
  }

  out = I;

  return num_parsed;
}

}    // namespace hpack
}    // namespace potok

#endif    // POTOK_HPACK_ENCODE_HPP_