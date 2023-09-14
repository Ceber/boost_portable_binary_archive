# `boost::archive::portable_binary`
<!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
[![All Contributors](https://img.shields.io/badge/all_contributors-1-orange.svg?style=flat-square)](#contributors-)
<!-- ALL-CONTRIBUTORS-BADGE:END -->

![Workflow](https://github.com/Ceber/boost_portable_binary_archive/actions/workflows/cmake-single-platform.yml/badge.svg)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://paypal.me/ceber68)

## Background

This library provides "portable binary" input/output archive objects usable with [boost serialization](https://www.boost.org/doc/libs/1_75_0/libs/serialization/doc/serialization.html) module.

It is inspired by [eos-portable-archive](https://github.com/daldegam/eos-portable-archive), and essentially serves as "portable/multiplatform" version for the binary serialization/de-serialization use-case

By portable, it's meant that you can seriliazed from a platform, and deserialized with another (e.g: aarch64 to armv7hf, and vice versa).

## Availibilities
- Standard Types serialization
- Objects/Structs serialization
- Vector/Array serialization 
- Map serialization
- Polymorphic serialization
- CPack/STGZ Packaging
- Conan Package management
- Code coverage computation
- Code formatting computation

## Build


### CMake Build
From project root directory:
```
cmake -S . -B build/<any_subdirectory>
cmake --build build/<any_subdirectory>
```

### Conan Build
From project root directory:
```
conan create . -pr:b=default -pr:h=Linux_Release --build boost_portable_binary_archive --build missing
```


## Usage
See unit-tests.

## Running unit-tests

From build folder:
```
ctest --verbose

```


## Requirements
- C++17
- [boost >=1.75](https://www.boost.org/) [boost::serialization]
- [gtest](https://github.com/google/googletest) [tests only]


## Notes

### Legacy C++ support

This library is written in C++17, but usage of syntax/STL features is fairly limited. Namely,
there is liberal usage of `if constexpr` over the older `enable_if` tricks for clarity.

If there is a demonstrated need for C++11/14 support, getting things in shape would be very doable.


### Build System Requirements

At least CMake v3.13.

### Contributors ✨
Special thanks to you !
<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->

<!-- markdownlint-restore -->
<!-- prettier-ignore-end -->

<!-- ALL-CONTRIBUTORS-LIST:END -->

This project follows the [all-contributors](https://github.com/all-contributors/all-contributors) specification. Contributions of any kind welcome!
