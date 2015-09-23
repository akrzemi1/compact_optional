// Copyright (C) 2015, Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "compact_optional.hpp"
#include <cassert>
#include <utility>
#include <string>

#if defined AK_TOOLBOX_USING_BOOST
#include <boost/optional.hpp>
#endif

using namespace ak_toolbox;

template <typename T>
void ignore(T&&) {}

void test_value_ctor()
{
  {
    typedef compact_optional< evp_int<int, -1> > opt_int;
    static_assert (sizeof(opt_int) == sizeof(int), "size waste");
    
    opt_int oi_, oiN1(-1), oi0(0), oi1(1);
    assert (!oi_.has_value());
    assert (!oiN1.has_value());
    assert ( oi0.has_value());
    assert ( oi1.has_value());
    
    assert (oi0.value() == 0);
    assert (oi1.value() == 1);
  }
  {
    typedef compact_optional< evp_int<int, 0> > opt_int;
    static_assert (sizeof(opt_int) == sizeof(int), "size waste");
    opt_int oi_, oiN1(-1), oi0(0), oi1(1);
    assert (!oi_.has_value());
    assert ( oiN1.has_value());
    assert (!oi0.has_value());
    assert ( oi1.has_value());
    
    assert (oiN1.value() == -1);
    assert (oi1.value() == 1);
  }
}

struct string_empty_value : compact_optional_type<std::string>
{
  static std::string empty_value() { return std::string("\0\0", 2); }
  static bool is_empty_value(const std::string& v) { return v == std::string("\0\0", 2); }
};

void test_string_traits()
{
  typedef compact_optional<string_empty_value, class tag_X> opt_str;
  static_assert (sizeof(opt_str) == sizeof(std::string), "size waste");
  
  opt_str os_, os00(std::string("\0\0", 2)), os0(std::string("\0")), osA(std::string("A"));
  assert (!os_.has_value());
  assert (!os00.has_value());
  assert ( os0.has_value());
  assert ( osA.has_value());
}

struct string_in_pair_empty_val : compact_optional_type< std::string, std::pair<bool, std::string> >
{
  static storage_type empty_value() { return storage_type(false, "anything"); }
  static bool is_empty_value(const storage_type& v) { return v.first == false; }
  
  static const value_type& access_value(const storage_type& v) { return v.second; }
  static storage_type store_value(const value_type& v) { return storage_type(true, v); }
  static storage_type store_value(value_type&& v) { return storage_type(true, std::move(v)); }
};

void test_custom_storage()
{
  typedef compact_optional<string_in_pair_empty_val> opt_str;
  opt_str os_, os0(std::string("\0")), osA(std::string("A"));
  
  assert (!os_.has_value());
  
  assert (os0.has_value());
  assert (os0.value() == "");
  
  assert (osA.has_value());
  assert (osA.value() == "A");

  swap (os_, os0);
    
  assert (os_.has_value());
  assert (os_.value() == "");
  assert (!os0.has_value());
}

void test_bool_storage()
{
  typedef compact_optional<evp_bool> opt_bool;
  static_assert (sizeof(opt_bool) == 1, "size waste");
  
  opt_bool ob_, obT(true), obF(false);
  assert (!ob_.has_value());
  
  assert (obT.has_value());
  assert (obT.value() == true);
  
  assert (obF.has_value());
  assert (obF.value() == false);
}

#if defined AK_TOOLBOX_USING_BOOST
void test_optional_as_storage()
{
  typedef compact_optional<evp_optional<boost::optional<int>>> opt_int;
  opt_int oi_, oiN1(-1), oi0(0);
  assert (!oi_.has_value());
  
  assert (oiN1.has_value());
  assert (oiN1.value() == -1);
  
  assert (oi0.has_value());
  assert (oi0.value() == 0);
}
#endif

int main()
{
  test_value_ctor();
  test_string_traits();
  test_custom_storage();
  test_bool_storage();
#if defined AK_TOOLBOX_USING_BOOST
  test_optional_as_storage();
#endif
}
