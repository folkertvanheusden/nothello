#include "tt.h"
#include "zobrist.h"


tt::tt()
{
	n_entries = 256 * 1024 * 1024 / sizeof(tt_entry);
	entries = new tt_entry[n_entries]();

	init_zobrist();
}

tt::~tt()
{
	delete [] entries;
}

size_t tt::tt_index(const uint64_t hash)
{
	return hash % n_entries;
}

std::optional<tt_entry> tt::lookup(const uint64_t hash)
{
	size_t index = tt_index(hash);

	if (entries[index].hash != (hash & 0xffffffff))
		return { };

	return entries[index];
}

void tt::store(const uint64_t hash, const tt_entry_flag f, const int d, const int score, const std::optional<std::pair<int, int> > & m)
{
	size_t index = tt_index(hash);

	tt_entry new_entry { };
	new_entry.hash = hash;
	new_entry.score = score;
	new_entry.depth = d;
	new_entry.move_valid = m.has_value();
	if (m.has_value()) {
		new_entry.x = m.value().first;
		new_entry.y = m.value().second;
	}
	new_entry.flags = f;

	entries[index] = new_entry;
}

int eval_to_tt(const int eval, const int ply)
{
        if (eval > 9800)
                return eval + ply;
        if (eval < -9800)
                return eval - ply;
        return eval;
}

int eval_from_tt(const int eval, const int ply)
{
        if (eval > 9800)
                return eval - ply;
        if (eval < -9800)
                return eval + ply;
        return eval;
}
