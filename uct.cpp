#include <algorithm>
#include <assert.h>
#include <cfloat>
#include <random>
#include <mutex>

#include "board.h"
#include "playout.h"
#include "random.h"
#include "str.h"
#include "time.h"
#include "uct.h"


thread_local auto rng = std::default_random_engine {};

uct_node::uct_node(uct_node *const parent, board & position, const board::disk player, const std::optional<std::pair<int, int> > & causing_move) :
	parent(parent),
	position(std::move(position)),
	player(player),
	causing_move(causing_move)
{
}

uct_node::~uct_node()
{
	for(auto & c: children)
		delete c;
}

std::optional<uct_node *> uct_node::add_child(const std::pair<int, int>  & m)
{
	board new_position(position);

	new_position.put(m.first, m.second, player);

	children.emplace_back(new uct_node(this, new_position, opponent_color(player), m));

	return { children.back() };
}

uint64_t uct_node::get_visit_count() const
{
	return visited;
}

double uct_node::get_score_count() const
{
	return score;
}

void uct_node::update_stats(const uint64_t visited, const double score)
{
	this->visited += visited;
	this->score   += score;
}

double uct_node::get_score()
{
	assert(visited);

	double UCTj = score / visited;

	constexpr double sqrt_2 = sqrt(2.0);

	UCTj += sqrt_2 * sqrt(log(parent->get_visit_count()) / visited);

	return UCTj;
}

uct_node *uct_node::pick_unvisited()
{
	if (first) {
		first = false;

		unvisited = this->position.get_valid(player);

		game_over = unvisited.empty();

		if (!game_over)
			std::shuffle(std::begin(unvisited), std::end(unvisited), rng);
	}

	// TODO: also 'pass'

	if (unvisited.empty())
		return nullptr;

	auto first = unvisited.back();

	auto new_node = add_child(first);

	unvisited.pop_back();

	if (new_node.has_value())
		return new_node.value();

	return nullptr;
}

bool uct_node::fully_expanded() const
{
	return unvisited.empty();
}

uct_node *uct_node::best_uct()
{
	uct_node *best       = nullptr;
	double    best_score = -DBL_MAX;

	for(auto & u : children) {
		double current_score = u->get_score();

		if (current_score > best_score) {
			best_score = current_score;
			best = u;
		}
	}

	return best;
}

uct_node *uct_node::traverse()
{
	uct_node *node = this;

	while(node->fully_expanded()) {
		uct_node *next = node->best_uct();

		if (!next)
			break;

		node = next;
	}

	uct_node *chosen = node;

	if (node && node->is_game_over() == false)
		chosen = node->pick_unvisited();

	return chosen;
}

const uct_node *uct_node::best_child() const
{
	const uct_node *best       = nullptr;
	uint64_t        best_count = 0;

	for(auto & u : children) {
		uint64_t count = u->get_visit_count();

		if (count > best_count) {
			best_count = count;
			best       = u;
		}
	}

	return best;
}

auto uct_node::get_children() const
{
	std::vector<std::tuple<std::pair<int, int> , uint64_t, double> > out;

	for(auto & u: children)
		out.push_back({ u->get_causing_move(), u->get_visit_count(), u->get_score_count() });

	return out;
}

uct_node *uct_node::get_parent()
{
	return parent;
}

void uct_node::backpropagate(uct_node *const leaf, double result)
{
	uct_node *node = leaf;

	do {
		node->update_stats(1, result);

		result = 1. - result;

		node = node->get_parent();
	}
	while(node);
}

const board & uct_node::get_position() const
{
	return position;
}

double uct_node::playout(const uct_node *const leaf)
{
	board::disk p = leaf->get_player();
	auto rc       = ::playout(leaf->get_position(), p);
	int  rc_score = std::get<2>(rc);

	double result = 0.5;

	if (rc_score > 0)
		result = 1.;
	else if (rc_score < 0)
		result = 0.;

	return result;
}

void uct_node::monte_carlo_tree_search()
{
	uct_node *leaf = traverse();

	if (leaf == nullptr)  // ko
		return;

	double simulation_result = playout(leaf);

	backpropagate(leaf, 1. - simulation_result);
}

const std::pair<int, int>  uct_node::get_causing_move() const
{
	return causing_move.value();
}

uct_node *uct_node::find_position(const board & which)
{
	for(auto it=children.begin(); it!=children.end();) {
		if ((*it)->get_position() == which) {
			uct_node *out = *it;

			out->reset_parent();

			children.erase(it);

			return out;
		}

		it++;
	}

	return nullptr;
}

std::tuple<std::optional<std::pair<int, int> >, uint64_t, uint64_t, std::vector<std::tuple<std::pair<int, int> , uint64_t, double> > > calculate_move(const board & b, const board::disk p, const uint64_t think_end_time, const uint64_t think_end_time_extra, uct_node **const root)
{
	if (*root) {
		uct_node *new_root = (*root)->find_position(b);
//		if (new_root)
//			send(true, "# Tree-hit");

		delete *root;

		*root = new_root;
	}

	if (*root == nullptr) {
		board b_copy(b);

		*root = new uct_node(nullptr, b_copy, p, { });
	}

	uint64_t use_think_end_time = think_end_time;
	bool     extra_time_check   = false;

	uint64_t  n_played          = 0;

	for(;;) {
		(*root)->monte_carlo_tree_search();

		n_played++;

		if (get_ts_ms() >= use_think_end_time) {
			auto best_node      = (*root)->best_child();

			std::optional<std::pair<int, int> > best_move;
			uint64_t best_count = 0;

			if (best_node) {
				best_move  = best_node->get_causing_move();

				best_count = best_node->get_visit_count();
			}

			auto children = (*root)->get_children();

			if (extra_time_check == false) {
				extra_time_check = true;

				int  count_best = 0;

				for(auto & c: children)
					count_best += std::get<1>(c) == best_count;

				if (count_best > 1) {
					use_think_end_time = think_end_time_extra;
					// send(true, "# using extra time (%zd)", think_end_time_extra - think_end_time);
					continue;
				}
			}

			return { best_move, n_played, best_count, children };
		}
	}
}
