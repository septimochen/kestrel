#pragma once

#include "kestrel/move.hpp"
#include <array>
#include <expected>
#include <string>

namespace kestrel {

enum class FenError { Fields, Placement, Side, Castling, EnPassant, Counters };

struct BoardState {
    Piece movingPiece;
    Piece capturedPiece;
    Square capturedSquare;
    int halfmoveClock;
    int fullmoveNumber;
    Color sideToMove;
    int enPassantSquare;
    uint8_t castlingRights;
};

class Board {
  public:
    Board();

    void clear();
    void setStartingPosition();
    [[nodiscard]] bool setFromFen(const std::string &fen);
    [[nodiscard]] std::expected<void, FenError> loadFen(const std::string &fen);

    bool operator==(const Board &) const = default;

    const Piece &pieceAt(Square square) const;
    Piece &pieceAt(Square square);

    Color sideToMove() const { return sideToMove_; }
    int enPassantSquare() const { return enPassantSquare_; }
    uint8_t castlingRights() const { return castlingRights_; }

    int halfmoveClock() const { return halfmoveClock_; }
    int fullmoveNumber() const { return fullmoveNumber_; }

    // Requires a consistent pseudo-legal move, valid squares. Clocks saturate
    // at the int maximum.
    [[nodiscard]] BoardState makeMove(const Move &move);
    void undoMove(const Move &move, const BoardState &state);

  private:
    std::array<Piece, 64> squares_{};
    Color sideToMove_ = Color::White;
    int enPassantSquare_ = -1;
    uint8_t castlingRights_ = 0;
    int halfmoveClock_ = 0;
    int fullmoveNumber_ = 1;
};

constexpr uint8_t WhiteKingSide = 1 << 0;
constexpr uint8_t WhiteQueenSide = 1 << 1;
constexpr uint8_t BlackKingSide = 1 << 2;
constexpr uint8_t BlackQueenSide = 1 << 3;

} // namespace kestrel
