#include "kestrel/perft.hpp"

#include "kestrel/movegen.hpp"
#include <stdexcept>

namespace kestrel {

uint64_t perft(Board &board, int depth) {
    if (depth < 0)
        throw std::invalid_argument("Perft depth must be nonnegative");
    if (depth == 0) {
        return 1;
    }

    uint64_t nodes = 0;
    auto moves = generatePseudoLegalMoves(board);

    for (const Move &move : moves) {
        BoardState state = board.makeMove(move);
        nodes += perft(board, depth - 1);
        board.undoMove(move, state);
    }

    return nodes;
}

} // namespace kestrel
