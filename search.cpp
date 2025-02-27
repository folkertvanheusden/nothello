#include <atomic>
#include <cassert>
#include <cinttypes>
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
	int best_score = -32767;
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

	if (best_score == -32767) {
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
			board new_position(b);
			auto rc = search(new_position, opponent_color(player), max_depth, depth - 1, -beta, -alpha, stop);
			best_score = -rc.first;
			best_move = rc.second;
		}
	}

        if (*stop == false) {
                tt_entry_flag flag = EXACT;
                if (best_score <= start_alpha)
                        flag = UPPERBOUND;
                else if (best_score >= beta)
                        flag = LOWERBOUND;

		assert(best_score > -32767);
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
		while(std::chrono::high_resolution_clock::now() < end_time && *stop == false)
			usleep(10000);
        }

	*stop = true;
}

std::string gen_pv_str_from_tt(const board & b, const std::optional<std::pair<int, int> > & first_move, const board::disk player)
{
	auto pv = get_pv_from_tt(b, first_move, player);
	std::string pv_str;
	for(auto & move : pv) {
		if (pv_str.empty() == false)
			pv_str += " ";
		if (move.has_value()) {
			pv_str += char('a' + move.value().first);
			pv_str += char('1' + move.value().second);
		}
		else {
			pv_str += "0000";
		}
	}
	return pv_str;
}

std::optional<std::pair<int, int> > generate_search_move(const board & b, const board::disk player, const int search_time)
{
	std::atomic_bool stop { false };
	uint64_t global_start_t = get_ts_ms();
	auto think_timeout_timer = new std::thread([search_time, &stop] { timer(search_time, &stop); });

	int alpha = -10000;
	int beta = 10000;
	int add_alpha = 15;
	int add_beta = 15;
	int d = 1;
	std::optional<std::pair<int, int> > best_move;
	int alpha_repeat = 0;
	int beta_repeat = 0;

	for(;;) {
		uint64_t start_t = get_ts_ms();
		auto rc = search(b, player, d, d, alpha, beta, &stop);
		uint64_t end_t = get_ts_ms();
		if (stop)
			break;
		int score = rc.first;

		printf("info depth %d score cp %d pv %s\n", d, score, gen_pv_str_from_tt(b, rc.second, player).c_str());

		if (score <= alpha) {
			if (alpha_repeat >= 3)
				alpha = -10000;
			else {
				beta = (alpha + beta) / 2;
				alpha = score - add_alpha;
				if (alpha < -10000)
					alpha = -10000;
				add_alpha += add_alpha / 5 + 1;

				alpha_repeat++;
			}
		}
		else if (score >= beta) {
			if (beta_repeat >= 3)
				beta = 10000;
			else {
				alpha = (alpha + beta) / 2;
				beta = score + add_beta;
				if (beta > 10000)
					beta = 10000;
				add_beta += add_beta / 5 + 1;

				beta_repeat++;
			}
		}
		else {
			d++;
			best_move = rc.second;
		}

		int64_t time_left = search_time - (end_t - global_start_t);
		if (end_t - start_t > time_left / 2)
			break;
	}

	stop = true;
	think_timeout_timer->join();
	delete think_timeout_timer;

	uint64_t global_end_t = get_ts_ms();
	printf("info string used %" PRIu64 " ms of %d ms\n", global_end_t - global_start_t, search_time);

	return best_move;
}
