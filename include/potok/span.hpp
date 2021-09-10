#ifndef POTOK_SPAN_HPP_
#define POTOK_SPAN_HPP_

#include <boost/span/span.hpp>

namespace potok {

template <class T, std::size_t E = boost::spans::dynamic_extent>
using span = boost::spans::span<T, E>;
}

#endif    // POTOK_SPAN_HPP_
