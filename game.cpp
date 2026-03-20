#include <algorithm>
#include <cinttypes>
#include <cstdio>
#include <getopt.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "board.h"
#include "random.h"
#include "search.h"
#include "time.h"
#include "ugi.h"


void console_mode()
{
	board b(INITIAL_FEN);

	board::disk player = board::black;

	for(;;) {
		printf("\n");
		b.dump();
		printf("FEN: %s\n", b.emit_fen(player).c_str());
		printf("\n");

		auto valid_moves = b.get_possible_move_list(player);
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
			auto move = generate_search_move(b, player, 1000);
			if (move.has_value()) {
				printf("I play: %c%c\n", move.value().first + 'A', move.value().second + '1');
				b.put(move.value().first, move.value().second, player);
			}
		}

		player = player == board::white ? board::black : board::white;
	}

	printf("score for white: %d\n", b.get_score(board::white));
	printf("score for black: %d\n", b.get_score(board::black));
}

void autoplay()
{
	board       b(INITIAL_FEN);
	board::disk player = board::white;

	for(;;) {
		auto move = generate_search_move(b, player, 750);
		if (move.has_value()) {
			printf("I play: %c%c\n", move.value().first + 'A', move.value().second + '1');
			b.put(move.value().first, move.value().second, player);
		}
		else {
			break;
		}

		player = player == board::white ? board::black : board::white;
	}

	b.dump();
}

uint64_t do_perft(board & b, const board::disk player, int depth)
{
	auto move_list = b.get_possible_move_list(player);
	if (depth == 1)
		return move_list.size();
	depth--;

	auto opp_color = opponent_color(player);
	uint64_t count = 0;
	for(const auto & move: move_list) {
		board b_new(b);
		b_new.put(move.first, move.second, player);
		count += do_perft(b_new, opp_color, depth);
	}

	return count;
}

void perft()
{
	board b(INITIAL_FEN);
	int d = 1;
	for(;;) {
		uint64_t t_start = get_ts_ms();
		uint64_t count   = do_perft(b, board::white, d);
		uint64_t t_end   = get_ts_ms();

		double   t_diff  = std::max(uint64_t(1), t_end - t_start) / 1000.;
		printf("%d: %" PRIu64 " (%.3f nps, %.2f seconds)\n", d, count, count / t_diff, t_diff);
		d++;
	}
}

void make_openings(int depth, int n)
{
	for(int count=0; count<n; count++)
	{
		board       b(INITIAL_FEN);
		board::disk player = board::white;

		for(int d=0; d<depth; d++) {
			auto move = generate_random_move(b, player);
			if (move.has_value() == false)
				break;

			b.put(move.value().first, move.value().second, player);
			player = player == board::white ? board::black : board::white;
		}

		printf("%s\n", b.emit_fen(player).c_str());
	}
}

void help()
{
	printf("Samei is (C) by folkert van heusden\n\n");
	printf("-m mode  console/autoplay/ugi/make-openings/perft\n");
	printf("-d x     \"make-openings\" requires a depth\n");
	printf("-n x     \"make-openings\" requires a count\n");
}

int main(int argc, char *argv[])
{
	std::string mode = "console";
	int depth = 5;
	int count = 1000;
	int c = -1;

	while((c = getopt(argc, argv, "m:d:n:h")) != -1) {
		if (c == 'm')
			mode = optarg;
		else if (c == 'd')
			depth = atoi(optarg);
		else if (c == 'n')
			count = atoi(optarg);
		else if (c == 'h') {
			help();
			return 0;
		}
	}

	if (mode == "console")
		console_mode();
	else if (mode == "autoplay")
		autoplay();
	else if (mode == "ugi")
		ugi();
	else if (mode == "perft")
		perft();
	else if (mode == "make-openings")
		make_openings(depth, count);
	else
		printf("Do not understand \"%s\"\n", mode.c_str());

	return 0;
}
