#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include <potok/hpack/common.hpp>
#include <potok/stdint.hpp>

static_assert(potok::hpack::get_num_required_octets(14, 5) == 1);
static_assert(potok::hpack::get_num_required_octets(35, 5) == 2);
static_assert(potok::hpack::get_num_required_octets(std::numeric_limits<potok::u64>::max(), 1) == 11);