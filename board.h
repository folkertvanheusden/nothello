class board
{
public:
	enum disk { empty = 0, black, white };

private:
	disk disks[8][8]; 

public:
	board();
	virtual ~board();

	void put(const int x, const int y, const disk cur);
	board::disk get(const int x, const int y) const;
};
