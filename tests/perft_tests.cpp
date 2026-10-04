#include "kestrel/board.hpp"
#include "kestrel/perft.hpp"
#include <array>
#include <iostream>
#include <stdexcept>

int main() {
    kestrel::Board board;
    const auto original = board;
    constexpr std::array<uint64_t, 4> expected{1, 20, 400, 8902};
    for (int depth = 0; depth < 4; ++depth) {
        const auto actual = kestrel::perft(board, depth);
        if (actual != expected[depth] || board != original) {
            std::cerr << "Starting depth " << depth << ": expected "
                      << expected[depth] << ", got " << actual
                      << "; board preserved: " << (board == original) << '\n';
            return 1;
        }
    }
    try {
        (void)kestrel::perft(board, -1);
        std::cerr << "Negative depth was accepted\n";
        return 1;
    } catch (const std::invalid_argument &) {
    }
    std::cout << "Starting depths 0–3 and state preservation passed.\n";
}
