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

It is not possible to change the stored value through function `value()`: it returns a non-mutable reference or value (based on the policy).

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

What type is being stored and how the empty value is encoded is controlled by _empty-value policy_. You can either define your own, or use one of the policies provided with this library:

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

### Defining a custom empty-value policy

In order to provide a custom empty-value policy to store a given type `T`, we need to provide a class that derive it from `compact_optional_type<T>` and implements two static member functions: `empty_value` and `is_empty_value`:

```c++
struct evp_string_with_0s : compact_optional_type<std::string>
{
  static storage_type empty_value() { 
    return std::string("\0\0", 2);
  }
  static bool is_empty_value(const storage_type& v) {
    return v.compare(0, v.npos, "\0\0", 2) == 0;
  }
};
```

Base class `compact_optional_type<T>` defines all the necessary nested types and some house-keeping functions. With it, we are declaring what type we will be storing.

Function `empty_value` returns a value of the stored type (`storage_type`) that represents the empty value.

Function `is_empty_value` returns true iff the the given value is recognized as the empty value.

In a less likely case where we want to store the represent an optional value of type `T`, but store it internally in a different type, we need to provide more arguments to `compact_optional_type<T>`. Suppose we want to implement the policy for storing type `bool` in a storage of size 1 (the same way that `evp_bool` does). We need three states: no-value, `true`, and `false`. We cannot store it in type `bool` because it only has two states. So, for storage we will use type `char`. We will use value `2` (`'\2'`) to represent the empty state, value `0` to represent value `false` and `1` to represent `true`. Now, apart from defining how the empty state is encodes, we also need to provide a recipe on how to encode a `bool` in a `char`, and how to extract the `bool` value from `char` storage. We need to define additional two static member functions: `access_value` and `store_value`:

```c++
struct compact_bool_policy : compact_optional_type<bool, char, bool> // see below
{
  static storage_type empty_value() { return char(2); }
  static bool is_empty_value(storage_type v) { return v == 2; }
  
  static reference_type access_value(const storage_type& v) { return bool(v); }
  static storage_type store_value(const value_type& v) { return v; }
};
```

The three types passed to `compact_optional_type` denote respectively:

1. `value_type` -- the type we are modelling.
2. `storage_type` -- the type we use to store the values internally.
3. `reference_type` -- what type function `value` should return.

because function `value()` should return a `bool`  and we are storing no `bool` we have to create a temporary value, and return it by value: therefore type `reference_type` is not really a reference.

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
