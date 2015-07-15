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

#if defined NDEBUG
# define AK_TOOLBOX_ASSERTED_EXPRESSION(CHECK, EXPR) (EXPR)
#else
# define AK_TOOLBOX_ASSERTED_EXPRESSION(CHECK, EXPR) ((CHECK) ? (EXPR) : (fail(#CHECK, __FILE__, __LINE__), (EXPR)))
  inline void fail(const char* expr, const char* file, unsigned line)
  {
  # if defined __clang__ || defined __GNU_LIBRARY__
    __assert(expr, file, line);
  # elif defined __GNUC__
    _assert(expr, file, line);
  # else
  #   error UNSUPPORTED COMPILER
  # endif
  }
#endif

namespace ak_toolbox {
namespace compact_optional_ns {

struct default_tag{};

template <typename T, typename NT = T, typename CREF = const T&>
struct compact_optional_type
{
  typedef T value_type;
  typedef NT storage_type;
  typedef CREF reference_type;
  
  static AK_TOOLBOX_CONSTEXPR const value_type& access_value(const storage_type& v) { return v; }
  static AK_TOOLBOX_CONSTEXPR const value_type& store_value(const value_type& v) { return v; }
  static AK_TOOLBOX_CONSTEXPR value_type&& store_value(value_type&& v) { return std::move(v); }
};

template <typename T, T Val>
struct empty_scalar_value : compact_optional_type<T>
{
  static AK_TOOLBOX_CONSTEXPR T empty_value() AK_TOOLBOX_NOEXCEPT { return Val; }
  static AK_TOOLBOX_CONSTEXPR bool is_empty_value(T v) { return v == Val; }
};

template <typename OT>
struct compact_optional_from_optional : compact_optional_type<typename OT::value_type, OT>
{
  typedef typename OT::value_type value_type;
  typedef OT storage_type;

  static OT empty_value() AK_TOOLBOX_NOEXCEPT { return OT(); }
  static bool is_empty_value(const OT& v) { return !v; }
  
  static const value_type& access_value(const storage_type& v) { return *v; }
  static storage_type store_value(const value_type& v) { return v; }
  static storage_type store_value(value_type&& v) { return std::move(v); }
};

struct compact_bool : compact_optional_type<bool, char, bool>
{
  static AK_TOOLBOX_CONSTEXPR char empty_value() AK_TOOLBOX_NOEXCEPT { return char(2); }
  static AK_TOOLBOX_CONSTEXPR bool is_empty_value(char v) { return v == 2; }
  
  static AK_TOOLBOX_CONSTEXPR bool access_value(const char& v) { return bool(v); }
  static AK_TOOLBOX_CONSTEXPR char store_value(const bool& v) { return v; }
};

namespace detail_ {

template <typename N>
class compact_optional_base
{
protected:
  typedef typename N::value_type value_type;
  typedef typename N::storage_type storage_type;
  typedef typename N::reference_type reference_type;
  
  storage_type value_;

public:
  AK_TOOLBOX_CONSTEXPR compact_optional_base() AK_TOOLBOX_NOEXCEPT_AS(storage_type(N::empty_value()))
    : value_(N::empty_value()) {}
    
  AK_TOOLBOX_CONSTEXPR compact_optional_base(const value_type& v)
    : value_(N::store_value(v)) {}
    
  AK_TOOLBOX_CONSTEXPR compact_optional_base(value_type&& v)
    : value_(N::store_value(std::move(v))) {}
    
  AK_TOOLBOX_CONSTEXPR bool has_value() const { return !N::is_empty_value(value_); }
  
  AK_TOOLBOX_CONSTEXPR reference_type value() const { return AK_TOOLBOX_ASSERTED_EXPRESSION(has_value(), N::access_value(value_)); }
};

} // namespace detail_

template <typename N, typename /* tag */ = default_tag>
class compact_optional : public detail_::compact_optional_base<N>
{
  typedef detail_::compact_optional_base<N> super;
  
public:

  typedef typename N::value_type value_type;
  typedef typename N::storage_type storage_type;
  typedef typename N::reference_type reference_type;

  AK_TOOLBOX_CONSTEXPR compact_optional() AK_TOOLBOX_NOEXCEPT_AS(storage_type(N::empty_value()))
    : super() {}
    
  AK_TOOLBOX_CONSTEXPR compact_optional(const value_type& v)
    : super(v) {}
    
  AK_TOOLBOX_CONSTEXPR compact_optional(value_type&& v)
    : super(std::move(v)) {}

  friend void swap(compact_optional& l, compact_optional&r)
  {
    using std::swap;
    swap (l.value_, r.value_);
  }
};

} // namespace compact_optional_ns

using compact_optional_ns::compact_optional;
using compact_optional_ns::empty_scalar_value;
using compact_optional_ns::compact_optional_type;
using compact_optional_ns::compact_optional_from_optional;
using compact_optional_ns::compact_bool;

} // namespace ak_toolbox

#undef AK_TOOLBOX_ASSERTED_EXPRESSION

#endif //AK_TOOLBOX_COMPACT_OPTIONAL_HEADER_GUARD_
