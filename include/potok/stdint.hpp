#ifndef POTOK_STDINT_HPP_
#define POTOK_STDINT_HPP_

#include <cstdint>

namespace potok {

namespace ints {

using u8    = std::uint8_t;
using u32   = std::uint32_t;
using i32   = std::int32_t;
using u64   = std::uint64_t;
using usize = std::size_t;

}    // namespace ints

using namespace ::potok::ints;

}    // namespace potok

#endif    // POTOK_STDINT_HPP_