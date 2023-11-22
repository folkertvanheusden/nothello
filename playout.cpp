#include <algorithm>
#include <cassert>
#include <cfloat>
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

	std::vector<std::pair<int, int> > coordinates;
	for(int y=0; y<8; y++) {
		for(int x=0; x<8; x++) {
			if (b.get(x, y) == board::empty)
				coordinates.push_back({ x, y });
		}
	}
	std::shuffle(std::begin(coordinates), std::end(coordinates), gen);

	bool any_valid = false;

	do {
		any_valid = false;

		for(size_t i=0; i<coordinates.size(); i++) {
			auto move = coordinates.at(i);

			if (b.is_valid(move.first, move.second, current_player) == false)
				continue;

			size_t last = coordinates.size() - 1;
			if (i != last)
				std::swap(i, last);
			coordinates.erase(coordinates.begin() + last);

			any_valid = true;

			if (first.has_value() == false)
				first = move;

			b.put(move.first, move.second, current_player);

			current_player = current_player == board::white ? board::black : board::white;
		}
	}
	while(any_valid);

	return std::tuple<std::optional<std::pair<int, int> >, int, int>(first, mc, b.get_score(start_player));
}

std::tuple<std::optional<std::pair<int, int> >, double> find_best_move(const board & in, const board::disk start_player, const int think_time)
{
	uint64_t time_end      = get_ts_ms() + think_time;
	uint64_t playout_count = 0;
	uint64_t move_count    = 0;
	int64_t  scores[8][8] { 0 };
	int64_t  counts[8][8] { 0 };

        do {
                auto rc   = playout(in, start_player);
                auto move = std::get<0>(rc);

                if (move.has_value()) {
                        int score = std::get<2>(rc);

                        scores[move.value().second][move.value().first] += score;
                        counts[move.value().second][move.value().first]++;
                }

		move_count += std::get<1>(rc);
		playout_count++;
	}
	while(get_ts_ms() < time_end);

	printf("%zu %zu / %f\n", move_count, playout_count, move_count / double(playout_count));

	double best_score = -DBL_MAX;
	std::optional<std::pair<int, int> > chosen_move;

	for(int y=0; y<8; y++) {
		for(int x=0; x<8; x++) {
			if (counts[y][x] == 0)
				continue;

			double score = double(scores[y][x]) / counts[y][x];

			if (score > best_score) {
				best_score  = score;
				chosen_move = { x, y };
			}
		}
	}

	if (think_time == 0)  // prevent divide by 0
		return { chosen_move, 0 };

	return { chosen_move, playout_count * 1000 / think_time };
}
