#pragma once

#include "kestrel/board.hpp"

namespace kestrel {

class Search {
public:
    Move findBestMove(Board& board, int depth);
};

} // namespace kestrel
