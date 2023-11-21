#pragma once
#include <string>
#include <vector>


class board
{
public:
	enum disk { empty = 0, black, white };

private:
	disk disks[8][8] { empty }; 

	bool scan(const int start_x, const int start_y, const int dx, const int dy, const disk cur) const;
	void scan_and_flip(const int start_x, const int start_y, const int dx, const int dy);

protected:
	void get_to(disk d[][8]) const;
	void set_fen(const std::string & fen);

public:
	board(const bool set_initial);
	board(const std::string & fen);
	board(const board & in);
	virtual ~board();

	board & operator=(const board & in);

	bool   is_valid(const int x, const int y, const disk cur) const;
	std::vector<std::pair<int, int> > get_valid(const disk cur) const;
	void        put(const int x, const int y, const disk cur);
	board::disk get(const int x, const int y) const;

	void        dump() const;
	std::string emit_fen(const disk current_player) const;

	int get_score(const disk for_whom) const;
	int estimate_total_move_count() const;
};
