# compact_optional
An alternative to `boost::optional<T>` which does not store an additional `bool` flag,
but encodes the empty state inside `T` using a special indicated value.

## installation
It is a single-header (header-only) library.

## Usage

Do you want to store a possibly missing `int`? Can you spare value `-1`? You can use it like this:

```c++
using namespace ak_toolbox;
typedef compact_optional<int, empty_scalar_value<int, -1>> opt_int;

opt_int oi;
opt_int o2 (2);

assert (!oi.has_value());
assert (o2.has_value());
assert (o2.value() == 2);

static_assert (sizeof(opt_int) == sizeof(int), "");
```

Do you want to store a possibly missing `std::string`, where 'missing' != 'empty'?
Can you spare some string value, like `std::string("\0\0", 2)`? This is how you do it:

```c++
struct string_empty_value
{
  static std::string empty_value() { return std::string("\0\0", 2); }
  static bool is_empty_value(const std::string& v) { return v == std::string("\0\0", 2); }
};

typedef ak_toolbox::compact_optional<std::string, string_empty_value> opt_str;
opt_str os, oE(std::string(""));

assert (!os.has_value());
assert (oE.has_value());
assert (oE.value() == "");

static_assert (sizeof(opt_str) == sizeof(std::string), "");
```

