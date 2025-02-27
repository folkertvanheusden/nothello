#include <algorithm>
#include <optional>
#include <random>
#include <vector>

#include "board.h"


auto produce_seed()
{
	std::vector<unsigned int> random_data(std::mt19937::state_size);

	std::random_device source;
	std::generate(std::begin(random_data), std::end(random_data), [&](){return source();});

	return std::seed_seq(std::begin(random_data), std::end(random_data));
}

thread_local auto mt_seed = produce_seed();
thread_local std::mt19937_64 gen { mt_seed };

thread_local auto rng = std::default_random_engine {};

std::optional<std::pair<int, int> > generate_random_move(const board & b, const board::disk player)
{
        auto moves = b.get_valid(player);
        if (moves.empty())
                return { };

        std::shuffle(std::begin(moves), std::end(moves), rng);

        return moves.at(0);
}
