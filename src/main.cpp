#include <iostream>
#include <string>

#include "kestrel/board.hpp"
#include "kestrel/movegen.hpp"
#include "kestrel/perft.hpp"
#include "kestrel/search.hpp"

int main(int argc, char **argv)
{
    kestrel::Board board;

    if (argc >= 3 && std::string(argv[1]) == "perft")
    {
        int depth = std::stoi(argv[2]);
        std::cout << kestrel::perft(board, depth) << '\n';
        return 0;
    }

    std::cout << "Kestrel 0.1.0\n";
    std::cout << "A small C++ chess engine.\n\n";

    auto moves = kestrel::generatePseudoLegalMoves(board);
    std::cout << "Starting position pseudo-legal moves: "
              << moves.size() << '\n';

    std::cout << "Try: ./kestrel perft 1\n";
    return 0;
}
