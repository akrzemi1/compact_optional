# compact_optional [![Build Status](https://travis-ci.org/akrzemi1/compact_optional.svg?branch=master)](https://travis-ci.org/akrzemi1/compact_optional)

**WARNING:** This library is not maintained, and deprecated. It is recommended that you use [`markable`](https://github.com/akrzemi1/markable) instead.

An alternative to `boost::optional<T>` which does not store an additional `bool` flag,
but encodes the empty state inside `T` using a special indicated value.

## installation
It is a C++11 single-header (header-only) library.

## Usage

Do you want to store a possibly missing `int`? Can you spare value `-1`? You can use it like this:

```c++
using namespace ak_toolbox;
typedef compact_optional<evp_int<int, -1>> opt_int;

opt_int oi;
opt_int o2 (2);

assert (!oi.has_value());
assert (o2.has_value());
assert (o2.value() == 2);

static_assert (sizeof(opt_int) == sizeof(int), "");
```

Do you want to store a possibly missing `std::string`, where 'missing' != 'empty'?
Can you spare some string values that contain a null character inside, like `std::string("\0\0", 2)`? This is how you do it:

```c++
struct string_empty_value : ak_toolbox::compact_optional_type<std::string>
{
  static std::string empty_value() { 
    return std::string("\0\0", 2);
  }
  static bool is_empty_value(const std::string& v) {
    return v.compare(0, v.npos, "\0\0", 2) == 0;
  }
};

typedef ak_toolbox::compact_optional<string_empty_value> opt_str;
opt_str os, oE(std::string(""));

assert (!os.has_value());
assert (oE.has_value());
assert (oE.value() == "");

static_assert (sizeof(opt_str) == sizeof(std::string), "");
```

Cannot spare any value, but still want to use this interface? You can use `boost::optional` or `std::experimental::optional` at the cost of storage size:

```c++
typedef compact_optional<evp_optional<boost::optional<int>>> opt_int;
static_assert (sizeof(opt_int) == sizeof(boost::optional<int>), "");
```

We also provide a customization for optional `bool` with the size of a `char`:

```c++
typedef compact_optional<evp_bool> opt_bool;
static_assert (sizeof(opt_bool) == 1, "");
```

For additional motivation and overview see this post: https://akrzemi1.wordpress.com/2015/07/15/efficient-optional-values/.

## Acknowledgments

The idea of encoding the policy for determining the empty state in a template parameter has been proposed by Vitali Lovich (see https://groups.google.com/a/isocpp.org/d/topic/std-proposals/46J1onhWJ-s/discussion).

Matt Calabrese and Tomasz Kami&#324;ski suggested the raw-storage representation for holding optional types with strong invariants.

## License
Distributed under the [Boost Software License, Version 1.0](http://www.boost.org/LICENSE_1_0.txt).
