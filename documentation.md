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

`FPT` needs to be a floating-point scalar type.

### evp_value_init

```c++
template <typename T> struct evp_value_init;
```

A policy for storing any `Regular` type, the empty value is represented by a value-initialized `T`.

`T` must meet the requirements of `Regular`: be default constructible, copyable, moveable, and `EqualityComparable`.

### evp_stl_empty

```c++
template <typename Cont> struct evp_stl_empty;
```

Empty value is created using value initialization. Empty value is checked by calling member function `empty()`. Useful for STL containers where we want the empty container to represent the no-value.

`T` must be default constructible and have a member function `empty()`.


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


### Using POD storage for empty value

Sometimes there is no spare value of `T`, but there is a spare sequence of bits in `POD<T>`, where `POD<T>` is a raw-memory representation of `T` (i.e., its local part -- the part that amounts to `sizeof(T)`). Consider the following type:

```c++
class minutes_since_midnight
{
  int min_;
  
public:
  bool invariant() const { return min_ >= 0 && min_ < 24 * 60; }
  
  explicit minutes_since_midnight(int m) : min_(m) 
  { 
    assert (invariant());
  }
  
  int as_int() const
  {
    assert (invariant());
    return min_;
  }
  
  ~minutes_since_midnight()
  {
    assert (invariant());
  }
};
```

Member subobject `min_` is expected to range from 0 (inclusive) to 1440 (exclusive). This leaves many spare values, e.g., -1. But if we try to use them, we violate the invariant, and trigger assertion failure. In such case, `compact_optional` allows you to use a POD type `int` for storage and only reinterpret it as `minutes_since_midnight` upon extracting the value. Of course, the wrapper deals with manual life-time management issues internally, calling in-place `new` and pseudo destructor calls where necessary. In order to use that functionality, you have to create an empty value policy that derives from `compact_optional_pod_storage_type`:

```c++
struct evp_minutes : compact_optional_pod_storage_type<minutes_since_midnight, int>
{
  static storage_type empty_value() { return -1; }
  static bool is_empty_value(const storage_type& v) { return v == -1; }
};
```

The first argument is the type we want to represent; the second type (`int`) is the POD type, of the same size and alignment as `T` (the first argument). If it is not provided, the implementation uses `std::aligned_storage_t<sizeof(T), alignof(T)>`. the two functions `empty_value` and `is_empty_value` describe the empty value on the POD type, where no invariant is enforced.

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


## Comparison with Boost.Optional

This library is not a replacement for [`boost::optional`](http://www.boost.org/doc/libs/1_59_0/libs/optional/doc/html/index.html). While there is some overlap, both libraries target different use cases.

### Genericity

`boost::optional` is really generic: It will work practically with any `T`, to the extent that you can use `optional<optional<T>>`. You just give the type `T` and you get the optional object wrapper. In contrast, in `compact_optional` from the outset you have to make a choice case-by-case how you want to store the `empty` value `T`. The policy for managing the empty state is part of the contract, part of semantics, part of the type. Having a nested `compact_optional` is technically possible, but requires sparing two values of `T`.

Some type `T` may not have a 'spare' value to indicate the empty state. In such case, `compact_optional` cannot help you. In contrast, `boost::optional<T>` will work just fine: the additional empty state is stored separately. In a way, `boost::optional<T>` can be thought as [`boost::variant`](http://www.boost.org/doc/libs/1_59_0/doc/html/variant.html)`<boost::none_t, T>`.

### Life-time management

`boost::optional<T>` gives you a useful guarantee that if you initialize it to a no-value state, no object of type `T` is created. This is useful for run-time performance reasons and allows a two phase initialization. In contrast, `compact_optional` upon construction, immediately constructs a `T`. `compact_optional` simply contains `T` as subobject. In contrast, `boost::optional` only provides a storage for `T` and creates and destroys the contained object as needed.

### No container-like semantics

`boost::optional<T>` is almost a container, capable of storing 0 or 1 elements. When it stores one element, you can alter its value. It is impossible in `compact_optional`: its value and the "container's size" are one thing. But in exchange, the latter, because it only provides a non-mutable access to the contained value, it can build the return value on the fly, similarly to the proxy reference in `std::vector<bool>`, but because we only provide a non-mutable reference, it is much simpler and safer. This is why we can provide a compact_optional for type `bool` (which has no spare value) with the `sizeof` of a single `char`.

### Expressiveness vs non-ambiguity

`compact_optional` has a minimalistic interface: this is also to avoid any surprises. As a cost it is less expressive and convenient. There are no implicit conversions, no overloaded operators; unlike `boost::optional`, it does not have `operator==`: you have to provide your own comparator and decide yourself how you want to compare the no-value states.


## Intended use

In general, it is expected that in the first pass of the implementation of your program, you will use `boost::optional<T>` as a simple ready-to-use solution. Later, if you determine that `boost::optional` kills your performance, you can think of replacing it with `compact_optional` and how you want to represent the no-value state.

another use case is when you are currently using objects of scalar types with encoded special values (like type `std::string::size_type` with value `std::string::npos`) and you want to change it into something safer but be sure you are adding no runtime overhead. You can change your type to `compact_optional<evp_int<string::size_type, string::npos>>`.

## Future plans

Provide relational operations upon request.
