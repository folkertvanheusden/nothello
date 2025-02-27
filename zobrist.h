#include <cstdint>

#include "board.h"


void init_zobrist();
uint64_t calculate_zobrist(const board & b, const board::disk player);
