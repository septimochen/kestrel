# Development guide

## Build and run

The Makefile wraps the existing CMake build without adding library dependencies.
Its default build type is Debug and language standard is C++23. Tests use explicit
checks that remain active in Release.

| Command | Purpose |
| --- | --- |
| `make build` | Configure and compile |
| `make check` or `make test` | Build and run CTest with failure output |
| `make run` | Build and run the demo |
| `make perft DEPTH=3` | Build and count starting-position nodes |
| `make format` | Format C++ source and headers with optional `clang-format` |

Override the build directory or configuration when needed:

```sh
make check BUILD_DIR=build-debug BUILD_TYPE=Debug
make build BUILD_DIR=build-release BUILD_TYPE=Release
```

`clang-format` is only needed for the formatting command. Review its diff before
committing, since formatting the whole tree can change unrelated code.

## Existing validation

CTest registers Perft, board-state, and invalid-CLI tests. Perft verifies starting
counts of 1, 20, 400, and 8902 at depths 0–3 plus full board preservation. Board
tests cover nested captures, special moves, rights, clocks, and FEN errors for
both colors. All checks run under `NDEBUG` too.

```sh
make check
./build/kestrel perft 0
./build/kestrel perft 1
./build/kestrel perft 2
./build/kestrel perft 3
```

These commands produce `1`, `20`, `400`, and `8902`. Legal move filtering,
attack detection, special-move generation, and search remain unimplemented.
Depth four's legal reference count is 197281; it is not currently a passing
bootstrap requirement.

Additional verified configurations:

```sh
make check BUILD_DIR=build-release BUILD_TYPE=Release
make check BUILD_DIR=build-sanitize SANITIZERS=ON
make check BUILD_DIR=build-cpp26 CXX_STANDARD=26 CPP_EXPERIMENTS=ON
```

AddressSanitizer and UndefinedBehaviorSanitizer are opt-in for Clang/GCC.
See [modern-cpp.md](modern-cpp.md) for supported feature examples.

## Adding engine features

1. Read the relevant header and implementation, plus the design rules in
   [AGENTS.md](../AGENTS.md).
2. Keep the change focused and readable; preserve deterministic behavior.
3. Add executable tests for the changed subsystem and register additional test
   executables in `CMakeLists.txt` when needed.
4. Run Debug checks and inspect `git diff --check`.
5. Update the feature status and documentation to match the actual behavior.
6. Commit with a descriptive message and push when a remote is configured.

For make/undo work, compare every square and all metadata before and after a
round trip. Cover quiet moves, captures, promotions, promotion captures,
castling, and en passant for both colors. Then test nested make/undo sequences.

For legal generation, cover attacks, pins, check evasions, and king safety,
including castling through attack and en passant exposing a king. Broaden Perft
to known positions with castling, en passant, and promotion only after undo is
reliable. Perft divide is a planned debugging tool, not a current CLI command.

## Debugging incorrect counts

A count mismatch may come from missing moves, extra illegal moves, or state
corruption between siblings. Check state restoration first. Once divide exists,
compare root move counts and follow the first mismatching subtree. A shallow
starting-position success alone is insufficient evidence of correctness.

## Tracked files

The root `.gitignore` uses an allowlist: new paths are ignored unless explicitly
allowed. Source headers, implementation files, test files, documentation, and
root build metadata are included. Build outputs remain ignored. Use
`git check-ignore -v <path>` when a new artifact does not appear in status, then
add a narrow allowlist rule if it belongs in the repository.
