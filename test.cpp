#include <cassert>
#include <cstdio>

#include "board.h"


int main(int argc, char *argv[])
{
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
		assert(b3.emit_fen(board::black) == "8/1oooo3/1xxxx3/1xxxx3/1xxxx3/8/8/8 x");
		assert(b3.is_valid(0, 4, board::white));
		b3.put(0, 4, board::white);

		assert(b3.emit_fen(board::black) == "8/1oooo3/1xoxx3/1oxxx3/oxxxx3/8/8/8 x");
		assert(b3.is_valid(5, 4, board::white));
		b3.put(5, 4, board::white);

		assert(b3.emit_fen(board::black) == "8/1oooo3/1xoox3/1oxxo3/oooooo2/8/8/8 x");
	}

	// regressions
	{
		board b("8/2o2o2/3o1o1x/2oooxxx/2ooooxx/2ooxxox/4oxoo/2ooooxx");  // x
		b.put(1, 7, board::black);  // B8
		assert(b.get(2, 7) == board::black);  // C8
		assert(b.get(3, 7) == board::black);  // D8
		assert(b.get(4, 7) == board::black);  // E8
		assert(b.get(5, 7) == board::black);  // F8
	}

	printf("All good.\n");

	return 0;
}
