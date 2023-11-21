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

void board::put(const int set_x, const int set_y, const disk cur)
{
	assert(disks[set_y][set_x] == empty);
	disks[set_y][set_x] = cur;

	for(int y=0; y<8; y++) {
		std::optional<disk> border;
		std::optional<int>  border_x;
		bool                any_other = false;

		for(int x=0; x<8; x++) {
			if (disks[y][x] == empty) {
				border  .reset();
				border_x.reset();
				any_other = false;
			}
			else {
				if (border.has_value() == true && disks[y][x] == border.value()) {  // continuing a color
					if (any_other) {  // there was another color in between
						// fill range
						for(int fill_x=border_x.value(); fill_x<x; fill_x++)
							disks[y][fill_x] = border.value();
					}

					border_x  = x;
					any_other = false;
				}
				else if (border.has_value() == true && disks[y][x] != border.value()) {  // different color detected; register
					any_other = true;
				}
				else if (border.has_value() == false) {  // new border color
					border   = disks[y][x];
					border_x = x;
				}
				else {
					assert(0);
				}
			}
		}
	}

	for(int x=0; x<8; x++) {
		std::optional<disk> border;
		std::optional<int>  border_y;
		bool                any_other = false;

		for(int y=0; y<8; y++) {
			if (disks[y][x] == empty) {
				border  .reset();
				border_y.reset();
				any_other = false;
			}
			else {
				if (border.has_value() == true && disks[y][x] == border.value()) {  // continuing a color
					if (any_other) {  // there was another color in between
						// fill range
						for(int fill_y=border_y.value(); fill_y<y; fill_y++)
							disks[fill_y][x] = border.value();
					}

					border_y  = y;
					any_other = false;
				}
				else if (border.has_value() == true && disks[y][x] != border.value()) {  // different color detected; register
					any_other = true;
				}
				else if (border.has_value() == false) {  // new border color
					border   = disks[y][x];
					border_y = y;
				}
				else {
					assert(0);
				}
			}
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
