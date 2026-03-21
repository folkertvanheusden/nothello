#include <cstring>
#include <cstdarg>
#include <set>
#include <string>

#include "board.h"
#include "random.h"
#include "search.h"
#include "str.h"
#include "time.h"


static void send(const std::string & fmt, ...)
{
	char *str = nullptr;

	va_list ap;
        va_start(ap, fmt);
        (void)vasprintf(&str, fmt.c_str(), ap);
        va_end(ap);

	printf("%s\n", str);
	fflush(nullptr);

	free(str);
}

std::optional<board::disk> str_to_disk(const std::string & in)
{
	if (in == "black" || in == "b" || in == "B" || in == "x" || in == "X")
		return board::black;
	if (in == "white" || in == "w" || in == "W" || in == "o" || in == "O")
		return board::white;
	return { };
}

void gtp()
{
	board *b         = new board(INITIAL_FEN);
	int    calc_time = 1000;
	std::set<std::string> commands { "protocol_version", "name", "version", "known_command", "list_commands",
					"quit", "boardsize", "clear_board", "komi", "play", "genmove",
					"time_settings", "gogui-rules_final_result" };

	for(;;) {
		char buffer[4096];
		if (fgets(buffer, sizeof buffer, stdin) == nullptr)
			break;

		char *lf = strchr(buffer, '\n');
		if (lf)
			*lf = 0x00;

		if (lf == buffer + 1)
			continue;

		auto parts = split(buffer, " ");
		if (parts.size() == 0)
			continue;

		std::string id;
		int offset = 0;
		if (isdigit(parts[0][0])) {
			offset = 1;
			id = parts[0];
		}

		std::string & cmd = parts[offset];

		if (cmd == "protocol_version")
			send(id + "= 2\n");
		else if (cmd == "name")
			send(id + "= Samei\n");
		else if (cmd == "version")
			send(id + "= 0.1\n");
		else if (cmd == "quit")
			break;
		else if (cmd == "boardsize") {
			if (parts[offset + 1] != "8")
				send("?" + id + "\n");
			else
				send(id + "= ok\n");
		}
		else if (cmd == "clear_board") {
			delete b;
			b = new board(INITIAL_FEN);
			send(id + "= ok\n");
		}
		else if (cmd == "komi")
			send(id + "= ok\n");
		else if (cmd == "gogui-rules_final_result") {
			auto score = b->get_score(board::black);
			if (score > 0)
				send(id + "= black\n");
			else if (score < 0)
				send(id + "= white\n");
			else
				send(id + "= draw\n");
		}
		else if (cmd == "time_settings") {
			calc_time = std::stoi(parts[++offset]) * 1000;
			send(id + "= ok\n");
		}
		else if (cmd == "list_commands") {
			for(auto & cmd: commands)
				send(id + "= " + cmd);
			send("\n");
		}
		else if (cmd == "known_command") {
			bool has = commands.find(parts[++offset]) != commands.end();
			send(id + "= " + (has ? "true": "false") + "\n");
		}
		else if (cmd == "play") {
			auto color = str_to_disk(parts[++offset]);
			auto move  = parts[++offset];

			if (color.has_value() && move.size() == 2) {
				int x = toupper(move.at(0)) - 'A';
				int y = toupper(move.at(1)) - '1';
				b->put(x, y, color.value());
				send(id + "= ok\n");
			}
			else {
				send("?" + id + " invalid move?\n");
			}
		}
		else if (cmd == "genmove") {
			auto color = str_to_disk(parts[++offset]);
			if (color.has_value() == false) {
				send("?" + id + " invalid colo?\n");
				continue;
			}

                        int moves_to_go = b->estimate_total_move_count() / 2;
                        int think_time = calc_time / double(moves_to_go + 7);
			int limit_duration_min = calc_time / 15;
			if (think_time > limit_duration_min)
				think_time = limit_duration_min;

                        // 50ms overhead
                        if (think_time > 5)
                                think_time -= 5;

			auto move = generate_search_move(*b, color.value(), think_time);
			if (move.has_value() == false)
				send(id + "= pass\n");
			else {
				b->put(move.value().first.first, move.value().first.second, color.value());
				send(id + "= %c%c\n", move.value().first.first + 'a', move.value().first.second + '1');
			}
                }
                else {
                        send("?" + id + " eh?\n");
                }
	}

	delete b;
}
