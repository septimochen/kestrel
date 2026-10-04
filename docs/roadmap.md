# Roadmap

Kestrel is at Stage 0, with its depth-three Perft validation still failing. The detailed design guidance lives in
[AGENTS.md](../AGENTS.md); this page records milestone outcomes and validation.

| Stage | Deliverable | Completion gate |
| --- | --- | --- |
| 0 — Bootstrap | C++20/CMake, array board, basic FEN, pseudo-legal moves, Perft skeleton | Starting-position depths 1–3 pass; current limitations documented |
| 1 — Board state | Exact make/undo, captures, original promotion piece, special moves, rights, clocks | Full-state round trips and nested sequences pass for both colors |
| 2 — Legal moves | Dedicated attack/check detection, legal filtering, castling, en passant | Tests cover pins, check evasions, king movement, and special-move safety |
| 3 — Perft | Broader reference positions and divide | Multiple depths pass for starting, castling, en passant, promotion, and check positions |
| 4 — Basic search | Material evaluation, negamax, then alpha-beta | Deterministic move selection, mate/stalemate handling, and board preservation tests |
| 5 — Search improvements | Iterative deepening, move ordering, quiescence, killer/history heuristics | Tactical tests and measured node counts for each incremental change |
| 6 — Hashing | Zobrist keys, transposition table, hash move ordering | Incremental/recomputed hashes agree; undo restores keys; cache bounds tested |
| 7 — Bitboards | Measured representation rewrite | Perft matches the array reference and benchmarks justify the change |
| 8 — UCI | Protocol layer, depth/time controls, stop/quit | Protocol and interruption tests; GUI integration verified |
| 9 — Stronger search | Investigate PVS, null move, reductions, pruning, aspiration windows, SEE | Each technique has correctness tests and before/after benchmarks |
| 10 — Evaluation | Piece-square tables, mobility, king/pawn features, endgame scaling | Focused evaluation tests and measured playing-strength changes |
| 11 — Advanced engine | Investigate NNUE, parallel search, books, tablebases, advanced attacks | Stable classical foundation and evidence for each added complexity |

## Immediate priorities

- Resolve the depth-three Perft mismatch (fresh run: 3735; reference: 8902).
- Save enough state to restore captured pieces and promotions exactly.
- Update castling rights when kings/rooks move or home-square rooks are captured.
- Track and restore halfmove and fullmove counters.
- Make FEN parsing strict and preserve the original board on failure.
- Test state restoration before introducing legal move filtering.
- Add attack detection, then legal moves and special-move generation.
- Expand Perft validation before implementing real search.

Optimization follows correctness, benchmarking, and profiling. Retain the simple
array implementation as a reference if it helps verify a later rewrite.
