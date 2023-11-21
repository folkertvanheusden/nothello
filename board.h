#pragma once
#include <vector>


class board
{
public:
	enum disk { empty = 0, black, white };

private:
	disk disks[8][8] { empty }; 

	bool scan(const int start_x, const int start_y, const int dx, const int dy, const disk cur);
	void scan_and_flip(const int start_x, const int start_y, const int dx, const int dy);

protected:
	void get_to(disk d[][8]) const;

public:
	board(const bool set_initial);
	board(const board & in);
	virtual ~board();

	board & operator=(const board & in);

	bool   is_valid(const int x, const int y, const disk cur);
	std::vector<std::pair<int, int> > get_valid(const disk cur);
	void        put(const int x, const int y, const disk cur);
	board::disk get(const int x, const int y) const;

	void dump() const;

	int get_score(const disk for_whom);
};
