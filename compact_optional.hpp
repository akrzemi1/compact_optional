// Copyright (C) 2015, Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef AK_TOOLBOX_COMPACT_OPTIONAL_HEADER_GUARD_
#define AK_TOOLBOX_COMPACT_OPTIONAL_HEADER_GUARD_

#include <cassert>
#include <utility>
#include <limits>
#include <new>
#  include <type_traits>

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
#  define AK_TOOLBOX_CONSTEXPR_NOCONST // fix in the future
#endif

#if defined NDEBUG
# define AK_TOOLBOX_ASSERTED_EXPRESSION(CHECK, EXPR) (EXPR)
#elif defined __clang__ || defined __GNU_LIBRARY__
# define AK_TOOLBOX_ASSERTED_EXPRESSION(CHECK, EXPR) ((CHECK) ? (EXPR) : (fail(#CHECK, __FILE__, __LINE__), (EXPR)))
  inline void fail(const char* expr, const char* file, int line)
  {
    __assert(expr, file, line);
  }
#elif defined __GNUC__
# define AK_TOOLBOX_ASSERTED_EXPRESSION(CHECK, EXPR) ((CHECK) ? (EXPR) : (fail(#CHECK, __FILE__, __LINE__), (EXPR)))
  inline void fail(const char* expr, const char* file, unsigned line)
  {
    _assert(expr, file, line);
  }
#else
# error UNSUPPORTED COMPILER
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
struct evp_int : compact_optional_type<T>
{
  static AK_TOOLBOX_CONSTEXPR T empty_value() AK_TOOLBOX_NOEXCEPT { return Val; }
  static AK_TOOLBOX_CONSTEXPR bool is_empty_value(T v) { return v == Val; }
};

// for backward compatibility only:
template <typename T, T Val>
struct empty_scalar_value : compact_optional_type<T>
{
  static AK_TOOLBOX_CONSTEXPR T empty_value() AK_TOOLBOX_NOEXCEPT { return Val; }
  static AK_TOOLBOX_CONSTEXPR bool is_empty_value(T v) { return v == Val; }
};

template <typename FPT>
struct evp_fp_nan : compact_optional_type<FPT>
{
  static AK_TOOLBOX_CONSTEXPR FPT empty_value() AK_TOOLBOX_NOEXCEPT { return std::numeric_limits<FPT>::quiet_NaN(); }
  static AK_TOOLBOX_CONSTEXPR bool is_empty_value(FPT v) { return v != v; }
};

template <typename T> // requires Regular<T>
struct evp_value_init : compact_optional_type<T>
{
  static AK_TOOLBOX_CONSTEXPR T empty_value() AK_TOOLBOX_NOEXCEPT_AS(T()) { return T(); }
  static AK_TOOLBOX_CONSTEXPR bool is_empty_value(const T& v) { return v == T(); }
};

template <typename T>
struct evp_stl_empty : compact_optional_type<T>
{
  static AK_TOOLBOX_CONSTEXPR T empty_value() AK_TOOLBOX_NOEXCEPT_AS(T()) { return T(); }
  static AK_TOOLBOX_CONSTEXPR bool is_empty_value(const T& v) { return v.empty(); }
};

template <typename OT>
struct evp_optional : compact_optional_type<typename OT::value_type, OT>
{
  typedef typename OT::value_type value_type;
  typedef OT storage_type;

  static OT empty_value() AK_TOOLBOX_NOEXCEPT { return OT(); }
  static bool is_empty_value(const OT& v) { return !v; }
  
  static const value_type& access_value(const storage_type& v) { return *v; }
  static storage_type store_value(const value_type& v) { return v; }
  static storage_type store_value(value_type&& v) { return std::move(v); }
};

// for backwards compatibility only:
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

struct evp_bool : compact_optional_type<bool, char, bool>
{
  static AK_TOOLBOX_CONSTEXPR char empty_value() AK_TOOLBOX_NOEXCEPT { return char(2); }
  static AK_TOOLBOX_CONSTEXPR bool is_empty_value(char v) { return v == 2; }
  
  static AK_TOOLBOX_CONSTEXPR bool access_value(const char& v) { return bool(v); }
  static AK_TOOLBOX_CONSTEXPR char store_value(const bool& v) { return v; }
};

typedef evp_bool compact_bool;


struct compact_optional_pod_storage_type_tag{};

#ifndef AK_TOOLBOX_NO_ARVANCED_CXX11
template <typename T, typename POD_T = typename std::aligned_storage<sizeof(T), alignof(T)>::type>
#else
template <typename T, typename POD_T>
#endif // AK_TOOLBOX_NO_ARVANCED_CXX11

struct compact_optional_pod_storage_type : compact_optional_pod_storage_type_tag
{
  static_assert(sizeof(T) == sizeof(POD_T), "pod storage for T has to have the same size and alignment as T");
  static_assert(std::is_pod<POD_T>::value, "second argument must be a POD type");
#ifndef AK_TOOLBOX_NO_ARVANCED_CXX11
  static_assert(alignof(T) == alignof(POD_T), "pod storage for T has to have the same alignment as T");
#endif // AK_TOOLBOX_NO_ARVANCED_CXX11

  typedef T value_type;
  typedef POD_T storage_type;
  typedef const T& reference_type;
  
  static const value_type& access_value(const storage_type& s) { return reinterpret_cast<const value_type&>(s); }
  static const storage_type& store_value(const value_type& v) { return reinterpret_cast<const storage_type&>(v); }  
};

#ifndef AK_TOOLBOX_NO_ARVANCED_CXX11
template <typename Enum, typename std::underlying_type<Enum>::type Val> 
struct evp_enum : compact_optional_pod_storage_type<Enum, typename std::underlying_type<Enum>::type>
{
  typedef compact_optional_pod_storage_type<Enum, typename std::underlying_type<Enum>::type> base;
  typedef typename base::storage_type storage_type;
  
  static storage_type empty_value() { return Val; }
  static bool is_empty_value(const storage_type& v) { return v == Val; }
};
#endif // AK_TOOLBOX_NO_ARVANCED_CXX11

namespace detail_ {


template <typename EVP>
struct member_storage
{
  typedef typename EVP::value_type value_type;
  typedef typename EVP::storage_type storage_type;
  typedef typename EVP::reference_type reference_type;
  
  storage_type value_;
  
  AK_TOOLBOX_CONSTEXPR member_storage() AK_TOOLBOX_NOEXCEPT_AS(storage_type(EVP::empty_value()))
    : value_(EVP::empty_value()) {}
    
  AK_TOOLBOX_CONSTEXPR member_storage(const value_type& v)
    : value_(EVP::store_value(v)) {}
    
  AK_TOOLBOX_CONSTEXPR member_storage(value_type&& v)
    : value_(EVP::store_value(std::move(v))) {}
    
  void swap_impl(member_storage& rhs)
  {
    using namespace std;
    swap(value_, rhs.value_);
  }
};

template <typename EVP>
struct buffer_storage
{
  typedef typename EVP::value_type value_type;
  typedef typename EVP::storage_type storage_type;
  typedef typename EVP::reference_type reference_type;
  
  storage_type value_;
  
private:
  void* address() { return static_cast<void*>(std::addressof(value_)); }
  void construct(const value_type& v) { ::new (address()) value_type(v); }
  void construct(value_type&& v) { ::new (address()) value_type(std::move(v)); }
  void call_destructor() { as_value_type().value_type::~value_type(); }
  void destroy() { call_destructor(); value_ = EVP::empty_value(); } // TODO: "fill_empty_value_pattern"
  bool has_value() const { return !EVP::is_empty_value(value_); }
  value_type& as_value_type() { return reinterpret_cast<value_type&>(value_); }
  const value_type& as_value_type() const { return reinterpret_cast<const value_type&>(value_); }
  
public:
  buffer_storage() AK_TOOLBOX_NOEXCEPT_AS(storage_type(EVP::empty_value()))
    : value_(EVP::empty_value()) {}
    
  buffer_storage(const value_type& v) : value_(EVP::empty_value())
    { construct(v); }
    
  buffer_storage(value_type&& v) : value_(EVP::empty_value())
    { construct(std::move(v)); }
    
  buffer_storage(const buffer_storage& rhs) : value_(EVP::empty_value())
    {
      if (rhs.has_value())
        construct(rhs.as_value_type());
    }
    
  buffer_storage(buffer_storage&& rhs) : value_(EVP::empty_value())
    {
      if (rhs.has_value())
        construct(std::move(rhs.as_value_type())); // TODO: add move
    }
    
  void operator=(const buffer_storage& rhs)
    {
      if (has_value() && rhs.has_value())
      {
        as_value_type() = rhs.as_value_type();
      }
      else if (has_value() && !rhs.has_value())
      {
        destroy();
      }
      else if (!has_value() && rhs.has_value())
      {
        construct(rhs.as_value_type());
      }
    }
    
  void operator=(buffer_storage&& rhs)
    {
      if (has_value() && rhs.has_value())
      {
        as_value_type() = std::move(rhs.as_value_type());
      }
      else if (has_value() && !rhs.has_value())
      {
        destroy();
      }
      else if (!has_value() && rhs.has_value())
      {
        construct(std::move(rhs.as_value_type()));
      }
    }
    
  void swap_impl(buffer_storage& rhs)
  {
    using namespace std;
    if (has_value() && rhs.has_value())
    {
      swap(as_value_type(), rhs.as_value_type());
    }
    else if (has_value() && !rhs.has_value())
    {
      rhs.construct(std::move(as_value_type()));
      destroy();
    }
    else if (!has_value() && rhs.has_value())
    {
      construct(std::move(rhs.as_value_type()));
      rhs.destroy();
    }
  }
    
  ~buffer_storage()
  {
    if (has_value())
      call_destructor();
  }
  // TODO: implement moves and copies, swap, dtor
};

template <typename T>
struct storage_destruction
{

  typedef typename std::conditional<std::is_base_of<compact_optional_pod_storage_type_tag, T>::value,
                                    buffer_storage<T>, 
                                    member_storage<T>>::type type;
};

template <typename N>
class compact_optional_base : storage_destruction<N>::type
{
  typedef typename storage_destruction<N>::type base;
  base& as_base() { return static_cast<base&>(*this); }
  
protected:
  typedef typename N::value_type value_type;
  typedef typename N::storage_type storage_type;
  typedef typename N::reference_type reference_type;
  void swap_storages(compact_optional_base& rhs) { as_base().swap_impl(rhs.as_base()); }
  
  AK_TOOLBOX_CONSTEXPR_NOCONST storage_type& raw_value() { return base::value_; }
  
public:
  AK_TOOLBOX_CONSTEXPR compact_optional_base() AK_TOOLBOX_NOEXCEPT_AS(base())
    : base() {}
    
  AK_TOOLBOX_CONSTEXPR compact_optional_base(const value_type& v)
    : base(v) {}
    
  AK_TOOLBOX_CONSTEXPR compact_optional_base(value_type&& v)
    : base(std::move(v)) {}
    
  AK_TOOLBOX_CONSTEXPR bool has_value() const { return !N::is_empty_value(base::value_); }
  
  AK_TOOLBOX_CONSTEXPR reference_type value() const { return AK_TOOLBOX_ASSERTED_EXPRESSION(has_value(), N::access_value(base::value_)); }
  
  AK_TOOLBOX_CONSTEXPR storage_type const& unsafe_raw_value() const { return base::value_; }
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
    l.swap_storages(r);
  }
};

} // namespace compact_optional_ns

using compact_optional_ns::compact_optional;
using compact_optional_ns::empty_scalar_value;
using compact_optional_ns::compact_optional_type;
using compact_optional_ns::compact_optional_pod_storage_type;
using compact_optional_ns::compact_optional_from_optional;
using compact_optional_ns::compact_bool;
using compact_optional_ns::evp_bool;
using compact_optional_ns::evp_int;
using compact_optional_ns::evp_fp_nan;
using compact_optional_ns::evp_value_init;
using compact_optional_ns::evp_optional;
using compact_optional_ns::evp_stl_empty;
using compact_optional_ns::evp_enum;

} // namespace ak_toolbox

#undef AK_TOOLBOX_ASSERTED_EXPRESSION

#endif //AK_TOOLBOX_COMPACT_OPTIONAL_HEADER_GUARD_
