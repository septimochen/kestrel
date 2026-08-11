#pragma once

#include <cstdint>
#include "kestrel/board.hpp"

namespace kestrel {

uint64_t perft(Board& board, int depth);

} // namespace kestrel
