#include <cassert>
#include <cstdint>
#include <iostream>

#include "kestrel/board.hpp"
#include "kestrel/perft.hpp"

int main() {
    kestrel::Board board;

    assert(kestrel::perft(board, 1) == 20);
    assert(kestrel::perft(board, 2) == 400);
    assert(kestrel::perft(board, 3) == 8902);

    std::cout << "Kestrel perft tests passed.\n";
    return 0;
}
