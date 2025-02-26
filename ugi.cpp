#include <cstring>
#include <cstdarg>

#include "board.h"
#include "random.h"
#include "search.h"
#include "str.h"
#include "time.h"


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
	board::disk player = board::black;

	for(;;) {
		char buffer[4096];
		if (fgets(buffer, sizeof buffer, stdin) == nullptr)
			break;

		uint64_t now = get_ts_ms();

		char *lf = strchr(buffer, '\n');
		if (lf)
			*lf = 0x00;

		auto parts = split(buffer, " ");
		if (parts.size() == 0)
			continue;

		if (parts[0] == "ugi") {
                        send("id name Samie\n");
                        send("id author Folkert van Heusden\n");
                        send("ugiok\n");
                }
                else if (parts.at(0) == "uginewgame") {
			delete b;
			b = new board(true);

			player = board::black;
		}
                else if (parts.at(0) == "query" && parts.at(1) == "p1turn") {
			send(player == board::black ? "response true\n" : "response false\n");
		}
                else if (parts.at(0) == "query" && parts.at(1) == "gameover") {
			auto valid_moves_w = b->get_valid(board::white);
			auto valid_moves_b = b->get_valid(board::black);
			send((valid_moves_w.empty() && valid_moves_b.empty()) ? "response true\n" : "response false\n");
		}
                else if (parts.at(0) == "query" && parts.at(1) == "fen") {
			send(b->emit_fen(player) + "\n");
		}
                else if (parts.at(0) == "query" && parts.at(1) == "board") {
			b->dump();
		}
                else if (parts.at(0) == "query" && parts.at(1) == "result") {
			int score = b->get_score(board::black);

			if (score > 0)
				send("response p1win\n");
			else if (score < 0)
				send("response p2win\n");
			else
				send("response draw\n");
		}
                else if (parts.at(0) == "moves") {
			auto valid_moves = b->get_valid(player);
			for(auto & move: valid_moves)
				send("%c%c ", move.first + 'a', move.second + '1');
			send("\n");
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

					i++;
                                }
                                else if (parts.at(i) == "startpos") {
					delete b;
                                        b = new board(true);
					player = board::black;
                                        i++;
                                }
                                else if (parts.at(i) == "moves") {
                                        moves = true;
                                        i++;
                                }
                                else if (moves) {
					if (parts[i] != "0000") {
						int x = tolower(parts[i][0]) - 'a';
						int y = parts[i][1] - '1';

						b->put(x, y, player);
					}

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
                                        b_time = std::stoi(parts.at(++i));
                                else if (parts.at(i) == "p2time")
                                        w_time = std::stoi(parts.at(++i));
                                else if (parts.at(i) == "p1inc")
                                        b_inc = std::stoi(parts.at(++i));
                                else if (parts.at(i) == "p2inc")
                                        w_inc = std::stoi(parts.at(++i));
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

			auto move = generate_search_move(*b, player, think_time);
			if (move.has_value() == false)
				send("bestmove 0000\n");
			else {
				b->put(move.value().first, move.value().second, player);
				send("bestmove %c%c\n", move.value().first + 'a', move.value().second + '1');
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
