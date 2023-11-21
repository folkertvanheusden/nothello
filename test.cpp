#include <cassert>
#include <cstdio>

#include "board.h"


int main(int argc, char *argv[])
{
	board b(false);

	// left
	b.put(1, 1, board::white);
	b.put(2, 1, board::black);
	b.put(3, 1, board::black);
	assert(b.is_valid(4, 1, board::white));
	assert(b.get_valid(board::white).size() == 1);
	assert(b.get_valid(board::black).size() == 1);
	b.put(4, 1, board::white);
	assert(b.get(2, 1) == board::white);
	assert(b.get(3, 1) == board::white);

	board b2(false);
	b2.put(1, 1, board::white);
	b2.put(2, 2, board::black);
	b2.put(3, 3, board::black);
	b2.put(4, 4, board::white);
	assert(b2.get(2, 2) == board::white);
	assert(b2.get(3, 3) == board::white);

	// up
	b.put(1, 2, board::black);
	b.put(1, 3, board::black);
	assert(b.is_valid(1, 4, board::white));
	b.put(1, 4, board::white);
	assert(b.get(1, 2) == board::white);
	assert(b.get(1, 3) == board::white);

	b.put(6, 1, board::black);
	b.put(5, 1, board::black);
	assert(b.is_valid(7, 1, board::white));
	b.put(7, 1, board::white);
	assert(b.get(5, 1) == board::white);
	assert(b.get(6, 1) == board::white);

	board b3(false);
	b3.put(1, 1, board::white);
	b3.put(2, 1, board::white);
	b3.put(3, 1, board::white);
	b3.put(4, 1, board::white);
	for(int y=2; y<=4; y++) {
		for(int x=1; x<=4; x++)
			b3.put(x, y, board::black);
	}
	assert(b3.is_valid(0, 4, board::white));
	b3.put(0, 4, board::white);
	assert(b3.is_valid(5, 4, board::white));
	b3.put(5, 4, board::white);
	for(int y=0; y<8; y++) {
		for(int x=0; x<8; x++)
			assert(b3.get(x, y) == board::empty || b3.get(x, y) == board::white);
	}
	assert(b3.get_valid(board::white).empty());
	assert(b3.get_valid(board::black).empty());

	printf("All good.\n");

	return 0;
}
