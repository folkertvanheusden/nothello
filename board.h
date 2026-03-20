#pragma once
#include <cstdint>
#include <string>
#include <vector>


#define INITIAL_FEN "8/8/8/3ox3/3xo3/8/8/8"

class board
{
public:
	enum disk { empty = 0, black, white };

private:
	uint64_t disks[3] { };

	uint64_t shift(const uint64_t disks, const int direction, const int S, const uint64_t M) const;

protected:
	void get_to(board & target) const;
	void set(const int x, const int y, board::disk d);
	void set_fen(const std::string & fen);

public:
	board();
	board(const std::string & fen);
	board(const char *const fen);
	board(const board & in);
	virtual ~board();

	board & operator= (const board & in );
	bool    operator==(const board & rhs) const;

	uint64_t    get_bitboard(const disk color) const;
	uint64_t    get_possible_moves(const disk color) const;
	std::vector<std::pair<int, int> > get_possible_move_list(const disk color) const;
	board::disk get(const int x, const int y) const;
	void        put(const int x, const int y, const disk color);
	void        put(const int move, const disk color);

	void        dump() const;
	std::string emit_fen(const disk current_player) const;

	int get_score(const disk for_whom) const;
	int estimate_total_move_count() const;
};

board::disk opponent_color(const board::disk & cur);
