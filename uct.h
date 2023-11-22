#include <optional>
#include <unordered_set>
#include <vector>

#include "board.h"


class uct_node
{
private:
	uct_node                       *parent    { nullptr };
	board                           position;
	const board::disk               player;
	const std::optional<std::pair<int, int> > causing_move;

	std::vector<uct_node *>         children;
	std::vector<std::pair<int, int> > unvisited;
	uint64_t                        visited   { 0  };
	double                          score     { 0. };

	bool                            game_over { false };
	bool                            first     { true  };

	std::optional<uct_node *> add_child(const std::pair<int, int> & m);

	uct_node *get_parent();
	uct_node *pick_unvisited();
	uct_node *traverse();
	uct_node *best_uct();
	void      backpropagate(uct_node *const node, double result);
	bool      fully_expanded() const;
	double    get_score();
	double    playout(const uct_node *const leaf);

	void      reset_parent() { parent = nullptr; }

public:
	uct_node(uct_node *const parent, board & position, const board::disk player, const std::optional<std::pair<int, int> > & causing_move);
	virtual ~uct_node();

	bool         is_game_over() const { return game_over; }

	board::disk  get_player() const { return player; }

	void         monte_carlo_tree_search();

	const board &get_position() const;

	const uct_node *best_child() const;
	auto         get_children() const;

	const std::pair<int, int>  get_causing_move() const;

	void         update_stats(const uint64_t visited, const double score);
	uint64_t     get_visit_count() const;
	double       get_score_count() const;

	uct_node    *find_position(const board & which);
};

std::tuple<std::optional<std::pair<int, int> >, uint64_t, uint64_t, std::vector<std::tuple<std::pair<int, int> , uint64_t, double> > > calculate_move(const board & b, const board::disk p, const uint64_t think_end_time, const uint64_t think_end_time_extra, uct_node **const root);
