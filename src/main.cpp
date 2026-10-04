#include <charconv>
#include <iostream>
#include <string_view>

#include "kestrel/board.hpp"
#include "kestrel/movegen.hpp"
#include "kestrel/perft.hpp"

int main(int argc, char **argv) {
    kestrel::Board board;
    if (argc != 1) {
        if (argc != 3 || std::string_view(argv[1]) != "perft") {
            std::cerr << "Usage: kestrel [perft <nonnegative depth>]\n";
            return 1;
        }
        const std::string_view text(argv[2]);
        int depth = 0;
        const auto [end, error] =
            std::from_chars(text.data(), text.data() + text.size(), depth);
        if (error != std::errc{} || end != text.data() + text.size() ||
            depth < 0) {
            std::cerr << "Perft depth must be a nonnegative integer within int "
                         "range.\n";
            return 1;
        }
        std::cout << kestrel::perft(board, depth) << '\n';
        return 0;
    }
    std::cout << "Kestrel 0.1.0\nA small C++ chess engine.\n\n";
    std::cout << "Starting position pseudo-legal moves: "
              << kestrel::generatePseudoLegalMoves(board).size() << '\n';
    std::cout << "Try: ./build/kestrel perft 3\n";
}
