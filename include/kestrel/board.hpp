#pragma once

#include <array>
#include <string>
#include "kestrel/move.hpp"

namespace kestrel {

struct BoardState {
    Color sideToMove;
    int enPassantSquare;
    uint8_t castlingRights;
};

class Board {
public:
    Board();

    void clear();
    void setStartingPosition();
    bool setFromFen(const std::string& fen);

    const Piece& pieceAt(Square square) const;
    Piece& pieceAt(Square square);

    Color sideToMove() const { return sideToMove_; }
    int enPassantSquare() const { return enPassantSquare_; }
    uint8_t castlingRights() const { return castlingRights_; }

    BoardState makeMove(const Move& move);
    void undoMove(const Move& move, const BoardState& state);

private:
    std::array<Piece, 64> squares_{};
    Color sideToMove_ = Color::White;
    int enPassantSquare_ = -1;
    uint8_t castlingRights_ = 0;
};

constexpr uint8_t WhiteKingSide  = 1 << 0;
constexpr uint8_t WhiteQueenSide = 1 << 1;
constexpr uint8_t BlackKingSide  = 1 << 2;
constexpr uint8_t BlackQueenSide = 1 << 3;

} // namespace kestrel
