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
