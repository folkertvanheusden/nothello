#include <cassert>
#include <cstdio>

#include "board.h"


board::board()
{
}

board::~board()
{
}

void board::put(const int x, const int y, const disk cur)
{
	assert(disks[y][x] == empty);

	disk opponent = cur == white ? black : white;

	disks[y][x] = cur;

	// left
	int cur_left_x = x - 1;

	while(cur_left_x >= 0) {
		if (disks[y][cur_left_x] == opponent)
			cur_left_x--;
		else {
			if (disks[y][cur_left_x] == cur) {
				for(int reset_x=cur_left_x + 1; reset_x<x; reset_x++)
					disks[y][reset_x] = cur;
			}

			break;
		}
	}

	// right
	int cur_right_x = x - 1;

	while(cur_right_x < 8) {
		if (disks[y][cur_right_x] == opponent)
			cur_right_x++;
		else {
			if (disks[y][cur_right_x] == cur) {
				for(int reset_x=x + 1; reset_x<cur_right_x; reset_x++)
					disks[y][reset_x] = cur;
			}

			break;
		}
	}

	// up
	int cur_up_y = y - 1;

	while(cur_up_y >= 0) {
		if (disks[cur_up_y][x] == opponent)
			cur_up_y--;
		else {
			if (disks[cur_up_y][x] == cur) {
				for(int reset_y=cur_up_y + 1; reset_y<y; reset_y++)
					disks[reset_y][x] = cur;
			}

			break;
		}
	}

	// down
	int cur_down_y = y + 1;

	while(cur_down_y < 8) {
		if (disks[cur_down_y][x] == opponent)
			cur_down_y++;
		else {
			if (disks[cur_down_y][x] == cur) {
				for(int reset_y=y + 1; reset_y<cur_down_y; reset_y++)
					disks[reset_y][x] = cur;
			}

			break;
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
