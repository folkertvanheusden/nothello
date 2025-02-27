#include <atomic>
#include <optional>
#include <thread>
#include <vector>

#include "board.h"
#include "time.h"
#include "tt.h"
#include "zobrist.h"


tt tti;

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

static std::pair<int, std::optional<std::pair<int, int> > > search(const board & b, const board::disk player, const int max_depth, const int depth, int alpha, int beta, std::atomic_bool *const stop)
{
	if (depth == 0)
		return { evaluate(b, player), { } };

	if (*stop)
		return { 0, { } };

	int start_alpha = alpha;

	const int csd = max_depth - depth;
	uint64_t hash = calculate_zobrist(b, player);
	bool is_top = depth == max_depth;
	std::optional<tt_entry> te = tti.lookup(hash);
	if (te.has_value()) {
		if (te.value().depth >= depth) {
			int  score      = te.value().score;
			int  work_score = eval_from_tt(score, csd);
			auto flag       = te.value().flags;
			bool use        = flag == EXACT ||
				(flag == LOWERBOUND && work_score >= beta) ||
				(flag == UPPERBOUND && work_score <= alpha);

			if (use) {
				if (te.value().move_valid) {
					int x = te.value().x;
					int y = te.value().y;
					return { work_score, { { x, y } } };
				}

				if (!is_top)
					return { work_score, { } };
			}
		}
	}

	auto moves = b.get_valid(player);
	std::optional<std::pair<int, int> > best_move;
	int best_score = -10000;
	for(auto & move: moves) {
		board new_position(b);
		new_position.put(move.first, move.second, player);

		auto rc = search(new_position, opponent_color(player), max_depth, depth - 1, -beta, -alpha, stop);
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

	if (best_move.has_value() == false) {
		if (b.get_valid(opponent_color(player)).empty() == true) {
			int score = evaluate(b, player);
			if (score < 0)
				best_score = -10000 + csd;
			else if (score > 0)
				best_score = 10000 - csd;
			else
				best_score = 0;
		}
		else {
			best_score = evaluate(b, player);
		}
	}

        if (*stop == false) {
                tt_entry_flag flag = EXACT;
                if (best_score <= start_alpha)
                        flag = UPPERBOUND;
                else if (best_score >= beta)
                        flag = LOWERBOUND;

		int work_score = eval_to_tt(best_score, csd);

                tti.store(hash, flag, depth, work_score, best_move);
        }

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
	uint64_t global_start_t = get_ts_ms();
	auto think_timeout_timer = new std::thread([search_time, &stop] { timer(search_time, &stop); });

	int alpha = -10000;
	int beta = 10000;
	int d = 1;
	std::optional<std::pair<int, int> > best_move;
	int best_score = -10000;

	for(;;) {
		uint64_t start_t = get_ts_ms();
		auto rc = search(b, player, d, d, alpha, beta, &stop);
		uint64_t end_t = get_ts_ms();
		if (stop)
			break;

		best_move = rc.second;
		best_score = rc.first;

		printf("info depth %d score cp %d\n", d, best_score);

		int64_t time_left = search_time - (end_t - global_start_t);
		if (end_t - start_t > time_left / 2)
			break;

		d++;
	}

	stop = true;
	think_timeout_timer->join();
	delete think_timeout_timer;

	return best_move;
}
