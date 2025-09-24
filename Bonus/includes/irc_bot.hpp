#pragma once

#include <string>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

class IrcBot
{

	private:
		std::string _host;
		std::string _port;
		std::string _server_password;
		int _socket_fd;
		bool _joined;
		std::vector<std::string> _channels;
		std::vector<std::string> _quotes;
		std::vector<std::string> _jokes;
		void send_msg(std::string msg);
		void on_connected();
		void on_disconnected();
		void load_file(const std::string &path, std::vector<std::string> &out);
		std::string random_from(const std::vector<std::string> &vec);
		std::string extract_channel_from_privmsg(const std::string &msg);
		void send_random_quote(const std::string &channel);
		void send_random_joke(const std::string &channel);

		// Pomodoro
		time_t _cycle_start;
		bool _is_break;
		bool _timer_paused;
		bool _timer_started;
		int _work_duration; // seconds
		int _break_duration; // seconds
		void check_timer();
		void start_timer(const std::string &channel);
		void toggle_pause(const std::string &channel);
		void report_status(const std::string &channel);
		void send_help(const std::string &channel);

	public:
		IrcBot(std::string host, std::string port, const std::vector<std::string> &channels, std::string server_password);
		~IrcBot();

		void bot_connect();
		bool tick();

};
