#include <algorithm>
#include <cstdio>
#include <readline/history.h>
#include <readline/readline.h>

#include "board.h"
#include "random.h"


void console_mode()
{
	board b(true);

	board::disk player = board::white;

	for(;;) {
		printf("\n");
		b.dump();
		printf("\n");

		auto valid_moves = b.get_valid(player);
		if (valid_moves.empty())
			break;

		if (player == board::black) {
			char *line = readline("Your move: ");
			if (line && line[0])
				add_history(line);
			else
				continue;

			int x = tolower(line[0]) - 'a';
			int y = line[1] - '1';

			std::pair<int, int> move { x, y };
			if (std::find(valid_moves.begin(), valid_moves.end(), move) == valid_moves.end()) {
				printf("%c%c is an invalid move!\n", x + 'A', y + '1');

				printf("Valid moves:");
				for(auto & move: valid_moves)
					printf(" %c%c", move.first + 'A', move.second + '1');
				printf("\n");

				continue;
			}

			b.put(x, y, player);
		}
		else {
			std::uniform_int_distribution<> rng(0, valid_moves.size() - 1);
			size_t nr   = rng(gen);
			auto   move = valid_moves.at(nr);

			printf("I play: %c%c\n", move.first + 'A', move.second + '1');

			b.put(move.first, move.second, player);
		}

		player = player == board::white ? board::black : board::white;
	}

	printf("score for white: %d\n", b.get_score(board::white));
	printf("score for black: %d\n", b.get_score(board::black));
}

int main(int argc, char *argv[])
{
	console_mode();

	return 0;
}
