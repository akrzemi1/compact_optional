// Copyright (C) 2015, Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef AK_TOOLBOX_COMPACT_OPTIONAL_HEADER_GUARD_
#define AK_TOOLBOX_COMPACT_OPTIONAL_HEADER_GUARD_

#include <cassert>
#include <utility>

#if defined AK_TOOLBOX_NO_ARVANCED_CXX11
#  define AK_TOOLBOX_NOEXCEPT
#  define AK_TOOLBOX_CONSTEXPR
#  define AK_TOOLBOX_EXPLICIT_CONV
#  define AK_TOOLBOX_NOEXCEPT_AS(E)
#else
#  define AK_TOOLBOX_NOEXCEPT noexcept 
#  define AK_TOOLBOX_CONSTEXPR constexpr 
#  define AK_TOOLBOX_EXPLICIT_CONV explicit 
#  define AK_TOOLBOX_NOEXCEPT_AS(E) noexcept(noexcept(E))
#endif


namespace ak_toolbox {
namespace compact_optional_ns {

struct default_tag{};

template <typename T, T Val>
struct empty_scalar_value
{
  static AK_TOOLBOX_CONSTEXPR T empty_value() { return Val; }
  static AK_TOOLBOX_CONSTEXPR bool is_empty_value(T v) { return v == Val; }
};

namespace detail_ {

template <typename T, typename N>
class compact_optional_base
{
  T value_;

public:

  AK_TOOLBOX_CONSTEXPR compact_optional_base() AK_TOOLBOX_NOEXCEPT_AS(T(N::empty_value()))
    : value_(N::empty_value()) {}
    
  AK_TOOLBOX_CONSTEXPR compact_optional_base(const T& v)
    : value_(v) {}
    
  AK_TOOLBOX_CONSTEXPR compact_optional_base(T&& v)
    : value_(std::move(v)) {}
    
  AK_TOOLBOX_CONSTEXPR bool has_value() const { return !N::is_empty_value(value_); }
  
  AK_TOOLBOX_CONSTEXPR const T& value() const { return assert(has_value()), value_; }
};

} // namespace detail_

template <typename T, typename N, typename /* tag */ = default_tag>
class compact_optional : public detail_::compact_optional_base<T, N>
{
  typedef detail_::compact_optional_base<T, N> super;
  
public:

  AK_TOOLBOX_CONSTEXPR compact_optional() AK_TOOLBOX_NOEXCEPT_AS(T(N::empty_value()))
    : super() {}
    
  AK_TOOLBOX_CONSTEXPR compact_optional(const T& v)
    : super(v) {}
    
  AK_TOOLBOX_CONSTEXPR compact_optional(T&& v)
    : super(std::move(v)) {}

};

} // namespace compact_optional_ns

using compact_optional_ns::compact_optional;
using compact_optional_ns::empty_scalar_value;

} // namespace ak_toolbox

#endif //AK_TOOLBOX_COMPACT_OPTIONAL_HEADER_GUARD_
