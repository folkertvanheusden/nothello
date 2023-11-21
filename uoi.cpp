#include <cstdarg>

#include "board.h"
#include "str.h"


void send(const std::string & fmt, ...)
{
	char *str = nullptr;

	va_list ap;
        va_start(ap, fmt);
        (void)vasprintf(&str, fmt.c_str(), ap);
        va_end(ap);

	printf("%s", str);
	fflush(nullptr);

	free(str);
}

void uoi()
{
	board      *b      = new board(true);
	board::disk player = board::black;

	for(;;) {
		char buffer[4096];
		if (fgets(buffer, sizeof buffer, stdin) == nullptr)
			break;

		auto parts = split(buffer, " ");

		if (parts[0] == "uoi") {
                        send("id name Nothello\n");
                        send("id author Folkert van Heusden\n");
                        send("uoiok\n");
                }
                else if (parts.at(0) == "uainewgame") {
			delete b;
			b = new board(true);

			player = board::black;
		}
                else if (parts.at(0) == "position") {
                        bool moves = false;

                        for(size_t i=1; i<parts.size();) {
                                if (parts.at(i) == "fen") {
                                        std::string fen;

                                        for(size_t f = i + 1; f<parts.size(); f++)
                                                fen += parts.at(f) + " ";

					delete b;
                                        b = new board(fen);

                                        break;
                                }
                                else if (parts.at(i) == "startpos") {
					delete b;
                                        b = new board(true);
                                        i++;
                                }
                                else if (parts.at(i) == "moves") {
                                        moves = true;
                                        i++;
                                }
                                else if (moves) {
                                        while(i < parts.size() && parts.at(i).size() < 4)
                                                i++;

					int x = tolower(parts[i][0]) - 'a';
					int y = parts[i][1] - '1';

					b ->put(x, y, player);

					player = player == board::black ? board::white : board::black;

                                        i++;
                                }
                                else {
                                }
                        }
                }
                else if (parts.at(0) == "go") {
                        int moves_to_go = 40 - pos.get_halfmoves() / 2;
                        int w_time = 0, b_time = 0, w_inc = 0, b_inc = 0;
                        bool time_set = false;

                        for(size_t i=1; i<parts.size(); i++) {
                                if (parts.at(i) == "movetime") {
                                        w_time = b_time = atoi(parts.at(++i).c_str());
                                        time_set = true;
                                }
                                else if (parts.at(i) == "wtime")
                                        w_time = atoi(parts.at(++i).c_str());
                                else if (parts.at(i) == "btime")
                                        b_time = atoi(parts.at(++i).c_str());
                                else if (parts.at(i) == "winc")
                                        w_inc = atoi(parts.at(++i).c_str());
                                else if (parts.at(i) == "binc")
                                        b_inc = atoi(parts.at(++i).c_str());
                                else if (parts.at(i) == "movestogo")
                                        moves_to_go = atoi(parts.at(++i).c_str());
                        }

                        int think_time = 0;
                        if (time_set)
                                think_time = (pos.get_turn() == libataxx::Side::White ? w_time : b_time) * 0.95;
                        else {
                                int cur_n_moves = moves_to_go <= 0 ? 40 : moves_to_go;

                                int time_inc = pos.get_turn() == libataxx::Side::White ? w_inc : b_inc;

                                int ms = pos.get_turn() == libataxx::Side::White ? w_time : b_time;
                                think_time = (ms + (cur_n_moves - 1) * time_inc) / double(cur_n_moves + 7);

                                int limit_duration_min = ms / 15;
                                if (think_time > limit_duration_min)
                                        think_time = limit_duration_min;
                        }

                        // 50ms overhead
                        if (think_time > 50)
                                think_time -= 50;

                        libataxx::Move move = calculate_move(pos, think_time);

                        send("bestmove %c%c\n" << move << std::endl;
                }
                else if (parts.at(0) == "isready")
			send("readyok\n");
                else if (parts.at(0) == "quit") {
                        break;
                }
                else {
                        send("Invalid command: %s\n", buffer);
                }
	}

	delete b;
}
