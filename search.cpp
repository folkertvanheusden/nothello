#include <atomic>
#include <optional>
#include <thread>
#include <vector>

#include "board.h"


static int evaluate(const board & b, const board::disk player)
{
	int score = b.get_score(board::black) * 10;

	int scores_borders[3] { };
	for(int i=0; i<8; i++) {
		scores_borders[b.get(0, i)]++;
		scores_borders[b.get(7, i)]++;
		scores_borders[b.get(i, 0)]++;
		scores_borders[b.get(i, 7)]++;
	}
	score += scores_borders[board::black] - scores_borders[board::white];

	int scores_corners[3] { };
	scores_corners[b.get(0, 0)]++;
	scores_corners[b.get(7, 0)]++;
	scores_corners[b.get(0, 7)]++;
	scores_corners[b.get(7, 7)]++;
	score += (scores_corners[board::black] - scores_corners[board::white]) * 3;

	if (player != board::black)
		return -score;

	return score;
}

static std::pair<int, std::optional<std::pair<int, int> > > search(const board & b, const board::disk player, const int depth, int alpha, int beta, std::atomic_bool *const stop)
{
	if (depth == 0)
		return { evaluate(b, player), { } };

	if (*stop)
		return { 0, { } };

	auto moves = b.get_valid(player);
	std::optional<std::pair<int, int> > best_move;
	int best_score = -1000;
	for(auto & move: moves) {
		board new_position(b);
		new_position.put(move.first, move.second, player);

		auto rc = search(new_position, opponent_color(player), depth - 1, -beta, -alpha, stop);
		int score = -rc.first;

		if (score > best_score) {
			best_move = move;
			best_score = score;

			if (score > alpha) {
				if (score >= beta)
					return { score, best_move };
				alpha = score;
			}
		}
	}

	if (best_move.has_value() == false)
		return { 0, { } };  // TODO score

	return { best_score, best_move };
}

static void timer(const int think_time, std::atomic_bool *const stop)
{
        if (think_time > 0) {
                auto end_time = std::chrono::high_resolution_clock::now() += std::chrono::milliseconds{think_time};

		// TODO replace by condition_variable
		while(std::chrono::high_resolution_clock::now() < end_time)
			usleep(10000);
        }

	*stop = true;
}


std::optional<std::pair<int, int> > generate_search_move(const board & b, const board::disk player, const int search_time)
{
	std::atomic_bool stop { false };

	auto think_timeout_timer = new std::thread([search_time, &stop] { timer(search_time, &stop); });

	int alpha = -1000;
	int beta = 1000;
	int d = 1;
	std::optional<std::pair<int, int> > best_move;
	int best_score = -1000;

	for(;;) {
		auto rc = search(b, player, d, alpha, beta, &stop);
		if (stop)
			break;

		best_move = rc.second;
		best_score = rc.first;

		d++;
	}

	stop = true;
	think_timeout_timer->join();
	delete think_timeout_timer;

	return best_move;
}
