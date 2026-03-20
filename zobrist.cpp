#include <cstdint>
#include <sys/random.h>

#include "board.h"


uint64_t zobrist_values[8 * 8 * 2 + 1];

void init_zobrist()
{
	getrandom(zobrist_values, sizeof zobrist_values, 0);
}

uint64_t calculate_zobrist(const board & b, const board::disk player)
{
	uint64_t hash = 0;

	auto stones_w = b.get_bitboard(board::white);
        while(stones_w) {
                int i = std::countr_zero(stones_w);
		hash ^= zobrist_values[i * 2 + 0];
                stones_w &= (stones_w - 1);
        }

	auto stones_b = b.get_bitboard(board::black);
        while(stones_b) {
                int i = std::countr_zero(stones_b);
		hash ^= zobrist_values[i * 2 + 1];
                stones_b &= (stones_b - 1);
        }

	if (player == board::black)
		hash ^= zobrist_values[8 * 8 * 2 + 0];

	return hash;
}
