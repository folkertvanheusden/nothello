#include <cassert>
#include <cstring>
#include <cstdio>
#include <optional>

#include "board.h"
#include "str.h"


board::board(const bool set_initial)
{
	if (set_initial)
		set_fen("8/8/8/3ox3/3xo3/8/8/8");
}

board::board(const std::string & fen)
{
	set_fen(fen);
}

board::board(const board & in)
{
	in.get_to(disks);
}

board::~board()
{
}

void board::get_to(disk d[][8]) const
{
	memcpy(d, disks, sizeof(disks));
}

board & board::operator=(const board & in)
{
	in.get_to(disks);

	return *this;
}

void board::set_fen(const std::string & fen)
{
	auto parts = split(fen, " " );

	// 8/8/8/3ox3/3xo3/8/8/8 x

	int x = 0;
	int y = 0;

	for(auto & c: parts[0]) {
		if (c == 'o' || c == 'O')
			disks[y][x++] = white;
		else if (c == 'x' || c == 'X')
			disks[y][x++] = black;
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

bool board::scan(const int start_x, const int start_y, const int dx, const int dy, const disk cur)
{
	int x = start_x + dx;
	int y = start_y + dy;

	const disk          border    = cur;
	std::optional<int>  border_x;
	std::optional<int>  border_y;
	bool                any_other = false;

	while(x >= 0 && x < 8 && y >= 0 && y < 8) {
		if (disks[y][x] == empty)
			return false;

		if (disks[y][x] == border)  // continuing a color
			return any_other;  // there was another color in between

		if (disks[y][x] != border)  // different color detected; register
			any_other = true;

		x += dx;
		y += dy;
	}

	return false;
}

bool board::is_valid(const int x, const int y, const disk cur)
{
	if (disks[y][x] != empty)
		return false;

	return
		scan(x, y,  0,  1, cur) ||
		scan(x, y,  1,  0, cur) ||
		scan(x, y,  0, -1, cur) ||
		scan(x, y, -1,  0, cur) ||

		scan(x, y,  1,  1, cur) ||
		scan(x, y,  1, -1, cur) ||
		scan(x, y, -1,  1, cur) ||
		scan(x, y, -1, -1, cur);
}

std::vector<std::pair<int, int> > board::get_valid(const disk cur)
{
	std::vector<std::pair<int, int> > out;

	for(int y=0; y<8; y++) {
		for(int x=0; x<8; x++) {
			if (is_valid(x, y, cur))
				out.emplace_back(x, y);
		}
	}

	return out;
}

void board::scan_and_flip(const int start_x, const int start_y, const int dx, const int dy)
{
	int x = start_x;
	int y = start_y;

	std::optional<disk> border;
	std::optional<int>  border_x;
	std::optional<int>  border_y;
	bool                any_other = false;

	while(x >= 0 && x < 8 && y >= 0 && y < 8) {
		if (disks[y][x] == empty)
			break;

		if (border.has_value() == false) {  // new border color
			border   = disks[y][x];
			border_x = x;
			border_y = y;
		}
		else if (disks[y][x] == border.value()) {  // continuing a color
			if (any_other) {  // there was another color in between
				// fill range
				int fill_x = border_x.value() + dx;
				int fill_y = border_y.value() + dy;

				do {
					disks[fill_y][fill_x] = border.value();
					fill_x += dx;
					fill_y += dy;
				}
				while(fill_x != x && fill_y != y);
			}

			break;
		}
		else if (disks[y][x] != border.value()) {  // different color detected; register
			any_other = true;
		}

		x += dx;
		y += dy;
	}
}

void board::put(const int x, const int y, const disk cur)
{
	assert(disks[y][x] == empty);
	disks[y][x] = cur;

	scan_and_flip(x, y,  0,  1);
	scan_and_flip(x, y,  1,  0);
	scan_and_flip(x, y,  0, -1);
	scan_and_flip(x, y, -1,  0);

	scan_and_flip(x, y,  1,  1);
	scan_and_flip(x, y,  1, -1);
	scan_and_flip(x, y, -1,  1);
	scan_and_flip(x, y, -1, -1);
}

board::disk board::get(const int x, const int y) const
{
	return disks[y][x];
}

void board::dump() const
{
	for(int y=0; y<8; y++) {
		printf("%d ", y + 1);

		for(int x=0; x<8; x++) {
			if (disks[y][x] == empty)
				printf(".");
			else if (disks[y][x] == black)
				printf("*");
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

int board::get_score(const disk for_whom)
{
	int scores[3] { 0 };

	for(int y=0; y<8; y++) {
		for(int x=0; x<8; x++)
			scores[disks[y][x]]++;
	}

	return scores[for_whom] - scores[for_whom == white ? black : white];
}

int board::estimate_total_move_count()
{
	int counts[3] { 0 };

	for(int y=0; y<8; y++) {
		for(int x=0; x<8; x++)
			counts[disks[y][x]]++;
	}

	return counts[empty];
}
