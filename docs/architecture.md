# Architecture and API

## Source map

| Files | Responsibility |
| --- | --- |
| `include/kestrel/types.hpp` | Colors, piece types, pieces, squares, coordinate helpers |
| `include/kestrel/move.hpp`, `src/move.cpp` | Readable move structure; implementation file currently has no helpers |
| `include/kestrel/board.hpp`, `src/board.cpp` | Position storage, FEN loading, make/undo |
| `include/kestrel/movegen.hpp`, `src/movegen.cpp` | Pseudo-legal move generation |
| `include/kestrel/perft.hpp`, `src/perft.cpp` | Recursive node counting |
| `include/kestrel/search.hpp`, `src/search.cpp` | Placeholder move selection |
| `src/main.cpp` | Starting-position demo and Perft CLI |
| `tests/perft_tests.cpp` | Starting-position checks and board preservation |
| `tests/board_tests.cpp` | Move round trips, rights, clocks, and FEN validation |
| `examples/modern_cpp.cpp` | Optional C++23/C++26 feature experiments |

CMake builds `kestrel_lib`, then links the `kestrel` CLI and `kestrel_tests`
against it. Public headers live under `include/kestrel`; all engine types use
namespace `kestrel`.

## Position representation

`Board` owns `std::array<Piece, 64>`. A piece has a strongly typed `PieceType`
and `Color`; `PieceType::None` denotes an empty square. Its color is irrelevant.

Files and ranks are zero-based. `makeSquare(file, rank)` computes `rank * 8 + file`:

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

White pawns move toward increasing ranks; Black pawns move toward decreasing
ranks. `Square` is a `uint8_t`. Coordinate helpers and `pieceAt` do not check
bounds: callers must supply files/ranks in 0–7 and squares in 0–63.

A new `Board` contains the starting position. `clear()` empties it and resets
White to move, no castling rights, and en passant target `-1`. Castling rights
are four bits: White kingside, White queenside, Black kingside, Black queenside.
Halfmove and fullmove counters are stored; `clear()` resets them to 0 and 1.

## FEN loading

`Board::loadFen(const std::string&)` returns C++23
`std::expected<void, FenError>`. It parses all six fields into local state and
commits only on success. `bool setFromFen` wraps this result for simple callers.

```cpp
#include "kestrel/board.hpp"

kestrel::Board board;
auto result = board.loadFen(
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
if (!result) {
    // Inspect result.error(); the board is unchanged.
}
```

Validation rejects bad piece symbols, rank widths, side tokens, duplicate/invalid
castling rights, wrong en passant rank for the active side, malformed counters,
and extra fields. Counters use nonnegative halfmoves and positive fullmoves,
within int range. This is structural validation: king counts, historical
consistency, and legal reachability are not checked, allowing teaching fixtures.
An en passant target need not have an adjacent capturing pawn.

## Moves and state changes

`Move` stores `from`, `to`, an optional promotion type, and capture, en passant,
and castling flags. Generation produces promotions to queen, rook, bishop,
and knight. The public `makeMove` API does not validate movement or flags.

The intended traversal pattern is:

```cpp
#include "kestrel/board.hpp"
#include "kestrel/movegen.hpp"

kestrel::Board board;
auto moves = kestrel::generatePseudoLegalMoves(board);
for (const auto& move : moves) {
    auto state = board.makeMove(move);
    // Inspect the child position here.
    board.undoMove(move, state);
}
```

`BoardState` saves the original moving piece, captured piece and square, clocks,
side to move, en passant target, and castling rights. Undo restores this saved
state, including promotion captures and en passant. Castling relocates the rook;
the internal API assumes consistent move flags and empty rook transit/destination
squares. Bounds and move legality are caller preconditions.

Making a king/rook move or capturing a home-square rook removes the corresponding
castling rights. Pawn moves and captures reset the halfmove clock; other moves
increment it. Black's moves increment the fullmove number. Counters saturate at
int maximum rather than overflowing. Both-color special-move round trips and
nested sequences are tested with full `Board` equality.

## Generation and traversal

`generatePseudoLegalMoves(const Board&)` scans squares in ascending order,
selects the side to move's pieces, and applies movement rules. Sliding pieces
stop at the first occupied square. Friendly targets are excluded; enemy targets
are captures. Castling and en passant are not generated. Kings can move into
attack and moves can expose their own king; king captures are excluded.

Current data flow:

```text
CLI → Board → pseudo-legal generation → make → recurse → undo → node count
```

`perft(Board&, int)` returns one at depth zero and sums child counts otherwise.
It accepts a mutable board and restores it through undo. Starting depths 0–3
and complete board preservation are tested. Negative depth throws
`std::invalid_argument`; the CLI rejects malformed depths before traversal.

`Search::findBestMove(Board&, int)` returns the first pseudo-legal move or a
default `Move{}` when none exists. Depth is ignored. A default move is not an
explicit game-over result; real search must define terminal-position handling.

The next foundation adds attack detection and legal filtering. Evaluation and search should remain separate from
board storage; a future UCI layer should call the engine core.
