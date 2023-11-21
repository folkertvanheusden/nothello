#include <cassert>
#include <cstdio>

#include "board.h"


int main(int argc, char *argv[])
{
	board b;

	// left
	b.put(1, 1, board::white);
	b.put(2, 1, board::black);
	b.put(3, 1, board::black);
	assert(b.is_valid(4, 1, board::white));
	b.put(4, 1, board::white);
	assert(b.get(2, 1) == board::white);
	assert(b.get(3, 1) == board::white);

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

	board b2;
	b2.put(1, 1, board::white);
	b2.put(2, 1, board::white);
	b2.put(3, 1, board::white);
	b2.put(4, 1, board::white);
	for(int y=2; y<=4; y++) {
		for(int x=1; x<=4; x++)
			b2.put(x, y, board::black);
	}
	assert(!b2.is_valid(0, 4, board::white));
	b2.put(0, 4, board::white);
	assert(b2.is_valid(5, 4, board::white));
	b2.put(5, 4, board::white);
	for(int y=0; y<8; y++) {
		for(int x=0; x<8; x++)
			assert(b2.get(x, y) == board::empty || b2.get(x, y) == board::white);
	}

	printf("All good.\n");

	return 0;
}
