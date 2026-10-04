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
| `tests/perft_tests.cpp` | Starting-position assertions |

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
Halfmove and fullmove counters are not stored yet.

## FEN loading

`bool Board::setFromFen(const std::string&)` reads placement, side to move,
castling rights, and en passant target. A standard six-field FEN is accepted,
but the last two fields are ignored:

```cpp
#include "kestrel/board.hpp"

kestrel::Board board;
bool loaded = board.setFromFen(
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
```

Check the return value. Loading clears the board before parsing; failure can
leave a partially loaded position and does not preserve the previous board.
Validation checks rank widths, side tokens, castling characters, and coordinate
syntax, but unknown piece characters become empty squares. It does not validate
king counts, position legality, en passant rank consistency, or clock fields.
Strict, transactional parsing remains future work.

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

This is currently reliable only for ordinary non-capturing, non-promoting moves.
`BoardState` saves side to move, en passant target, and castling rights; it saves
neither captured pieces nor the original moving piece. Undo clears the destination
instead of restoring an ordinary captured piece and returns a promoted piece
instead of a pawn. En passant undo reconstructs a pawn rather than saving it.
Castling moves relocate the rook, but make does not update castling rights when
kings or rooks move or rooks are captured.

## Generation and traversal

`generatePseudoLegalMoves(const Board&)` scans squares in ascending order,
selects the side to move's pieces, and applies movement rules. Sliding pieces
stop at the first occupied square. Friendly targets are excluded; enemy targets
are captures. Castling and en passant are not generated. Kings can move into
attack and moves can expose their own king; king captures are not excluded.

Current data flow:

```text
CLI → Board → pseudo-legal generation → make → recurse → undo → node count
```

`perft(Board&, int)` returns one at depth zero and sums child counts otherwise.
It accepts a mutable board and relies on undo for restoration, so existing undo
limitations also affect Perft. Use nonnegative depths only.

`Search::findBestMove(Board&, int)` returns the first pseudo-legal move or a
default `Move{}` when none exists. Depth is ignored. A default move is not an
explicit game-over result; real search must define terminal-position handling.

The next foundation adds attack detection and legal filtering after state
restoration is correct. Evaluation and search should remain separate from
board storage; a future UCI layer should call the engine core.
