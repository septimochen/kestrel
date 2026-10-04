# Kestrel — contributor and agent instructions

## Purpose and priorities

Kestrel is a chess engine and a practical course in modern C++ and chess.
Prioritize correctness, readability, testability, and incremental complexity,
then measure performance before optimizing. Preserve deterministic results for
fixed positions, search limits, and configuration.

Use C++23 by default, CMake 3.30+, Clang or GCC, and the standard library. No external
runtime or test library is required initially. C++26 is available as an opt-in experiment with `CXX_STANDARD=26`. Use verified
features that improve clarity; language-mode acceptance is not full conformance.
See [docs/modern-cpp.md](docs/modern-cpp.md) for local feature probes.

## Workflow

- Read [README.md](README.md) for current behavior and
  [docs/roadmap.md](docs/roadmap.md) for the actionable plan. Roadmap checkboxes
  are the single source of milestone status; this file records durable rules.
- Keep a root Makefile for common build, check, run, Perft, and format commands.
- Keep `.gitignore` in allowlist form. Explicitly allow new project inputs and
  leave generated outputs ignored.
- Implement one coherent learning step per change. Update affected docs and
  checkboxes only when implementation and validation support completion.
- Run appropriate checks, report existing failures accurately, and distinguish
  them from regressions. Never claim a milestone is complete because it compiles.
- Always commit completed tasks with a descriptive message and push when a Git
  remote exists. Stage only files belonging to the task.
- If auxiliary Python tooling is introduced, manage it with uv and add Ruff, ty,
  and pytest as development dependencies; do not add Python to the engine core.

```sh
make check
make run
make perft DEPTH=3
```

Direct build equivalent:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Tests use explicit checks that also run under `NDEBUG`. Starting Perft depths
0–3 and exact state restoration pass; legal filtering is still pending.

## Learning approach

For each milestone, explain the chess rule, the state invariant, the relevant
C++ concept, and how a test demonstrates it. Use short examples tied to real
source files. Let the user experiment with small positions and inspect results.
Do not introduce a language feature merely to demonstrate it.

Prefer value semantics, `std::array`, `std::vector`, `enum class`, `constexpr`,
references, and const correctness in the foundation. Practice recursion and
algorithms in Perft/search, RAII when rollback or resources need lifetime
management, and bit operations after correctness. Templates, concurrency,
atomics, SIMD, and custom allocation belong only where a measured need exists.

## Module boundaries

```text
CLI / future UCI → engine operations
                      ├── Board + Move
                      ├── move generation + attack detection
                      ├── Perft (validation consumer)
                      └── Search → evaluation + ordering + future TT
```

Perft and search are separate consumers of the chess core. Search does not
call Perft; evaluation does not depend on UCI. Keep input/output and protocol
handling outside board, generation, and search logic. Introduce modules such
as evaluation, attacks, hashing, and UCI only when their implementations need them.
Avoid a generic engine framework, inheritance hierarchy, or service layer.

Use namespace `kestrel`, executable `kestrel`, and library `kestrel_lib`.
Public headers live in `include/kestrel`, implementations in `src`, tests in
`tests`, and learning/design docs in `docs`.

## Board, moves, and state invariants

Start with `std::array<Piece, 64>`. Square mapping is `rank * 8 + file`, with
zero-based file/rank: a1 = 0, h1 = 7, a8 = 56, h8 = 63. White pawn movement
increases ranks. Use strongly typed colors and piece types; an empty piece has
`PieceType::None` and its color has no meaning.

Document bounds and preconditions. Validate external coordinates before indexing.
Do arithmetic in `int`, validate bounds, then convert to `Square`; prevent
unsigned wraparound. Include the standard headers each file directly needs.

Keep the readable `Move` struct initially. Its flags and promotion field must
be consistent. Kings are never captured. A validated external move should match
a generated legal move before mutation; internal make can assume a documented
valid move. A default `Move{}` is not a no-move sentinel: use `std::optional<Move>`
in a future search result, with score, completed depth, and node count as needed.

The board owns position state: pieces, side to move, castling rights, en passant
target, halfmove clock, and fullmove number. Add hashes only in the hashing stage.
Keep game history separate from a single position; repetition needs history,
not just a current board.

`makeMove` returns a state value consumed by the matching `undoMove`. Save the
original moving piece, captured piece and its actual square, prior metadata,
and enough rook state for castling. Never reconstruct captured state heuristically.
Update castling rights on king/rook movement and capture of a home-square rook.
A rook returning home does not regain rights. Promotion undo restores a pawn.

Reset the halfmove clock on pawn moves and captures; increment it otherwise.
Increment the fullmove number after Black moves. Exact undo includes both clocks.
Every successful traversal, early return, cutoff, or future cancellation must
restore the entry position. Start with explicit make/undo; add a small RAII
rollback guard if exit paths become difficult to audit.

## FEN and input handling

Parse all six FEN fields into a temporary position and commit only on success.
Reject invalid piece symbols, rank widths, side tokens, castling syntax,
en passant syntax, and invalid counters; reject unwanted trailing input.
Document structural validation separately from full legal reachability.

Playable positions require one king per color and consistent basic invariants;
allow deliberately incomplete teaching fixtures only through explicit test setup.
An en passant target can appear after a double push even when no capture is
available. Do not reject such FEN simply because no adjacent pawn can capture.
Define stricter historical-consistency checks explicitly rather than silently.

Keep parsing failure explicit. A bool is adequate initially; add an error enum
or result struct if useful diagnostics justify it. Mark meaningful return values
`[[nodiscard]]` when ignoring them would hide an error. Validate CLI depth and
numeric conversion; reject negative depth and report errors without uncaught
exceptions. Document a practical depth/resource limit when introducing one.

## Attacks and legal moves

Keep attack detection independent of legal move generation. Pawns attack
diagonally even when their target is empty; kings attack adjacent squares.
Pinned pieces still attack squares for king-safety purposes. Use the same
dedicated detector for check, king destinations, and castling safety.

Being in check is a valid game state. A move is illegal if it leaves the moving
side's king attacked; the side to move may need to evade an existing check.
Do not confuse legal check positions with malformed positions.

Generate ordinary pseudo-legal moves, make each candidate, test own king safety,
and undo. En passant must remove the captured pawn before testing discovered
attacks. Promotion has exactly queen, rook, bishop, and knight choices.

For orthodox castling, require rights, king/rook on their home squares, empty
intervening squares, and a safe king start, transit, and destination. Rook squares
need not be unattacked. Check transit with appropriate occupancy; testing only
the final king square is insufficient. Chess960 is outside the initial scope.

## Tests and correctness gates

Use executable-based tests with CTest. Replace assertion-only test checks with
explicit failure checks that remain active in Release. Failure messages should
identify the position, move, expected result, and observed result.

Test FEN success/failure and failure preservation; board invariants; all movement
and attack types; exact make/undo for both colors; nested sequences; captures;
all promotions; castling; en passant; pins; check evasions; mate and stalemate.
Compare full state, not only piece counts. Verify Perft and search preserve input.

Starting-position legal Perft references are 20, 400, 8902, and 197281 at depths
1–4. Add multiple known positions/depths for special rules and record the source
of each fixture. Add divide and compare branches when counts differ. Perft
counts legal move paths; do not apply repetition or move-count draw adjudication
to ordinary Perft. Optional differential testing may use another engine as a
development tool without making it a runtime dependency.

Add opt-in AddressSanitizer/UndefinedBehaviorSanitizer builds for supported
compilers. Verify Debug and Release once test checks work in both. Keep production
code free of undefined behavior; use profiling and reproducible benchmarks before
optimizing. Do not rewrite to bitboards merely because array Perft compiles.

## Search and game outcomes

Begin only after the legal-move and Perft foundation passes. Use a consistent
side-to-move evaluation and test sign changes. Implement reference negamax,
then compare alpha-beta against it on small positions.

Handle no legal moves before static leaf evaluation: checkmate when in check,
stalemate otherwise. Keep mate scores outside the material range and use ply
distance to prefer quicker mates and delay losses. Never evaluate kings as
ordinary capturable material.

Quiescence may start with captures and promotions, but when in check it must
search legal evasions and must not use stand-pat. Test recaptures and promotions.
Future time-limited search returns the last completed iteration, with a legal
fallback move when interrupted before completing one. Inject a stop/deadline
mechanism rather than hiding protocol logic in recursion.

Draw adjudication needs an explicit policy: repetition history, claimable draws,
automatic move-count draws, and dead positions are distinct. Track these before
claiming full game-rule support. Account for history/halfmove-sensitive results
when caching scores; a placement hash alone cannot encode every draw condition.

Use fixed-seed Zobrist keys. Specify the en passant treatment for repetition
identity (only legally available captures affect it) separately from a safe
transposition-table key policy. Test full recomputation against incremental keys.
Store TT depth, bound type, best move, and score; normalize mate-distance scores
across plies and verify collisions/entry replacement do not break correctness.

## C++ style and ownership

Prefer Rule of Zero classes whose members manage their own resources. Pass small
values by value, larger read-only objects by const reference, and mutable boards
by reference. References and spans borrow; never return them to expired storage.
Use `std::optional` for absence, `std::chrono::steady_clock` for elapsed time,
and `std::unique_ptr` only for actual dynamic ownership. Avoid owning raw pointers,
mutable globals, C-style casts, unnecessary macros, and premature templates.
Keep public headers self-contained. Explain representation tradeoffs when changing
an API or layout. A clearer loop is preferable to an opaque ranges expression.

## Further reading

- [FIDE Laws of Chess](https://handbook.fide.com/chapter/E012023): movement,
  attacks, check, castling, and game outcomes.
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines):
  value semantics, ownership, RAII, and Rule of Zero.

For the ordered learning plan and future advanced features, maintain
[docs/roadmap.md](docs/roadmap.md) rather than duplicating a second roadmap here.
