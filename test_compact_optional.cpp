// Copyright (C) 2015, Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "compact_optional.hpp"
#include <cassert>
#include <utility>
#include <string>

using namespace ak_toolbox;

template <typename T>
void ignore(T&&) {}

void test_value_ctor()
{
  {
    typedef compact_optional< int, empty_scalar_value<int, -1> > opt_int;
    opt_int oi_, oiN1(-1), oi0(0), oi1(1);
    assert (!oi_.has_value());
    assert (!oiN1.has_value());
    assert ( oi0.has_value());
    assert ( oi1.has_value());
    
    assert (oi0.value() == 0);
    assert (oi1.value() == 1);
  }
  {
    typedef compact_optional< int, empty_scalar_value<int, 0> > opt_int;
    opt_int oi_, oiN1(-1), oi0(0), oi1(1);
    assert (!oi_.has_value());
    assert ( oiN1.has_value());
    assert (!oi0.has_value());
    assert ( oi1.has_value());
    
    assert (oiN1.value() == -1);
    assert (oi1.value() == 1);
  }
}

struct string_empty_value
{
  static std::string empty_value() { return std::string("\0\0", 2); }
  static bool is_empty_value(const std::string& v) { return v == std::string("\0\0", 2); }
};

void test_string_traits()
{
  typedef compact_optional<std::string, string_empty_value, class tag_X> opt_str;
  opt_str os_, os00(std::string("\0\0", 2)), os0(std::string("\0")), osA(std::string("A"));
  assert (!os_.has_value());
  assert (!os00.has_value());
  assert ( os0.has_value());
  assert ( osA.has_value());
}

int main()
{
  test_value_ctor();
  test_string_traits();
}
