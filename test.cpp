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
	b.put(4, 1, board::white);
	assert(b.get(2, 1) == board::white);
	assert(b.get(3, 1) == board::white);

	// up
	b.put(1, 2, board::black);
	b.put(1, 3, board::black);
	b.put(1, 4, board::white);
	assert(b.get(1, 2) == board::white);
	assert(b.get(1, 3) == board::white);

	b.put(7, 1, board::white);
	b.put(6, 1, board::black);
	b.put(5, 1, board::black);
	assert(b.get(5, 1) == board::white);
	assert(b.get(6, 1) == board::white);

	printf("All good.\n");

	return 0;
}
