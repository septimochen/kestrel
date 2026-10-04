# Roadmap and learning checklist

Kestrel has a C++23 bootstrap and tested reversible board state. Starting Perft
depths 0–3 pass in Debug, Release, C++26 mode, and sanitizer builds. Checkboxes track verified work; a checked item does
not imply its entire stage is complete. [AGENTS.md](../AGENTS.md) contains the
contributor rules; [architecture.md](architecture.md) describes current behavior.

## Next work, in order

- [x] Reproduce depth-three failure and add a focused capture make/undo regression.
- [x] Restore original pieces and captured squares; test quiet moves, captures,
  promotions, and nested make/undo for both colors.
- [x] Re-run starting Perft depths 1–3 and verify the entire input board is unchanged.
- [x] Complete special-move state, castling rights, and clocks, with round-trip tests.
- [x] Make FEN loading structurally strict and transactional.
- [ ] Add dedicated attack detection.
- [ ] Implement legal filtering and special-move generation; validate deeper Perft.

The capture-restoration defect is fixed. Passing shallow counts still does not
prove legal generation; attack detection is next.

## Stage 0 — Bootstrap and useful diagnostics

Learn C++ translation units, headers, linking, build configurations, and chess
coordinates. Explain why a legal move count differs from a movement-only count.

- [x] Establish C++23/CMake library, CLI, and CTest executable.
- [x] Add array board, readable pieces/moves, and ordinary pseudo-legal generation.
- [x] Document implemented behavior, known limits, and build commands.
- [x] Replace `assert` test checks with checks that execute under `NDEBUG`.
- [x] Make test failures identify the fixture and expected/observed result.
- [x] Validate CLI arguments and reject negative/non-integer/overflowing depth.
- [x] Fix existing compiler warnings without unrelated formatting changes.
- [x] Add opt-in sanitizer configuration and document Debug/Release checks.

Gate: test failures remain visible in every configuration; invalid input exits
cleanly. The Perft foundation is completed in Stages 1–3.

## Stage 1 — Exact position state

Learn value semantics, structs/classes, const references, enums, integer bounds,
and transactional updates. Explain irreversible chess metadata and why undo
must save information instead of guessing it.

- [x] Define full-position comparison for tests (all squares and metadata).
- [x] Save original moving piece, captured piece/square, and prior metadata.
- [x] Restore ordinary captures, all promotion choices, and promotion captures.
- [x] Restore en passant and both castling rook relocations exactly.
- [x] Update castling rights on king/rook moves and home-rook captures.
- [x] Track halfmove/fullmove counters and restore them on undo.
- [x] Test each move type for White and Black plus nested move sequences.
- [x] Parse all six FEN fields, reject malformed input, and preserve state on failure.
- [ ] Add FEN serialization and round-trip tests to make fixtures inspectable.
- [x] Confirm starting depths 1–3 and state preservation after each traversal.

Gate: every tested make/undo sequence restores exactly; FEN errors leave the
previous board intact. Serialization is planned, not currently implemented.

## Stage 2 — Attacks and legal chess

Learn const APIs, small helper functions, array traversal, and separation of
responsibilities. Study check, pins, discovered attacks, and king safety.

- [ ] Detect pawn, knight, slider, and king attacks independently of legal moves.
- [ ] Test empty-square pawn attacks and attacks by pinned pieces.
- [ ] Locate kings and detect check; support valid positions where a side is in check.
- [ ] Filter candidates by make/check/undo; never generate king captures.
- [ ] Test pins, single/double check, evasions, and king adjacency.
- [ ] Generate en passant and test discovered-check rejection.
- [ ] Generate both castling sides with rights, rook presence, emptiness, and
  start/transit/destination safety tests.
- [ ] Test all promotions and underpromotions for both colors.
- [ ] Verify legal generation preserves every part of the input board.

Gate: legal moves always preserve own king safety; all special-rule tests pass.

## Stage 3 — Perft validation and debugging

Learn recursion, accumulation, state lifetimes, and reproducible experiments.
Explain how branch counts isolate rule and state-management bugs.

- [ ] Use legal generation in Perft, with depth zero returning one.
- [ ] Validate starting depths 1–4: 20, 400, 8902, 197281.
- [ ] Add sourced fixtures at multiple depths for castling, en passant,
  promotions, checks, and pins.
- [ ] Add coordinate move notation and Perft divide, with sums matching total Perft.
- [ ] Test that repeated runs return the same count and preserve the board.
- [ ] Run Debug, Release, and supported sanitizer checks.
- [ ] Record a reproducible baseline: position, depth, compiler, build type,
  hardware, elapsed time, and nodes/second.

Gate: all reference fixtures pass and no sanitizer/state-restoration defect is
known. Ordinary Perft does not stop for repetition or move-count draws.

## Stage 4 — A first playable engine

Learn recursive score propagation, explicit result types, `std::optional`, and
algorithm comparison. Study material, mate, stalemate, and tactical choices.

- [ ] Implement simple material evaluation from the side-to-move perspective.
- [ ] Test color/sign symmetry and material changes.
- [ ] Define a result with optional best move, score, completed depth, and nodes.
- [ ] Implement reference negamax with mate/stalemate handling at leaf boundaries.
- [ ] Prefer faster mates and delayed losses using consistent mate-distance scores.
- [ ] Add alpha-beta and compare scores with reference negamax on small positions.
- [ ] Test legal move selection, determinism, terminal results, and board preservation.
- [ ] Add a simple depth-limited CLI search command and document examples.

Gate: a legal, deterministic best move is available; mate/stalemate are explicit
and alpha-beta agrees with the reference implementation.

## Stage 5 — Reliable tactical search

Learn sorting, stable tie-breaking, RAII rollback where useful, and
`std::chrono::steady_clock`. Study recaptures and the horizon effect.

- [ ] Add iterative deepening and retain the last completed iteration.
- [ ] Add deterministic capture/promotion ordering and measure node reduction.
- [ ] Add quiescence with captures/promotions, all legal evasions when in check,
  and no stand-pat while in check.
- [ ] Add tactical tests for recaptures, underpromotions, and checking sequences.
- [ ] Add optional killer/history ordering separately, with benchmarks.
- [ ] Add cancellation/deadline handling and a legal fallback move.
- [ ] Verify board restoration after cutoffs, timeouts, and early returns.

Gate: tactical tests pass; interruption preserves state and returns usable results.

## Stage 6 — Hashing and game history

Learn fixed-width integers, bitwise XOR, reproducible key generation, bounded
storage, and cache contracts. Study transpositions and repetition identity.

- [ ] Define fixed-seed Zobrist keys and test full recomputation.
- [ ] Update/restore hashes through every move type and nested sequences.
- [ ] Track game history separately and test repetition identity, including
  castling rights and legally available en passant.
- [ ] Define claimable/automatic draw policy and document dead-position limits.
- [ ] Keep history/halfmove-sensitive draw results safe when probing/storing TT scores.
- [ ] Add TT key, depth, score, bound, and best move with explicit replacement rules.
- [ ] Normalize mate scores across search plies; test bounds and collisions.
- [ ] Compare TT on/off results, hash ordering, memory use, and hit rates.

Gate: hashes round-trip exactly; TT preserves search correctness; supported draw
rules and remaining omissions are documented.

## Stage 7 — Measured bitboard experiment

Learn unsigned shifts, masks, `std::popcount`, bit scans, layout, and profiling.

- [ ] Profile the correct array implementation and record bottlenecks.
- [ ] Add bitboard helpers with edge-square and shift-boundary tests.
- [ ] Keep an array reference or reference fixtures for differential checks.
- [ ] Match all legal-move, make/undo, and Perft results.
- [ ] Compare speed and memory before adopting a replacement representation.

Gate: correctness is unchanged and measurements explain the representation choice.
This is an experiment after the foundation; UCI need not wait for a rewrite.

## Stage 8 — UCI and playing games

Learn parsing, streams, process lifetimes, and eventually `std::jthread`/stop tokens
when responsive protocol handling requires a search worker. Study time controls.

- [ ] Separate UCI parsing/output from engine operations.
- [ ] Implement `uci`, `isready`, `ucinewgame`, `position`, and `quit`.
- [ ] Support startpos/FEN plus legal move application with clear error handling.
- [ ] Implement depth/time searches, responsive `stop`, and `bestmove`.
- [ ] Test command transcripts, cancellation, new-game reset, and terminal positions.
- [ ] Connect to a GUI and record a reproducible integration test.

Gate: the GUI can complete games using the documented rule/time-control support.
Protocol responsiveness is separate from parallelizing the search tree.

## Stages 9–11 — Optional strength and systems work

These are investigation topics after the playable foundation. Each change needs
focused tests and before/after measurements; not every topic must be implemented.

- [ ] Stage 9: investigate PVS, aspiration windows, SEE, then selective search
  techniques such as null move, reductions, and futility pruning one at a time.
- [ ] Stage 10: add piece-square tables, mobility, pawn structure, king safety,
  rook activity, bishop pair, and endgame scaling in separate measured steps.
- [ ] Establish repeatable tactical and playing-strength comparisons.
- [ ] Stage 11: investigate books/tablebases, parallel search, advanced attack
  tables, SIMD, or NNUE only when the existing design and measurements justify it.

## Completing a learning step

- [ ] Explain the chess rule and C++ concept in your own words, with a small example.
- [ ] Link the explanation to the implementation and a test that would catch a bug.
- [ ] Run the relevant checks and record any unresolved limitation.
- [ ] Update docs/status, then commit and push the focused change.

References: [FIDE Laws of Chess](https://handbook.fide.com/chapter/E012023) and
[C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines).
