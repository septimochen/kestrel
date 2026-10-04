# Modern C++ on this machine

Verified on 2026-10-04: Apple Clang 21.0.0 (`clang-2100.3.34.2`), arm64 macOS,
Command Line Tools, with CMake 4.4.3. Compiler and standard-library support are
separate; upstream version tables do not prove Apple's shipped feature set.

| Feature | Local result |
| --- | --- |
| `-std=c++23` | Accepted; full project tests pass |
| `std::expected`, including `transform` | Compiles, links, and runs |
| `std::println` | Compiles, links, and runs |
| Explicit object parameters (`this const T&`) | Compiles and runs |
| `if consteval`, `std::to_underlying` | Compile-time and runtime checks pass |
| `-std=c++26` | Accepted; full project tests pass |
| C++26 pack indexing (`Types...[0]`) | Compiles in C++26; rejected in strict C++23 |
| C++26 constexpr throw/catch | Probe rejected at constant evaluation |
| Reflection with `<meta>` / `^^int` | Probe rejected: `<meta>` unavailable |

This is partial C++26 support, not a claim that all C++23 or C++26 features work.
Probes used `-pedantic-errors` to avoid mistaking extensions for standard support.

## Try the examples

Default engine code uses C++23. `Board::loadFen` returns
`std::expected<void, FenError>`: successful parsing carries no value, while errors
carry a category. The bool `setFromFen` wrapper remains available.

```cpp
kestrel::Board board;
auto result = board.loadFen("8/8/8/8/8/8/8/8 w - - 0 1");
if (!result) {
    auto category = result.error();
    // Handle the category; the original board was preserved.
    (void)category;
}
```

The optional `examples/modern_cpp.cpp` demonstrates the other supported features
without adding unrelated machinery to the engine:

```sh
make check BUILD_DIR=build-cpp23 CPP_EXPERIMENTS=ON
./build-cpp23/kestrel_cpp_features
make check BUILD_DIR=build-cpp26 CXX_STANDARD=26 CPP_EXPERIMENTS=ON
./build-cpp26/kestrel_cpp_features
```

For direct CMake, use `-DKESTREL_CXX_STANDARD=26` and
`-DKESTREL_CPP_EXPERIMENTS=ON`. CMake 3.30+ is required for the `cxx_std_26` compile feature used here.
Features are selected individually; changing mode does not install a newer
standard library. Add a small strict compilation probe before adopting another
feature. Keep experimental examples separate and use features in the core when
they make the code clearer.

Official support references: [Clang language status](https://clang.llvm.org/cxx_status.html)
and [libc++ C++23 status](https://libcxx.llvm.org/Status/Cxx23.html).
