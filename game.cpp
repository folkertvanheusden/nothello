#include <algorithm>
#include <cstdio>
#include <getopt.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "board.h"
#include "playout.h"
#include "random.h"
#include "time.h"
#include "uct.h"
#include "ugi.h"


void console_mode()
{
	board b(true);

	board::disk player = board::black;

	for(;;) {
		printf("\n");
		b.dump();
		printf("FEN: %s\n", b.emit_fen(player).c_str());
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
			uint64_t  now  = get_ts_ms();
			uct_node *root = nullptr;
			auto      rc   = calculate_move(b, player, now + 1000, now + 1000, &root);
			auto      move = std::get<0>(rc);
			if (move.has_value()) {
				printf("I play: %c%c (%zu playouts per second)\n", move.value().first + 'A', move.value().second + '1', size_t(std::get<1>(rc)));

				b.put(move.value().first, move.value().second, player);
			}

			delete root;
		}

		player = player == board::white ? board::black : board::white;
	}

	printf("score for white: %d\n", b.get_score(board::white));
	printf("score for black: %d\n", b.get_score(board::black));
}

void autoplay(void)
{
	board       b(true);
	board::disk player = board::white;

	for(;;) {
		auto valid_moves = b.get_valid(player);
		if (valid_moves.empty())
			break;

		uint64_t  now  = get_ts_ms();
		uct_node *root = nullptr;
		auto      rc   = calculate_move(b, player, now + 1000, now + 1000, &root);
		auto      move = std::get<0>(rc);
		if (move.has_value()) {
			printf("I play: %c%c (%zu playouts per second)\n", move.value().first + 'A', move.value().second + '1', size_t(std::get<1>(rc)));

			b.put(move.value().first, move.value().second, player);
		}

		delete root;

		player = player == board::white ? board::black : board::white;
	}

	b.dump();
}

int main(int argc, char *argv[])
{
	std::string mode = "console";
	int c = -1;

	while((c = getopt(argc, argv, "m:")) != -1) {
		if (c == 'm')
			mode = optarg;
	}

	if (mode == "console")
		console_mode();
	else if (mode == "autoplay")
		autoplay();
	else if (mode == "ugi")
		ugi();
	else
		printf("Do not understand \"%s\"\n", mode.c_str());

	return 0;
}
