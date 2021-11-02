#ifndef POTOK_HPACK_COMMON_HPP_
#define POTOK_HPACK_COMMON_HPP_

#include <potok/stdint.hpp>

namespace potok {
namespace hpack {

// to get the required number of octets to encode an integer, we first check if the supplied value fits inside the
// prefix
//
// assuming it does, we know we can immediately return a value of 1
//
// if it does not, we know from the pseudocode algorithm that we need a number of octets N such that:
//     128^N >= (x - max_prefix_value)
// is true, minimizing for N
//
// solving the above exponential would be mathematically equivalent to:
//     floor(log(x - max_prefix_value) / log(128))
// so we use an inline while-loop as that's likely faster than a `std::log` or `log` call
//
constexpr auto get_num_required_octets(u64 const x, u8 const num_prefix_bits) -> u32
{
  u64 const max_prefix_value = (u64{1} << num_prefix_bits) - 1;

  if (x < max_prefix_value) { return 1; }

  auto I = x - max_prefix_value;

  auto count = 1;
  while (I >= 128) {
    ++count;
    I /= 128;
  }
  ++count;
  return count;
}

constexpr auto get_max_prefix_value(u8 const num_prefix_bits)
{
  return (u64{1} << num_prefix_bits) - 1;
}

}    // namespace hpack
}    // namespace potok

#endif    // POTOK_HPACK_COMMON_HPP_