# Bootstrap correctness review

Reviewed and repaired on 2026-10-04. This review covers the current array-based
bootstrap, not a complete legal chess engine.

| Finding | Repair and evidence |
| --- | --- |
| Undo erased captured pieces, corrupting sibling Perft branches | Save captured piece/square; nested capture regression; starting depth 3 now returns 8902 instead of 3735 |
| Undo returned promoted pieces instead of pawns | Save original moving piece; all four promotions and promotion captures tested for both colors |
| En passant undo guessed captured state | Restore saved piece at its actual square; both-color round trips tested |
| Castling rights never changed | Clear rights for king/rook moves and home-rook captures; both sides/colors tested |
| Clocks were ignored | Parse, update, and undo halfmove/fullmove counters; saturate at int maximum to avoid overflow |
| FEN accepted unknown pieces and malformed rights; errors mutated state | Six-field transactional structural parser with typed expected errors; rejection and preservation tests |
| Negative Perft recursively decremented without a base case | Library throws `invalid_argument`; CLI validates full integer input with `from_chars` |
| CLI numeric errors escaped as exceptions; trailing garbage was accepted | Explicit usage/errors for invalid commands, arity, negative, malformed, and overflowing numbers |
| Release builds disabled assertion-only tests | Explicit test failures independent of `NDEBUG`; Release CTest now verifies real behavior |
| Move generation could capture an enemy king | Exclude king destinations; regression test |
| Knight tables warned about scalar braces; abs relied on indirect headers | Simplify initializers and include the required standard header |

Validation: C++23 Debug/Release, C++26 Debug, and C++23 AddressSanitizer plus
UndefinedBehaviorSanitizer tests pass. Feature examples compile and run in
C++23 and C++26 modes. Tests include starting Perft depths 0–3 with exact input
preservation and state round trips for both colors. This does not prove deeper
legal Perft correctness.

## Remaining implementation work

- Attack detection, check detection, and legal filtering are absent. Kings can
  move into attack and pinned pieces can expose their own king.
- Castling and en passant are not generated; their internal make/undo paths are
  tested only with consistent manually supplied moves.
- FEN validation is structural, not legal/reachability validation. It allows
  incomplete teaching positions and does not check king counts or historical
  consistency of rights/en passant occupancy.
- `pieceAt` and `makeMove` are trusted internal APIs with bounds/consistency
  preconditions. Validate externally supplied moves before calling them.
- Search remains a placeholder returning the first pseudo-legal move. Its depth
  is ignored, and `Move{}` ambiguously signals no move; replace this when real
  search is introduced.
- Perft has no practical runtime limit or node-overflow policy for enormous
  depths. Nonnegative input alone does not make arbitrarily deep searches usable.
- FEN serialization, Perft divide, game-history draws, and hashing are pending.

The next implementation step is dedicated attack detection. See
[roadmap.md](roadmap.md) for the remaining checklists.
