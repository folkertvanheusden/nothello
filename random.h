#include <cstdint>
#include <optional>
#include <random>


std::uint_least32_t produce_seed();
extern thread_local std::uint_least32_t mt_seed;
extern thread_local std::mt19937_64 gen;

std::optional<std::pair<int, int> > generate_move(const board & b, const board::disk player);
