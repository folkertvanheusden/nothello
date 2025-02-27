#include <cstdint>
#include <optional>
#include <utility>


typedef enum { NOTVALID = 0, EXACT = 1, LOWERBOUND = 2, UPPERBOUND = 3 } tt_entry_flag;

typedef struct __attribute__ ((__packed__))
{
	uint32_t hash;
	int16_t score;
	uint8_t depth: 7;
	uint8_t x: 3; 
	uint8_t y: 3; 
	uint8_t move_valid: 1;
	uint8_t flags: 2;
} tt_entry;

class tt
{
private:
	tt_entry *entries;
	size_t n_entries;

	size_t tt_index(const uint64_t hash);

public:
	tt();
	virtual ~tt();

        std::optional<tt_entry> lookup(const uint64_t board_hash);
        void store(const uint64_t hash, const tt_entry_flag f, const int d, const int score, const std::optional<std::pair<int, int> > & m);
};

int eval_from_tt(const int eval, const int ply);
int eval_to_tt(const int eval, const int ply);
