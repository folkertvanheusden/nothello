#include <cstdint>
#include <optional>
#include <unordered_set>

#include "board.h"


std::tuple<std::optional<std::pair<int, int> >, int, int> playout(const board & in, const board::disk current_player);
std::tuple<std::optional<std::pair<int, int> >, double> find_best_move(const board & in, const board::disk start_player, const int think_time);
