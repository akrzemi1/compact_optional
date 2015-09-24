# Documentation

NOTE: the documentation is not complete yet. I am still writing it...

## Overview

This library offers a way to represent optional (or nullable) objects (objects that may not have a value) of type `T`
by encoding the no-value state in one of the states of `T`. This is an alternative to [`boost::optional`](http://www.boost.org/doc/libs/1_59_0/libs/optional/doc/html/index.html) that offers a limited interface and a guarantee that the size of optional `T` is the same as the size of `T`.

In order to create an optional object type for your type `T`, you first need to decide how you want to represent the the no-value state in `T`. Suppose you want to use type `int` and want `-1` to represent the no-value. Now you have to define an _empty value policy_: a type that formally reflects how the no-value is represented. Luckily, for this common case the library comes with a predefined class template that you can use: `evp_int<int, -1>`. With this policy, you can define the type of the optional object, and use it:

```c++
compact_optional<evp_int<int, -1>> oi; // optional int
static_assert(sizeof(oi) == sizeof(int), "no size penalty");
```

Instances of `compact_optional` have a very small interface. They are copyable and movable. The default constructor initializes an object with the value indicated by the empty-value policy. Another explicit constructor takes the value of `T`:

```c++
using opt_int = compact_optional<evp_int<int, -1>>;
opt_int oi;      // internal value initialized to -1 (as per policy)
opt_int o1 {0};  // internal value initialized to 0
opt_int oN {-1}; // internal value initialized to -1
```

There are also three observer functions:
* `has_value` that checks if the currently stored value is that indicated in the empty-value policy (EVP); 
* `value` that extracts the stored value, with the precondition that the value is different than the one indicated in (EVP);
* `unsafe_raw_value` that extracts the value as stored internally, with no precondition.

```c++
// continuing the previous example
assert (!oi.has_value());
assert ( o0.has_value());
assert (!oN.has_value());

assert (o0.value() == 0);
// oi.value() is UB
// oN.value() is UB

assert (oi.unsafe_raw_value() == -1);
assert (o0.unsafe_raw_value() ==  0);
assert (o0.unsafe_raw_value() == -1);
```

As you can see, there are two ways to set the special empty value: either by default construction, or by providing it explicitly.

There are no relational operations provided, as it is not obvious how a no-value should compare against other values.
If you want to compare them, you need to provide a custom comparator, where you explicitly state how the empty state is treated.

There is no way to 'reset' the optional object other than to move-assign a new value:

```c++
// continuing the previous example
o0 = {};         // reset to empty value
oN = opt_int{2}; // new value
```

Each instance of `compact_optional` also provides three nested types:
* `value_type` - value we want to represent,
* `reference_type` - what function `value` returns: in most cases it is `const value_type&`,
* `storage_type` - type of the value that is used for storage: in most of the cases it is same as `value_type`.

## Empty-value policies

The library comes with a number of ready-to-use policies:

### evp_int

```c++
template <typename Integral, Integral EV> struct evp_int;
```

Can be used with all types that can be used as template non-type parameters, including built-in integral arithmetic types and pointers.

`Integral` represents the stored type.

`EV` is the value the empty value representation.

### evp_fp_nan

```c++
template <typename FPT> struct evp_fp_nan;
```

A policy for floating-point types, where the empty value is encoded as quiet NaN.

`FPT` Needs to be a floating-point scalar type.

### evp_value_init

```c++
template <typename T> struct evp_value_init;
```

A policy for storing any `Regular` type, the empty value is represented by a value-initialized `T`.

`T` Must meet the requirements of `Regular`: be default constructible, copyable, moveable, and `EqualityComparable`.

### evp_bool

```c++
struct evp_bool;
```

This is the policy for storing an optional `bool` in a compact way, such that the size of `compact_optional<evp_bool>` is 1.

### evp_optional

```c++
template <typename Optional> struct evp_optional;
```

This policy is used for types that cannot (or do not want to) spare any value to indicate the empty state. We logically represent optional `T`, but store it in `boost::optional<T>` or `std::experimental::optional<T>`.

`Optional` must be an instance of either `boost::optional` or `std::experimental::optional`.


## Type-altering tag

It is possible to pass a second type parameter to class template `compact_optional`.
Such a type does not need to be complete. It is used as a 'tag' to trigger different
instantiations of template `compact_optional` with the same empty-value policy:

```c++
using Count = compact_optional<evp_int<int, -1>, class count_tag>;
using Num   = compact_optional<evp_int<int, -1>, class num_tag>;

static_assert(!std::is_same<Count, Num>::value, "different types");
```

This behaves similarly to 'opaque typedef' feature: we get identical interface and behaviour, but two distinct non-interchangeable types.
