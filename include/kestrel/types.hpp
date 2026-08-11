#pragma once

#include <cstdint>

namespace kestrel {

enum class Color : uint8_t {
    White,
    Black
};

constexpr Color opposite(Color color) {
    return color == Color::White ? Color::Black : Color::White;
}

enum class PieceType : uint8_t {
    None,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

struct Piece {
    PieceType type = PieceType::None;
    Color color = Color::White;

    constexpr bool empty() const {
        return type == PieceType::None;
    }
};

using Square = uint8_t;

constexpr Square makeSquare(int file, int rank) {
    return static_cast<Square>(rank * 8 + file);
}

constexpr int fileOf(Square square) {
    return square % 8;
}

constexpr int rankOf(Square square) {
    return square / 8;
}

constexpr Square A1 = 0;
constexpr Square E1 = 4;
constexpr Square H1 = 7;
constexpr Square A8 = 56;
constexpr Square E8 = 60;
constexpr Square H8 = 63;

} // namespace kestrel
