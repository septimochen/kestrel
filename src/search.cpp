#include "kestrel/search.hpp"

#include "kestrel/movegen.hpp"

namespace kestrel {

Move Search::findBestMove(Board& board, int /*depth*/) {
    auto moves = generatePseudoLegalMoves(board);

    if (moves.empty()) {
        return {};
    }

    // Bootstrap placeholder:
    // return the first pseudo-legal move until legal move filtering
    // and alpha-beta search are implemented.
    return moves.front();
}

} // namespace kestrel
