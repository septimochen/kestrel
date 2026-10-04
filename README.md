# Kestrel

Kestrel is an educational chess engine written in C++20. It starts with a readable
64-square board and adds complexity only after correctness is established.
There are no external library dependencies.

## Quick start

Requirements: CMake 3.20 or newer and a C++20 compiler (Clang or GCC).
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
invalid arguments are not handled safely yet. The CLI does not accept FEN,
play interactive games, or implement UCI.

## Current implementation

| Component | Status |
| --- | --- |
| Board | 64-square array, side to move, castling rights, en passant target |
| FEN | Reads four fields; ignores clocks and trailing fields; validation is incomplete |
| Move generation | Ordinary movement, captures, and four promotion choices for both colors |
| Special moves | Make/undo has special-move branches; generation omits castling and en passant |
| Make/undo | Partial restoration; captures and promotions are not fully reversible |
| Perft | Traverses pseudo-legal moves; starting depths 1–3 have assertions; depth 3 currently fails |
| Search | Returns the first pseudo-legal move; ignores depth |

Attack detection, check detection, and legal move filtering are not implemented.
Depths 1 and 2 return 20 and 400; a fresh depth-three run currently returns
3735 rather than the legal reference count 8902. `make check` currently fails
at the depth-three assertion. Shallow starting-position counts do not establish
a correct chess engine. Capture restoration can also corrupt the board during traversal.

## Documentation

- [Architecture and API](docs/architecture.md): source map, board layout, data flow, and API contracts.
- [Development guide](docs/development.md): commands, tests, debugging, and contribution workflow.
- [Roadmap](docs/roadmap.md): actionable checklists, C++/chess learning goals, and correctness gates.
- [Design review](docs/design-review.md): design improvements and implementation tradeoffs.
- [AGENTS.md](AGENTS.md): detailed project design and coding rules.

The immediate next milestone is complete make/undo restoration, followed by
attack detection, legal move generation, and broader Perft validation.
