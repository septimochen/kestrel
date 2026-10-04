# Kestrel

Kestrel is an educational chess engine written in C++23. It starts with a readable
64-square board and adds complexity only after correctness is established.
There are no external library dependencies.

## Quick start

Requirements: CMake 3.30 or newer and a C++23 compiler with `std::expected` support (Clang or GCC).
Make is optional. On macOS, the Xcode Command Line Tools provide Clang.

From the repository root:

```sh
make check
make run
make perft DEPTH=3
```

Or use CMake directly:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/kestrel
./build/kestrel perft 3
```

The default invocation prints the version and the starting position's 20
pseudo-legal moves. `perft` prints a node count from the starting position.
Use a nonnegative integer depth; depth zero returns `1`. Negative depths and
invalid arguments are rejected with an error. The CLI does not accept FEN,
play interactive games, or implement UCI.

## Current implementation

| Component | Status |
| --- | --- |
| Board | 64-square array, side to move, castling rights, en passant target, clocks |
| FEN | Six-field structural validation; failure preserves the board; typed errors |
| Move generation | Ordinary movement, captures, and four promotion choices for both colors |
| Special moves | Make/undo has special-move branches; generation omits castling and en passant |
| Make/undo | Exact restoration for consistent moves; rights and clocks updated |
| Perft | Traverses pseudo-legal moves; starting depths 0–3 and board preservation pass |
| Search | Returns the first pseudo-legal move; ignores depth |

Attack detection, check detection, and legal move filtering are not implemented.
Starting depths 1–3 return 20, 400, and 8902. These shallow counts do not
establish a correct legal chess engine.

## Documentation

- [Architecture and API](docs/architecture.md): source map, board layout, data flow, and API contracts.
- [Development guide](docs/development.md): commands, tests, debugging, and contribution workflow.
- [Roadmap](docs/roadmap.md): actionable checklists, C++/chess learning goals, and correctness gates.
- [Modern C++ support](docs/modern-cpp.md): verified compiler features and optional experiments.
- [Bootstrap review](docs/bootstrap-review.md): confirmed bugs, fixes, and remaining limits.
- [Design review](docs/design-review.md): design improvements and implementation tradeoffs.
- [AGENTS.md](AGENTS.md): detailed project design and coding rules.

The immediate next milestone is attack detection, followed by legal move
generation and broader Perft validation. FEN serialization remains planned.
