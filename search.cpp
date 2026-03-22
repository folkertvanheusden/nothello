#include <algorithm>
#include <atomic>
#include <cassert>
#include <cinttypes>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <set>
#include <thread>
#include <vector>

#include "board.h"
#include "time.h"
#include "tt.h"
#include "zobrist.h"


tt tti;

constexpr const uint64_t CORNER_MASK = 0x8100000000000081;
constexpr const uint64_t BORDER_MASK = 0xff818181818181ff;

static int evaluate(const board & b, const board::disk player)
{
	auto possible_moves_white = b.get_possible_moves(board::white);
	auto possible_moves_black = b.get_possible_moves(board::black);

	int mul = possible_moves_white == 0 && possible_moves_black == 0 ? 150 : 2;
	int score = b.get_score(board::black) * mul;

	// mobility
        score += (std::popcount(possible_moves_black) - std::popcount(possible_moves_white)) * 2;

	// borders, corners
	auto bb_w = b.get_bitboard(board::white);
	auto bb_b = b.get_bitboard(board::black);

	auto border_w = bb_w & BORDER_MASK;
	auto border_b = bb_b & BORDER_MASK;
	score += std::popcount(border_b) - std::popcount(border_w);

	auto corner_w = bb_w & CORNER_MASK;
	auto corner_b = bb_b & CORNER_MASK;
	score += (std::popcount(corner_b) - std::popcount(corner_w)) * 3;

#if !defined(NDEBUG)
	if (abs(score) > 10000) {
		printf("%d\n", score);
		printf("%d, %d, %d\n", mul, b.get_score(board::black), b.get_score(board::black) * mul);
		b.dump();
		exit(1);
	}
#endif

	if (player != board::black)
		return -score;

	return score;
}

void sorter(std::vector<std::pair<int, int> > *const moves, const std::vector<std::pair<int, int> > & front)
{
	std::sort(moves->begin(), moves->end(), [front](const std::pair<int, int> & a, const std::pair<int, int> & b) {
		auto it1 = std::find(front.begin(), front.end(), a);
		auto it2 = std::find(front.begin(), front.end(), b);
		int  d1  = INT_MAX;
		int  d2  = INT_MAX;
		if (it1 != front.end())
			d1 = std::distance(front.begin(), it1);
		if (it2 != front.end())
			d2 = std::distance(front.begin(), it2);

		return d2 > d1;
	});
}

static std::pair<int, std::optional<std::pair<int, int> > > search(const board & b, const board::disk player, const int max_depth, const int depth, int alpha, int beta, uint64_t *const node_count, std::atomic_bool *const stop)
{
	if (depth == 0)
		return { evaluate(b, player), { } };

	if (*stop)
		return { 0, { } };

	(*node_count)++;

	int start_alpha = alpha;

	const int csd = max_depth - depth;
	uint64_t hash = calculate_zobrist(b, player);
	bool is_top = depth == max_depth;
	std::optional<std::pair<int, int> > tt_move;
	std::optional<tt_entry> te = tti.lookup(hash);
	if (te.has_value()) {
		bool valid = b.get_possible_moves(player) & (uint64_t(1) << (te.value().y * 8 + te.value().x));

		if (te.value().depth >= depth) {
			int  score      = te.value().score;
			int  work_score = eval_from_tt(score, csd);
			auto flag       = te.value().flags;
			bool use        = flag == EXACT ||
				(flag == LOWERBOUND && work_score >= beta) ||
				(flag == UPPERBOUND && work_score <= alpha);

			if (use) {
				if (te.value().not_pass) {
					if (valid) {
						int x = te.value().x;
						int y = te.value().y;
						return { work_score, { { x, y } } };
					}
				}

				if (!is_top)
					return { work_score, { } };
			}
		}

		if (valid && te.value().not_pass)
			tt_move = { { int(te.value().x), int(te.value().y) } };
	}

	std::optional<std::pair<int, int> > best_move;
	int  best_score = -32767;
	auto opp_c      = opponent_color(player);
	auto moves      = b.get_possible_move_list(player);
	if (tt_move.has_value()) {
		std::vector<std::pair<int, int> > front;
		front.push_back(tt_move.value());
		sorter(&moves, front);
		// if (tt_move.value().first != moves[0].first || tt_move.value().second != moves[0].second)
		// 	printf("hier %d,%d -> %d,%d\n", tt_move.value().first, tt_move.value().second, moves[0].first, moves[0].second);
	}

        for(auto & move : moves) {
		board new_position(b);
		new_position.put(move.first, move.second, player);

		auto rc = search(new_position, opp_c, max_depth, depth - 1, -beta, -alpha, node_count, stop);
		int score = -rc.first;

		if (score > best_score) {
			best_move = move;
			best_score = score;

			if (score > alpha) {
				if (score >= beta)
					break;
				alpha = score;
			}
		}
	}

	if (best_score == -32767) {
		if (b.get_possible_moves(opp_c) == 0) {
			int score = evaluate(b, player);
			if (score < 0)
				best_score = score + csd;
			else if (score > 0)
				best_score = score - csd;
			else
				best_score = 0;
		}
		else {
			board new_position(b);
			auto rc = search(new_position, opp_c, max_depth, depth - 1, -beta, -alpha, node_count, stop);
			best_score = -rc.first;
			best_move.reset();
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

struct end_t {
	std::atomic_bool        flag;
	std::condition_variable cv;
	std::mutex              cv_lock;
};

void set_flag(end_t *const stop)
{
	std::unique_lock<std::mutex> lck(stop->cv_lock);
	stop->cv.notify_all();
	stop->flag = true;
}

static void timer(const int think_time, end_t *const ei)
{
	if (think_time > 0) {
		auto end_time = std::chrono::high_resolution_clock::now() += std::chrono::milliseconds{think_time};

		std::unique_lock<std::mutex> lk(ei->cv_lock);
		while(!ei->flag) {
			if (ei->cv.wait_until(lk, end_time) == std::cv_status::timeout)
				break;
		}
	}

	set_flag(ei);
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

// returns true for repetition
bool update_and_check_repetition(std::set<uint64_t> *const history, const board & b, const board::disk player)
{
	uint64_t hash = calculate_zobrist(b, player);
	if (history->find(hash) != history->end())
		return true;
	history->insert(hash);
	return false;
}

std::optional<std::pair<std::pair<int, int>, int> > generate_search_move(const board & b, const board::disk player, const int search_time)
{
	end_t    ei { };
	uint64_t global_start_t = get_ts_ms();

	auto moves = b.get_possible_move_list(player);
	if (moves.empty())  // pass when no moves possible
		return { };
	if (moves.size() == 1)
		return { { moves.at(0), 0 } };

	auto think_timeout_timer = new std::thread([search_time, &ei] { timer(search_time, &ei); });

	int alpha = -10000;
	int beta = 10000;
	int add_alpha = 15;
	int add_beta = 15;
	int d = 1;
	std::pair<int, int> best_move { -1, -1 };
	int best_score = 0;
	int alpha_repeat = 0;
	int beta_repeat = 0;

	uint64_t node_count = 0;
	for(;;) {
		uint64_t start_t = get_ts_ms();
		auto rc = search(b, player, d, d, alpha, beta, &node_count, &ei.flag);
		uint64_t end_t = get_ts_ms();
		if (ei.flag)
			break;
		int score = rc.first;

		uint64_t t_delta = std::max(end_t - global_start_t, uint64_t(1));
		printf("info depth %d time %zu nps %zu score cp %d pv %s\n", d, size_t(t_delta), size_t(node_count * 1000 / t_delta), score, gen_pv_str_from_tt(b, rc.second, player).c_str());

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
			best_score = rc.first;
			if (rc.second.has_value())
				best_move = rc.second.value();
			else
				best_move = { -1, -1 };
			if (d >= 127)
				break;
		}

		int64_t time_left = search_time - (end_t - global_start_t);
		if (end_t - start_t > time_left / 2)
			break;
	}

	set_flag(&ei);
	think_timeout_timer->join();
	delete think_timeout_timer;

	uint64_t global_end_t = get_ts_ms();
	printf("info string used %" PRIu64 " ms of %d ms\n", global_end_t - global_start_t, search_time);

	if (best_move.first != -1)
		return { { best_move, best_score } };

	return { };
}
