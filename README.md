# Kestrel

A Chess Engine Built by Modern C++

## Current milestone

- Board representation using a simple 64-square array
- FEN parsing
- Move representation
- Pseudo-legal move generation
- Basic `makeMove`
- Perft
- Minimal test suite
- Placeholder search interface

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run:

```bash
./build/kestrel
```

Perft:

```bash
./build/kestrel perft 1
./build/kestrel perft 2
./build/kestrel perft 3
```

## Roadmap

1. Correct legal move generation
2. Make/undo with complete state restoration
3. Perft through castling, en passant, and promotion
4. Negamax
5. Alpha-beta pruning
6. Evaluation
7. Iterative deepening
8. Quiescence search
9. Move ordering
10. Zobrist hashing and transposition tables
11. Bitboards
12. UCI
13. Parallel search
14. Stronger evaluation / NNUE

Kestrel intentionally starts with a readable board representation.
Performance-oriented representations will come later.
