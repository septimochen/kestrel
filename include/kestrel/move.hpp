#pragma once

#include "kestrel/types.hpp"

namespace kestrel {

struct Move {
    Square from = 0;
    Square to = 0;
    PieceType promotion = PieceType::None;

    bool isCapture = false;
    bool isEnPassant = false;
    bool isCastling = false;
};

} // namespace kestrel
