#include <cassert>
#include <cstdio>
#include <optional>

#include "board.h"


board::board()
{
}

board::~board()
{
}

bool board::is_valid(const int set_x, const int set_y, const disk cur)
{
	if (disks[set_y][set_x] != empty)
		return false;

	disk opponent = cur == white ? black : white;

	// right
	if (set_x < 7 && disks[set_y][set_x + 1] == opponent) {
		std::optional<bool> validr;

		for(int x=set_x + 2; x<8; x++) {
			if (disks[set_y][x] == empty) {
				validr = false;
				break;
			}
			else if (disks[set_y][x] == cur) {
				validr = true;
				break;
			}
		}

		if (validr.has_value() == true && validr.value())
			return true;
	}

	// left
	if (set_x > 0 && disks[set_y][set_x - 1] == opponent) {
		std::optional<bool> validl;

		for(int x=set_x - 2; x>=0; x--) {
			if (disks[set_y][x] == empty) {
				validl = false;
				break;
			}
			else if (disks[set_y][x] == cur) {
				validl = true;
				break;
			}
		}

		if (validl.has_value() == true && validl.value())
			return true;
	}

	// down
	if (set_y < 7 && disks[set_y + 1][set_x] == opponent) {
		std::optional<bool> validd;

		for(int y=set_y + 2; y<8; y++) {
			if (disks[y][set_x] == empty) {
				validd = false;
				break;
			}
			else if (disks[y][set_x] == cur) {
				validd = true;
				break;
			}
		}

		if (validd.has_value() == true && validd.value())
			return true;
	}

	// up
	if (set_y > 0 && disks[set_y - 1][set_x] == opponent) {
		std::optional<bool> validu;

		for(int y=set_y - 2; y>=0; y--) {
			if (disks[y][set_x] == empty) {
				validu = false;
				break;
			}
			else if (disks[y][set_x] == cur) {
				validu = true;
				break;
			}
		}

		if (validu.has_value() == true && validu.value())
			return true;
	}

	return false;
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

void board::put(const int set_x, const int set_y, const disk cur)
{
	assert(disks[set_y][set_x] == empty);
	disks[set_y][set_x] = cur;

	for(int y=0; y<8; y++) {
		for(int x=0; x<8; x++) {
			if (disks[y][x] == empty)
				continue;

			scan_and_flip(x, y, 0, 1);
			scan_and_flip(x, y, 1, 0);
			scan_and_flip(x, y, 0, -1);
			scan_and_flip(x, y, -1, 0);

			scan_and_flip(x, y,  1,  1);
			scan_and_flip(x, y,  1, -1);
			scan_and_flip(x, y, -1,  1);
			scan_and_flip(x, y, -1, -1);
		}
	}
}

board::disk board::get(const int x, const int y) const
{
	return disks[y][x];
}

void board::dump() const
{
	for(int y=7; y>=0; y--) {
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
}

int board::score(const disk for_whom)
{
	int scores[3] { 0 };

	for(int y=0; y<8; y++) {
		for(int x=0; x<8; x++)
			scores[disks[y][x]]++;
	}

	return scores[for_whom] - scores[for_whom == white ? black : white];
}
