#pragma once

#include <vector>
#include "kestrel/board.hpp"

namespace kestrel {

std::vector<Move> generatePseudoLegalMoves(const Board& board);

} // namespace kestrel
