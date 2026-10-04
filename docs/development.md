# Development guide

## Build and run

The Makefile wraps the existing CMake build without adding library dependencies.
Its default build type is Debug so the current assertion-based tests execute.

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

CTest registers one executable as `kestrel_perft`. It asserts starting-position
counts of 20, 400, and 8902 at depths 1, 2, and 3. Run checks in Debug: builds that
define `NDEBUG`, including typical Release builds, disable these assertions.

```sh
make check
./build/kestrel perft 0
./build/kestrel perft 1
./build/kestrel perft 2
./build/kestrel perft 3
```

The reference outputs are `1`, `20`, `400`, and `8902`. Current fresh runs
produce `1`, `20`, `400`, and `3735`: depth three fails. `make check` also fails
at its depth-three assertion. This is an existing engine defect, exposed by
Debug validation; the documentation update does not change engine behavior.
The tests cover neither FEN validation nor complete state restoration, legal
moves, special moves, or search. Keep this coverage limit explicit when reporting
results. Depth four's legal reference count is 197281, but it is not a passing
requirement of the current bootstrap.

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
