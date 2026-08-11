# Kestrel — AGENTS.md

## Project Overview

Kestrel is a C++ chess engine

Kestrel should prioritize **correctness, readability, testability, and incremental complexity** before optimization.

The engine should eventually progress from a simple educational implementation to a reasonably strong high-performance chess engine.

---

## Technology

- Language: Modern C++
- Build system: CMake
- Testing: CTest / executable-based tests
- Primary platform: macOS
- Compiler: Clang or GCC
- Standard library: C++ standard library
- No external dependencies should be required initially.

Build:

```bash
cmake -S . -B build
cmake --build build
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

Run the engine:

```bash
./build/kestrel
```

Run Perft:

```bash
./build/kestrel perft 1
./build/kestrel perft 2
./build/kestrel perft 3
```

---

# Architecture

The engine should evolve around these major components:

```text
                         Kestrel
                            │
              ┌─────────────┴─────────────┐
              │                           │
           Position                      Move
              │                           │
              └─────────────┬─────────────┘
                            │
                            ▼
                       Move Generation
                            │
                            ▼
                       Make / Undo
                            │
                            ▼
                           Perft
                            │
                            ▼
                          Search
                            │
              ┌─────────────┼──────────────┐
              ▼             ▼              ▼
          Evaluation    Move Ordering   Hashing
              │             │              │
              └─────────────┼──────────────┘
                            ▼
                    Transposition Table
                            │
                            ▼
                          UCI
```

Eventually:

```text
Board
  ↓
Move Generation
  ↓
Make / Undo
  ↓
Search
  ├── Negamax
  ├── Alpha-Beta
  ├── Iterative Deepening
  ├── Quiescence Search
  ├── Move Ordering
  └── Transposition Table
  ↓
Evaluation
  ├── Classical evaluation
  └── NNUE
  ↓
UCI
```

---

# Current Project Structure

The initial project should use:

```text
kestrel/
├── CMakeLists.txt
├── README.md
├── AGENTS.md
├── LICENSE
│
├── include/
│   └── kestrel/
│       ├── types.hpp
│       ├── board.hpp
│       ├── move.hpp
│       ├── movegen.hpp
│       ├── perft.hpp
│       └── search.hpp
│
├── src/
│   ├── main.cpp
│   ├── board.cpp
│   ├── move.cpp
│   ├── movegen.cpp
│   ├── perft.cpp
│   └── search.cpp
│
└── tests/
    └── perft_tests.cpp
```

As the engine grows, additional modules may be introduced:

```text
evaluation.hpp/cpp
search.hpp/cpp
zobrist.hpp/cpp
transposition_table.hpp/cpp
uci.hpp/cpp
bitboard.hpp/cpp
attacks.hpp/cpp
```

Do not create abstractions prematurely.

---

# Development Principles

## 1. Correctness before performance

The first implementation should be easy to understand.

Do not immediately optimize using:

- bitboards
- SIMD
- complicated templates
- custom allocators
- lock-free data structures
- assembly
- advanced search heuristics

First make the engine correct.

Optimization comes after profiling.

---

## 2. Keep the engine deterministic

Given:

```text
same position
same search parameters
same engine configuration
```

the engine should produce the same result unless nondeterminism is intentionally introduced.

Avoid unnecessary global state.

---

## 3. Test every major subsystem

Important components must have tests.

Especially:

- FEN parsing
- board representation
- move generation
- make/undo
- attack detection
- check detection
- castling
- en passant
- promotion
- Perft
- search
- hashing

Chess engines are particularly sensitive to tiny state-management bugs.

---

# Chess Board Representation

## Initial implementation

Use a simple 64-square array.

```cpp
std::array<Piece, 64>
```

Square mapping:

```text
8  56 57 58 59 60 61 62 63
7  48 49 50 51 52 53 54 55
6  40 41 42 43 44 45 46 47
5  32 33 34 35 36 37 38 39
4  24 25 26 27 28 29 30 31
3  16 17 18 19 20 21 22 23
2   8  9 10 11 12 13 14 15
1   0  1  2  3  4  5  6  7
    a  b  c  d  e  f  g  h
```

Therefore:

```cpp
a1 == 0
b1 == 1
...
h1 == 7

a8 == 56
...
h8 == 63
```

Helper functions:

```cpp
constexpr Square makeSquare(int file, int rank) {
    return static_cast<Square>(rank * 8 + file);
}

constexpr int fileOf(Square square) {
    return square % 8;
}

constexpr int rankOf(Square square) {
    return square / 8;
}
```

---

# Pieces

Use strongly typed enums:

```cpp
enum class Color : uint8_t {
    White,
    Black
};

enum class PieceType : uint8_t {
    None,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};
```

A piece initially consists of:

```cpp
struct Piece {
    PieceType type = PieceType::None;
    Color color = Color::White;

    constexpr bool empty() const {
        return type == PieceType::None;
    }
};
```

Do not encode pieces into complicated integers until there is a demonstrated need.

---

# Move Representation

The initial representation should prioritize readability:

```cpp
struct Move {
    Square from = 0;
    Square to = 0;

    PieceType promotion = PieceType::None;

    bool isCapture = false;
    bool isEnPassant = false;
    bool isCastling = false;
};
```

Eventually this can become a compact integer representation.

Possible future representation:

```text
bits:
from square
to square
promotion
special move flags
```

But this should only happen after the basic engine is correct.

---

# Board State

The board must track at least:

```text
pieces
side to move
castling rights
en passant square
```

Later it should also track:

```text
halfmove clock
fullmove number
Zobrist hash
```

A make/undo operation should preserve all reversible state.

The target API is:

```cpp
BoardState state = board.makeMove(move);

...

board.undoMove(move, state);
```

The state object should eventually contain enough information to restore the board **exactly**.

This includes captured pieces.

Never rely on reconstructing state heuristically during undo.

---

# FEN

Kestrel should support FEN input.

Starting position:

```text
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
```

The FEN parser should eventually support all six fields:

```text
1. piece placement
2. side to move
3. castling availability
4. en passant target square
5. halfmove clock
6. fullmove number
```

Malformed FEN should be rejected.

---

# Move Generation

Move generation should be implemented incrementally.

Recommended order:

```text
1. Pawn
2. Knight
3. Bishop
4. Rook
5. Queen
6. King
7. Captures
8. Promotion
9. En passant
10. Castling
11. Check detection
12. Legal move filtering
```

Separate:

```text
pseudo-legal moves
```

from:

```text
legal moves
```

Pseudo-legal moves satisfy movement rules but may leave the king in check.

Legal moves must never leave the moving side's king in check.

---

# Pawn Rules

Pawns require special handling.

White moves toward increasing ranks.

Black moves toward decreasing ranks.

Support:

```text
single push
double push
diagonal capture
promotion
en passant
```

Promotion choices:

```text
Queen
Rook
Bishop
Knight
```

There is no promotion to king or pawn.

---

# Sliding Pieces

Bishop directions:

```text
(+1,+1)
(+1,-1)
(-1,+1)
(-1,-1)
```

Rook directions:

```text
(+1,0)
(-1,0)
(0,+1)
(0,-1)
```

Queen uses both.

Sliding generation must stop at the first occupied square.

If that square contains an enemy piece, it may be captured.

If it contains a friendly piece, movement stops without adding the square.

---

# King

The king moves one square in any direction.

Legal king movement must eventually check whether the destination square is attacked.

Castling requires:

```text
king has not moved
rook has not moved
squares between king and rook are empty
king is not currently in check
king does not cross an attacked square
king does not end on an attacked square
```

---

# Attack Detection

Implement a dedicated attack detector.

Conceptually:

```cpp
bool isSquareAttacked(
    const Board& board,
    Square square,
    Color byColor
);
```

This function is fundamental.

It should detect attacks from:

```text
pawns
knights
bishops
rooks
queens
king
```

Once implemented, it should be reused for:

```text
check detection
legal move filtering
king movement
castling
```

Avoid duplicating attack logic in multiple places.

---

# Check Detection

Implement:

```cpp
bool isInCheck(
    const Board& board,
    Color color
);
```

A position is illegal if the side to move's king is attacked.

Legal move generation can initially be implemented as:

```text
generate pseudo-legal moves
        ↓
make move
        ↓
if own king is not in check
        ↓
keep move
        ↓
undo move
```

This is not necessarily the fastest approach, but it is easy to verify.

Optimize only later.

---

# Perft

Perft is one of the most important components of Kestrel.

It counts the number of leaf nodes reachable at a given depth.

Basic implementation:

```cpp
uint64_t perft(Board& board, int depth) {
    if (depth == 0)
        return 1;

    uint64_t nodes = 0;

    auto moves = generateLegalMoves(board);

    for (const Move& move : moves) {
        auto state = board.makeMove(move);
        nodes += perft(board, depth - 1);
        board.undoMove(move, state);
    }

    return nodes;
}
```

Starting position expected results:

```text
depth 1 = 20
depth 2 = 400
depth 3 = 8902
depth 4 = 197281
```

More depths should be added as the implementation becomes correct.

Perft must be correct before serious search development begins.

---

# Perft Debugging

A useful future feature is:

```text
perft divide
```

Instead of:

```text
depth 3 = 8902
```

produce:

```text
a2a3 380
a2a4 420
b2b3 420
...
```

This makes it possible to identify exactly which move produces an incorrect subtree.

---

# Search

The initial search should use negamax.

Conceptually:

```text
search(position)
    |
    +-- generate legal moves
    |
    +-- make move
    |
    +-- search child position
    |
    +-- undo move
    |
    +-- choose best score
```

Eventually:

```cpp
int negamax(
    Board& board,
    int depth,
    int alpha,
    int beta
);
```

Use the side-to-move perspective:

```text
score(position) =
    max(-score(child))
```

This makes the implementation considerably simpler than maintaining separate maximizing/minimizing code.

---

# Alpha-Beta

After basic negamax, implement alpha-beta pruning.

Conceptually:

```cpp
int negamax(
    Board& board,
    int depth,
    int alpha,
    int beta
) {
    if (depth == 0)
        return evaluate(board);

    int best = -INF;

    for (const Move& move : generateLegalMoves(board)) {
        auto state = board.makeMove(move);

        int score = -negamax(
            board,
            depth - 1,
            -beta,
            -alpha
        );

        board.undoMove(move, state);

        best = std::max(best, score);
        alpha = std::max(alpha, score);

        if (alpha >= beta)
            break;
    }

    return best;
}
```

Alpha-beta should be introduced only after basic negamax works.

---

# Evaluation

Start extremely simple.

Material values:

```text
Pawn   = 100
Knight = 320
Bishop = 330
Rook   = 500
Queen  = 900
King   = very large / handled separately
```

Evaluation should be from White's perspective or, preferably, consistently from the side-to-move perspective depending on the search design.

Eventually add:

```text
material
piece-square tables
mobility
king safety
pawn structure
passed pawns
isolated pawns
doubled pawns
bishop pair
rook activity
space
tempo
```

Do not implement NNUE until the classical engine is already working.

---

# Iterative Deepening

Search progressively:

```text
depth 1
depth 2
depth 3
depth 4
...
```

Advantages:

- always have a move available
- improves move ordering
- works naturally with time controls
- provides predictable search behavior

The engine should eventually support a search deadline.

---

# Quiescence Search

A normal fixed-depth search can stop during a tactical sequence.

Example:

```text
Queen captures piece
    ↓
depth reaches zero
    ↓
evaluation happens immediately
```

This can produce severe horizon effects.

Quiescence search should extend tactical positions using moves such as:

```text
captures
promotions
possibly checks
```

The first implementation can search captures only.

---

# Move Ordering

Move ordering is critical for alpha-beta performance.

Eventually prioritize:

```text
1. Transposition-table / hash move
2. Winning captures
3. Promotions
4. Killer moves
5. History heuristic
6. Quiet moves
```

Good move ordering can drastically reduce the number of searched nodes.

---

# Zobrist Hashing

Each chess position should eventually have a 64-bit Zobrist hash.

Generate random keys for combinations such as:

```text
piece × color × square
side to move
castling rights
en passant file
```

Then a position can be represented by:

```cpp
uint64_t hash;
```

The hash must change incrementally when making/undoing moves.

---

# Transposition Table

Different move sequences can lead to the same chess position.

Example:

```text
A → B → C

A → C → B
```

may result in the same position.

Store previously searched positions:

```text
hash
depth
score
bound
best move
```

A future entry might look like:

```cpp
struct TTEntry {
    uint64_t key;
    int depth;
    int score;
    Move bestMove;
    Bound bound;
};
```

The transposition table should be implemented only after Zobrist hashing works.

---

# Bitboards

Once the array-based implementation is correct, introduce bitboards.

A bitboard is:

```cpp
using Bitboard = uint64_t;
```

Each bit corresponds to one square.

Eventually the position can contain:

```cpp
Bitboard whitePieces;
Bitboard blackPieces;

Bitboard pawns;
Bitboard knights;
Bitboard bishops;
Bitboard rooks;
Bitboard queens;
Bitboard kings;
```

Bitboards enable extremely efficient operations using:

```text
&
|
^
~
<<
>>
popcount
bit scans
```

This is an important Kestrel learning milestone.

Do not replace the simple board representation before Perft is correct.

---

# Performance Engineering

Performance work should follow:

```text
correctness
    ↓
benchmark
    ↓
profile
    ↓
identify bottleneck
    ↓
optimize
    ↓
benchmark again
```

Do not optimize based purely on intuition.

Important metrics:

```text
nodes searched
nodes/second
search depth
branching factor
transposition-table hit rate
cutoff rate
move-generation time
evaluation time
```

Perft is especially useful as a move-generation benchmark.

---

# UCI

Eventually Kestrel should implement the Universal Chess Interface.

Important commands include:

```text
uci
isready
ucinewgame
position
go
stop
quit
```

Example:

```text
position startpos
go depth 10
```

Response:

```text
bestmove e2e4
```

The UCI layer should be separate from the chess engine core.

Architecture:

```text
UCI
 │
 ▼
Engine API
 │
 ├── Board
 ├── MoveGen
 ├── Search
 └── Evaluation
```

The engine should not depend on a GUI.

---

# Future Advanced Features

After the classical engine is strong enough, investigate:

```text
Opening book
Syzygy tablebases
Parallel search
Lazy SMP
NNUE
SIMD
Advanced bitboard attacks
Magic bitboards
CPU-specific optimization
```

These are later milestones.

Do not introduce them prematurely.

---

# Suggested Complete Roadmap

## Stage 0 — Bootstrap

Current:

```text
CMake
C++20
project structure
basic Board
Piece
Move
FEN
pseudo-legal moves
Perft skeleton
tests
```

---

## Stage 1 — Correct Board State

Implement:

```text
complete makeMove
complete undoMove
captured piece restoration
castling state
en passant state
halfmove clock
fullmove number
```

Every make/undo pair must restore the exact original position.

---

## Stage 2 — Legal Move Generation

Implement:

```text
attack detection
king location
check detection
legal move filtering
castling
en passant
promotion
```

Then validate using Perft.

---

## Stage 3 — Perft Validation

Validate:

```text
starting position
castling positions
en passant positions
promotion positions
check positions
pin positions
```

Add Perft Divide.

Do not begin serious search until this stage is reliable.

---

## Stage 4 — Basic Search

Implement:

```text
evaluation
negamax
alpha-beta
```

The engine should be able to choose a move.

---

## Stage 5 — Search Improvements

Implement in this order:

```text
iterative deepening
move ordering
quiescence search
killer moves
history heuristic
```

---

## Stage 6 — Hashing

Implement:

```text
Zobrist hashing
transposition table
hash move ordering
```

---

## Stage 7 — Bitboards

Reimplement board representation using bitboards.

Compare:

```text
array implementation
vs
bitboard implementation
```

Benchmark both.

The old implementation can be retained as a reference if useful.

---

## Stage 8 — UCI

Implement:

```text
uci
isready
ucinewgame
position
go depth
go movetime
stop
quit
bestmove
```

Then connect Kestrel to a chess GUI.

---

## Stage 9 — Stronger Search

Investigate:

```text
Principal Variation Search
Null Move Pruning
Late Move Reductions
Futility Pruning
Razoring
Aspiration Windows
Static Exchange Evaluation
```

Only introduce each technique with tests and benchmarks.

---

## Stage 10 — Evaluation

Improve evaluation:

```text
piece-square tables
mobility
king safety
pawn structure
passed pawns
rook activity
bishop pair
endgame scaling
```

---

## Stage 11 — Advanced Engine

Eventually:

```text
NNUE
parallel search
opening book
Syzygy tablebases
advanced bitboard attacks
```

---

# Coding Style

Prefer clear modern C++.

Use:

```cpp
std::array
std::vector
std::string
std::span
std::optional
std::unique_ptr
constexpr
enum class
```

where appropriate.

Prefer RAII.

Avoid:

```text
raw owning pointers
global mutable state
unnecessary macros
C-style casts
premature template metaprogramming
unnecessary inheritance
```

Prefer composition over inheritance.

Use `const` appropriately.

Use references where ownership is not transferred.

Use `std::unique_ptr` only when dynamic ownership is actually needed.

Do not add abstractions merely to demonstrate a C++ feature.

---

# Error Handling

Public parsing functions should communicate failure clearly.

For example:

```cpp
bool setFromFen(const std::string& fen);
```

Later, if appropriate, use:

```cpp
std::expected
```

or another explicit error mechanism.

Do not silently accept malformed FEN.

---

# Naming

Project name:

```text
Kestrel
```

Namespace:

```cpp
namespace kestrel
```

Executable:

```text
kestrel
```

Library:

```text
kestrel_lib
```

Use descriptive names.

Examples:

```text
Board
Move
Piece
Square
generateLegalMoves
generatePseudoLegalMoves
isSquareAttacked
isInCheck
makeMove
undoMove
perft
negamax
evaluate
```

---

# Important Design Rule

Kestrel is a learning project.

When there is a choice between:

```text
shorter but opaque implementation
```

and:

```text
slightly longer but understandable implementation
```

prefer the understandable implementation during early stages.

After correctness is established, performance-oriented rewrites are encouraged.

This makes the project useful both as a chess engine and as a way to learn C++.

---

# Relationship to C++ Learning

Kestrel should be used to practice:

```text
C++20
RAII
references
const correctness
structs/classes
enum class
STL containers
algorithms
templates
bit manipulation
recursion
memory/layout
benchmarking
profiling
multithreading
atomics
```

The project should not force a C++ feature into the code when the feature does not naturally belong there.

---

# Relationship to Systems Programming

Kestrel should eventually provide hands-on experience with:

```text
CPU cache behavior
data-oriented design
memory layout
branch prediction
bit operations
hash tables
parallel computation
profiling
benchmarking
serialization
protocol implementation
```

The progression should be:

```text
Readable implementation
        ↓
Correct implementation
        ↓
Measured implementation
        ↓
Optimized implementation
```

---

# Relationship to llama.cpp Learning

Kestrel and llama.cpp can be used as complementary systems-programming projects.

Kestrel emphasizes:

```text
algorithms
search
recursion
bitboards
hashing
heuristics
game trees
```

llama.cpp emphasizes:

```text
tensor computation
memory management
SIMD
quantization
neural networks
parallelism
CPU/GPU execution
```

Together they provide a broad path toward high-performance C++.

---

# What NOT to Do

Do not:

- start by copying Stockfish
- implement NNUE first
- implement bitboards before understanding the simple board
- optimize before profiling
- skip Perft
- skip make/undo tests
- use undefined behavior for speed
- introduce global state unnecessarily
- make the UCI layer responsible for chess logic
- mix search logic with board representation
- assume pseudo-legal moves are legal
- trust a single Perft depth as proof of correctness

---

# Definition of Done for a Milestone

A milestone is not complete merely because the code compiles.

A milestone should have:

```text
implementation
tests
correctness validation
reasonable API
documentation where necessary
no known state-corruption bugs
```

For chess-specific functionality, use known Perft positions whenever possible.

---

# Immediate Next Task

The first serious Kestrel task after the bootstrap is:

```text
Implement complete makeMove/undoMove state restoration.
```

Then:

```text
Implement attack detection.
```

Then:

```text
Implement legal move generation.
```

Then:

```text
Make Perft pass known chess positions.
```

Only after these are complete should search be implemented.

The immediate target is:

```text
Board
  ↓
Pseudo-legal moves
  ↓
Make / Undo
  ↓
Attack detection
  ↓
Legal moves
  ↓
Perft
  ↓
Correct chess engine foundation
```

Once that foundation is correct, Kestrel can evolve into:

```text
Perft
  ↓
Negamax
  ↓
Alpha-Beta
  ↓
Iterative Deepening
  ↓
Quiescence
  ↓
Move Ordering
  ↓
Zobrist
  ↓
Transposition Table
  ↓
Bitboards
  ↓
UCI
  ↓
Advanced Search
  ↓
NNUE
```