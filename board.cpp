#include <cassert>
#include <cstring>
#include <cstdio>
#include <optional>

#include "board.h"
#include "str.h"


constexpr const uint64_t MASKS[] {
	0x7F7F7F7F7F7F7F7F,  // Right
	0x007F7F7F7F7F7F7F,  // Down-right
	0x00FFFFFFFFFFFFFF,  // Down
	0x00FEFEFEFEFEFEFE,  // Down-left
	0xFEFEFEFEFEFEFEFE,  // Left
	0xFEFEFEFEFEFEFE00,  // Up-left
	0xFFFFFFFFFFFFFFFF,  // Up
	0x7F7F7F7F7F7F7F00   // Up-right
};
constexpr const uint64_t CORNER_MASK = 0x8100000000000081;
constexpr const int SHIFTS [] { 1, 9, 8, 7, 1, 9, 8, 7 };  // 4 right- and 4 left-shifts

board::board()
{
}

board::board(const std::string & fen)
{
	set_fen(fen);
}

board::board(const char *const fen)
{
	set_fen(fen);
}

board::board(const board & in)
{
	disks[white] = in.disks[white];
	disks[black] = in.disks[black];
}

board::~board()
{
}

// movegen is a transpile from/inspired by
// https://github.com/shedskin/shedskin/blob/master/examples/othello2/othello2.py (by Mark Dufour)

uint64_t board::shift(const uint64_t disks, const int direction, const int S, const uint64_t M) const
{
	if (direction < 4)
		return (disks >> S) & M;

	return (disks << S) & M;
}

void board::get_to(board & target) const
{
	target.disks[white] = disks[white];
	target.disks[black] = disks[black];
}

board & board::operator=(const board & in)
{
	in.get_to(*this);

	return *this;
}

inline uint64_t gen_mask(const int x, const int y)
{
	int      offset = y * 8 + x;
	return uint64_t(1) << offset;
}

void board::set(const int x, const int y, board::disk d)
{
	uint64_t mask = gen_mask(x, y);

	if (d == white)
		disks[white] |= mask;
	else if (d == black)
		disks[black] |= mask;
	else {
		disks[white] &= ~mask;
		disks[black] &= ~mask;
	}
}

void board::set_fen(const std::string & fen)
{
	auto parts = split(fen, " " );

	// 8/8/8/3ox3/3xo3/8/8/8 x

	disks[0] = disks[1] = disks[2] = 0;

	int x = 0;
	int y = 0;

	for(auto & c: parts[0]) {
		if (c == 'o' || c == 'O')
			set(x++, y, white);
		else if (c == 'x' || c == 'X')
			set(x++, y, black);
		else if (c == '/') {
		}
		else
			x += c - '0';

		if (x == 8) {
			y++;
			x = 0;
		}
	}
}

uint64_t board::get_possible_moves(const disk color) const
{
    uint64_t moves = 0;

    auto my_disks  = disks[color];
    auto opp_disks = disks[opponent_color(color)];
    auto empties   = ~(my_disks | opp_disks);

    for(int direction=0; direction<8; direction++) {
        auto S = SHIFTS[direction];
        auto M = MASKS[direction];
	auto MO = M & opp_disks;

        // Get opponent disks adjacent to my disks in direction dir.
        uint64_t x = shift(my_disks, direction, S, MO);

        // Add opponent disks adjacent to those, and so on.
	for(int i=0; i<5; i++)
		x |= shift(x, direction, S, MO);

        // Empty cells adjacent to those are valid moves.
        moves |= shift(x, direction, S, M) & empties;
    }

    return moves;
}

std::vector<std::pair<int, int> > board::get_possible_move_list(const disk color) const
{
	std::vector<std::pair<int, int> > out;

	auto pattern = get_possible_moves(color);
	out.reserve(std::popcount(pattern));

	while(pattern) {
		int i = std::countr_zero(pattern);
		out.push_back({ i & 7, i >> 3 });
		pattern &= (pattern - 1);
	}

	return out;
}

uint64_t board::get_bitboard(const disk color) const
{
	return disks[color];
}

board::disk board::get(const int x, const int y) const
{
	uint64_t mask = gen_mask(x, y);

	if (disks[white] & mask)
		return white;
	if (disks[black] & mask)
		return black;
	return empty;
}

void board::put(const int x, const int y, const disk color)
{
	int move = y * 8 + x;
	put(move, color);
}

void board::put(const int move, const disk color)
{
	uint64_t disk = uint64_t(1) << move;
	disks[color] |= disk;

	auto my_disks  = disks[color];
	auto opp_disks = disks[opponent_color(color)];

	uint64_t captured_disks = 0;

	for(int direction=0; direction<8; direction++) {
		auto S = SHIFTS[direction];
		auto M = MASKS[direction];

		// Find opponent disk adjacent to the new disk.
		auto x = shift(disk, direction, S, M) & opp_disks;

		// Add any adjacent opponent disk to that one, and so on.
		for(int i=0; i<5 && x != 0; i++)
			x |= shift(x, direction, S, M) & opp_disks;

		// Determine whether the disks were captured.
		auto bounding_disk = shift(x, direction, S, M) & my_disks;
		if (bounding_disk)
			captured_disks |= x;
	}

	disks[color] ^= captured_disks;
	disks[opponent_color(color)] ^= captured_disks;
}

void board::dump() const
{
	for(int y=0; y<8; y++) {
		printf("%d ", 7 - y + 1);

		for(int x=0; x<8; x++) {
			auto d = get(x, 7 - y);
			if (d == empty)
				printf(".");
			else if (d == black)
				printf("x");
			else
				printf("o");
		}

		printf("\n");
	}

	printf("  ");
	for(int x=0; x<8; x++)
		printf("%c", 'A' + x);
	printf("\n");
}

int board::get_score(const disk for_whom) const
{
	return std::popcount(disks[for_whom]) - std::popcount(disks[opponent_color(for_whom)]);
}

int board::estimate_total_move_count() const
{
	auto empties = ~(disks[white] | disks[black]);
	return std::popcount(empties);
}

std::string board::emit_fen(const disk current_player) const
{
	std::string out;

	for(int y=0; y<8; y++) {
		int skip = 0;

		for(int x=0; x<8; x++) {
			auto d = get(x, y);
			if (d == empty)
				skip++;
			else {
				if (skip) {
					out += myformat("%d", skip);
					skip = 0;
				}

				if (d == white)
					out += "o";
				else
					out += "x";
			}
		}

		if (skip)
			out += myformat("%d", skip);

		if (y != 7)
			out += "/";
	}

	out += myformat(" %c", current_player == white ? 'o' : 'x');

	return out;
}

bool board::operator==(const board & rhs) const
{
	return disks[white] == rhs.disks[white] && disks[black] == rhs.disks[black];
}

board::disk opponent_color(const board::disk & cur)
{
	return cur == board::white ? board::black : board::white;
}
