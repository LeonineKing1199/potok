#ifndef POTOK_HPACK_ENCODE_HPP_
#define POTOK_HPACK_ENCODE_HPP_

#include <potok/hpack/common.hpp>
#include <potok/hpack/error.hpp>

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

struct integer_encoder {
  enum class state { first, continuation, final, done };

  u64      v_               = 0;
  state    state_           = state::first;
  u8 const num_prefix_bits_ = 0;

  integer_encoder()                       = delete;
  integer_encoder(integer_encoder const&) = default;
  integer_encoder(integer_encoder&&)      = default;

  integer_encoder(u64 const v, u8 const num_prefix_bits)
      : v_{v}
      , num_prefix_bits_{num_prefix_bits}
  {
  }

  template <class MutableBufferSequence>
  auto operator()(MutableBufferSequence      mutable_buf_seq,    //
                  boost::system::error_code& ec) -> usize
  {
    auto pos = boost::asio::buffers_begin(mutable_buf_seq);
    auto end = boost::asio::buffers_end(mutable_buf_seq);

    auto bytes_written = usize{0};

    if (pos == end) {
      ec = error::needs_more;
      return bytes_written;
    }

    u64 const max_prefix_value = get_max_prefix_value(num_prefix_bits_);

    switch (state_) {
      case state::first: {
        // clear the initial octet in preparation of writing the first N bits of the integer
        //
        *pos &= ~max_prefix_value;

        if (v_ < max_prefix_value) {
          *pos++ |= v_;
          ++bytes_written;

          state_ = state::done;

          break;
        }

        *pos++ |= max_prefix_value;
        ++bytes_written;

        state_ = state::continuation;

        v_ -= max_prefix_value;
      }

      case state::continuation: {
        for (; (pos != end) && (v_ >= 128); ++pos, ++bytes_written, v_ /= 128) {
          auto const v = (v_ % 128 + 128);

          *pos = v;
        }

        if (v_ >= 128) {
          ec     = error::needs_more;
          state_ = state::continuation;
          break;
        }

        if (pos == end) {
          ec     = error::needs_more;
          state_ = state::final;
          break;
        }
      }

      case state::final: {
        *pos++ = v_;
        ++bytes_written;

        state_ = state::done;
        break;
      }

      case state::done:
        break;
    }

    return bytes_written;
  }
};

}    // namespace hpack
}    // namespace potok

#endif    // POTOK_HPACK_ENCODE_HPP_