#ifndef POTOK_HPACK_ERROR_HPP_
#define POTOK_HPACK_ERROR_HPP_

#include <boost/system/error_category.hpp>
#include <boost/system/error_code.hpp>

#include <string>

namespace potok {
namespace hpack {

enum class error : int { needs_more };

struct hpack_error_category final : public boost::system::error_category {
  auto name() const noexcept -> char const* override
  {
    return "potok.hpack";
  }

  auto message(int const err_cond) const -> std::string override
  {
    switch (static_cast<error>(err_cond)) {
      case error::needs_more:
        return "needs more";

      default:
        return "potok.hpack error";
    }
  }

  auto default_error_condition(int const err_cond) const noexcept -> boost::system::error_condition override
  {
    return boost::system::error_condition(err_cond, *this);
  }

  bool equivalent(int const error_cond, boost::system::error_condition const& cond) const noexcept override
  {
    return cond.value() == error_cond && &cond.category() == this;
  }

  bool equivalent(boost::system::error_code const& err, int const err_cond) const noexcept override
  {
    return err.value() == err_cond && &err.category() == this;
  }
};

inline auto make_error_code(error const err) -> boost::system::error_code
{
  static hpack_error_category const category;
  return boost::system::error_code(static_cast<int>(err), category);
}

}    // namespace hpack
}    // namespace potok

template <>
struct boost::system::is_error_code_enum<potok::hpack::error> {
  static bool const value = true;
};

#endif    // POTOK_HPACK_ERROR_HPP_