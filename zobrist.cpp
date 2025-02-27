#include <cstdint>
#include <sys/random.h>

#include "board.h"


uint64_t zobrist_values[8 * 8 * 3 + 1];

void init_zobrist()
{
	getrandom(zobrist_values, sizeof zobrist_values, 0);
}

uint64_t calculate_zobrist(const board & b, const board::disk player)
{
	uint64_t hash = 0;

	for(int y=0; y<8; y++) {
		for(int x=0; x<8; x++)
			hash ^= zobrist_values[y * 8 * 3 + x * 3 + b.get(x, y)];
	}

	if (player == board::black)
		hash ^= zobrist_values[8 * 8 * 3 + 0];

	return hash;
}
