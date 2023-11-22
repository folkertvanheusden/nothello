#include <cstring>
#include <cstdarg>

#include "board.h"
#include "playout.h"
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

void ugi()
{
	board      *b      = new board(true);
	board::disk player = board::white;

	for(;;) {
		char buffer[4096];
		if (fgets(buffer, sizeof buffer, stdin) == nullptr)
			break;

		char *lf = strchr(buffer, '\n');
		if (lf)
			*lf = 0x00;

		auto parts = split(buffer, " ");

		if (parts[0] == "ugi") {
                        send("id name Nothello\n");
                        send("id author Folkert van Heusden\n");
                        send("ugiok\n");
                }
                else if (parts.at(0) == "uginewgame") {
			delete b;
			b = new board(true);

			player = board::white;
		}
                else if (parts.at(0) == "query" && parts.at(1) == "p1turn") {
			send(player == board::white ? "response true\n" : "response false\n");
		}
                else if (parts.at(0) == "query" && parts.at(1) == "gameover") {
			auto valid_moves = b->get_valid(player);
			send(valid_moves.empty() ? "response true\n" : "response false\n");
		}
                else if (parts.at(0) == "query" && parts.at(1) == "result") {
			int score = b->get_score(board::white);

			if (score > 0)
				send("response p1win\n");
			else if (score < 0)
				send("response p2win\n");
			else
				send("response draw\n");
		}
                else if (parts.at(0) == "position") {
                        bool moves = false;

                        for(size_t i=1; i<parts.size();) {
                                if (parts.at(i) == "fen") {
                                        std::string fen   = parts.at(++i);
					std::string color = parts.at(++i);

					delete b;
                                        b = new board(fen);

					player = color == "o" ? board::white : board::black;

                                        break;
                                }
                                else if (parts.at(i) == "startpos") {
					delete b;
                                        b = new board(true);
					player = board::white;
                                        i++;
                                }
                                else if (parts.at(i) == "moves") {
                                        moves = true;
                                        i++;
                                }
                                else if (moves) {
					int x = tolower(parts[i][0]) - 'a';
					int y = parts[i][1] - '1';

					b->put(x, y, player);

					player = player == board::black ? board::white : board::black;

                                        i++;
                                }
                                else {
					fprintf(stderr, "%s not understood\n", parts.at(i).c_str());
                                }
                        }
                }
                else if (parts.at(0) == "go") {
                        int moves_to_go = b->estimate_total_move_count() / 2;

                        int w_time = 0, b_time = 0, w_inc = 0, b_inc = 0;
                        bool time_set = false;

                        for(size_t i=1; i<parts.size(); i++) {
                                if (parts.at(i) == "movetime") {
                                        w_time = b_time = std::stoi(parts.at(++i));
                                        time_set = true;
                                }
                                else if (parts.at(i) == "p1time")
                                        w_time = std::stoi(parts.at(++i));
                                else if (parts.at(i) == "p2time")
                                        b_time = std::stoi(parts.at(++i));
                                else if (parts.at(i) == "p1inc")
                                        w_inc = std::stoi(parts.at(++i));
                                else if (parts.at(i) == "p2inc")
                                        b_inc = std::stoi(parts.at(++i));
                                else if (parts.at(i) == "movestogo")
                                        moves_to_go = std::stoi(parts.at(++i));
                        }

                        int think_time = 0;
                        if (time_set)
                                think_time = (player == board::white ? w_time : b_time) * 0.95;
                        else {
                                int cur_n_moves = moves_to_go <= 0 ? 40 : moves_to_go;

                                int time_inc = player == board::white ? w_inc : b_inc;

                                int ms = player == board::white ? w_time : b_time;
                                think_time = (ms + (cur_n_moves - 1) * time_inc) / double(cur_n_moves + 7);

                                int limit_duration_min = ms / 15;
                                if (think_time > limit_duration_min)
                                        think_time = limit_duration_min;
                        }

                        // 50ms overhead
                        if (think_time > 50)
                                think_time -= 50;

			auto rc   = find_best_move(*b, player, think_time);
			auto move = std::get<0>(rc);
			if (move.has_value() == false) {
				send("bestmove 0000\n");
			}
			else {
				//fprintf(stderr, "I play: %c%c (%.2f playouts per second)\n", move.value().first + 'A', move.value().second + '1', std::get<1>(rc));

				b->put(move.value().first, move.value().second, player);

				send("bestmove %c%c\n", move.value().first + 'A', move.value().second + '1');
			}

			player = player == board::black ? board::white : board::black;
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
