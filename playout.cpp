#include <optional>

#include "board.h"
#include "playout.h"
#include "random.h"
#include "time.h"


std::tuple<std::optional<std::pair<int, int> >, int, int> playout(const board & in, const board::disk start_player)
{
	board b(in);
	int mc = 0;
	board::disk current_player = start_player;

	std::optional<std::pair<int, int> > first;

	while(++mc < 8 * 8 * 8) {
		auto valid_moves = b.get_valid(current_player);
		if (valid_moves.empty())
			break;

		std::uniform_int_distribution<> rng(0, valid_moves.size() - 1);
		size_t nr   = rng(gen);
		auto   move = valid_moves.at(nr);

		if (first.has_value() == false)
			first = move;

		b.put(move.first, move.second, current_player);

		current_player = current_player == board::white ? board::black : board::white;
	}

	return std::tuple<std::optional<std::pair<int, int> >, int, int>(first, mc, b.get_score(start_player));
}

std::tuple<std::optional<std::pair<int, int> >, double> find_best_move(const board & in, const board::disk start_player, const int think_time)
{
	uint64_t time_end      = get_ts_ms() + think_time;
	uint64_t playout_count = 0;
	int64_t  counts[8][8];

	for(int y=0; y<8; y++) {
		for(int x=0; x<8; x++)
			counts[y][x] = std::numeric_limits<int64_t>::min();
	}

	do {
		auto rc   = playout(in, start_player);
		auto move = std::get<0>(rc);

		if (move.has_value()) {
			int score = std::get<2>(rc);

			if (score > 0)
				counts[move.value().second][move.value().first]++;
			else if (score < 0)
				counts[move.value().second][move.value().first]--;
		}

		playout_count++;
	}
	while(get_ts_ms() < time_end);

	int64_t best_score = std::numeric_limits<int64_t>::min();
	std::pair<int, int> chosen_move;

	for(int y=0; y<8; y++) {
		for(int x=0; x<8; x++) {
			if (counts[y][x] > best_score) {
				best_score  = counts[y][x];
				chosen_move = { x, y };
			}
		}
	}

	if (think_time == 0)  // prevent divide by 0
		return { chosen_move, 0 };

	return { chosen_move, playout_count * 1000 / think_time };
}
