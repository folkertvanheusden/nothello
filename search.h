#include <set>

bool update_and_check_repetition(std::set<uint64_t> *const history, const board & b, const board::disk player);
std::optional<std::pair<std::pair<int, int>, int> > generate_search_move(const board & b, const board::disk player, const int search_time);
