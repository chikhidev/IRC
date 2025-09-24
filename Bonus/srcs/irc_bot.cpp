#include "irc_bot.hpp"

#define BOT_NAME "Messi"

#define CMD_PASS(password) std::string("PASS ") + password + "\r\n"
#define CMD_NICK(nickname) std::string("NICK ") + nickname + "\r\n"
#define CMD_USER(username, realname) std::string("USER ") + username + " 0 * " + realname + "\r\n"
#define CMD_JOIN(channel) std::string("JOIN ") + channel + "\r\n"
#define CMD_PRIVMSG(target, message) std::string("PRIVMSG ") + target + " :" + std::string(message) + "\r\n"

IrcBot::IrcBot(std::string host, std::string port, const std::vector<std::string> &channels, std::string server_password)
{
	this->_host = host;
	this->_port = port;
	this->_server_password = server_password;
	this->_channels = channels;
	this->_socket_fd = -1;
	this->_joined = false;

	_cycle_start = 0;
	_is_break = false;
	_timer_paused = false;
	_timer_started = false;
	_work_duration = 10;
	_break_duration = 5;

	srand(static_cast<unsigned int>(time(NULL)));

	load_file("./additionals/quotes.txt", _quotes);
	load_file("./additionals/jokes.txt", _jokes);
}

IrcBot::~IrcBot()
{
}

void IrcBot::bot_connect()
{
	struct addrinfo hints;
	struct addrinfo *addrinfos;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(_host.c_str(), _port.c_str(), &hints, &addrinfos) != 0)
	{
		throw std::runtime_error(gai_strerror(errno));
	}

	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd < 0)
	{
		freeaddrinfo(addrinfos);
		throw std::runtime_error(strerror(errno));
	}

	if (connect(_socket_fd, addrinfos->ai_addr, addrinfos->ai_addrlen) != 0)
	{
		freeaddrinfo(addrinfos);
		close(_socket_fd);
		throw std::runtime_error(strerror(errno));
	}
	freeaddrinfo(addrinfos);
	on_connected();
}

void IrcBot::send_msg(std::string msg)
{
	if (send(_socket_fd, msg.c_str(), msg.length(), MSG_DONTWAIT | MSG_NOSIGNAL) < 0)
	{
		throw std::runtime_error(strerror(errno));
	}
}

void IrcBot::on_connected()
{
	if (_server_password.empty() == false)
		send_msg(CMD_PASS(_server_password));
	send_msg(CMD_USER(BOT_NAME, BOT_NAME));
	send_msg(CMD_NICK(BOT_NAME));
}

void IrcBot::on_disconnected()
{
	std::cout << "Disconnected" << std::endl;
}


// Helper: load lines from a file into vector
void IrcBot::load_file(const std::string &path, std::vector<std::string> &out)
{
	std::ifstream f(path.c_str());
	std::string line;
	if (!f)
		return;
	while (std::getline(f, line))
	{
		if (!line.empty())
			out.push_back(line);
	}
	f.close();
}

std::string IrcBot::random_from(const std::vector<std::string> &vec)
{
	if (vec.empty())
		return std::string("No data available");
	return vec[rand() % vec.size()];
}

void IrcBot::send_random_quote(const std::string &channel)
{
	std::string quote = random_from(_quotes);
	send_msg(CMD_PRIVMSG(channel.c_str(), quote));
}

void IrcBot::send_random_joke(const std::string &channel)
{
	std::string joke = random_from(_jokes);
	send_msg(CMD_PRIVMSG(channel.c_str(), joke));
}

std::string IrcBot::extract_channel_from_privmsg(const std::string &msg)
{
	size_t pos = msg.find("PRIVMSG ");
	if (pos == std::string::npos)
		return "";
	pos += 8;
	size_t end = msg.find(' ', pos);
	if (end == std::string::npos)
		return "";
	return msg.substr(pos, end - pos);
} // end extract_channel_from_privmsg

void IrcBot::check_timer()
{
	if (!_timer_started || _timer_paused)
		return;
	time_t now = time(NULL);
	int elapsed = static_cast<int>(now - _cycle_start);
	int target = _is_break ? _break_duration : _work_duration;
	if (elapsed >= target)
	{
		_is_break = !_is_break;
		_cycle_start = now;
		for (size_t i = 0; i < _channels.size(); ++i)
		{
			if (_is_break)
				send_msg(CMD_PRIVMSG(_channels[i].c_str(), "⏰ Time to take a break!"));
			else
				send_msg(CMD_PRIVMSG(_channels[i].c_str(), "✅ Break over. Back to work!"));
		}
	}
}

void IrcBot::toggle_pause(const std::string &channel)
{
	if (!_timer_started)
	{
		send_msg(CMD_PRIVMSG(channel.c_str(), "Timer not running. Use !start first."));
		return;
	}
	_timer_paused = !_timer_paused;
	std::string msg = _timer_paused ? "⏸️ Pomodoro paused." : "▶️ Pomodoro resumed.";
	send_msg(CMD_PRIVMSG(channel.c_str(), msg));
}

void IrcBot::start_timer(const std::string &channel)
{
	if (_timer_started)
	{
		send_msg(CMD_PRIVMSG(channel.c_str(), "Timer already running."));
		return;
	}
	_timer_started = true;
	_timer_paused = false;
	_is_break = false;
	_cycle_start = time(NULL);
	send_msg(CMD_PRIVMSG(channel.c_str(), "▶️ Pomodoro started! Work for 25 minutes."));
}

void IrcBot::report_status(const std::string &channel)
{
	if (!_timer_started)
	{
		send_msg(CMD_PRIVMSG(channel.c_str(), "Timer not started. Use !start to begin."));
		return;
	}
	time_t now = time(NULL);
	int elapsed = static_cast<int>(now - _cycle_start);
	int target = _is_break ? _break_duration : _work_duration;
	int remaining = target - elapsed;
	if (remaining < 0) remaining = 0;
	std::string phase = _is_break ? "Break" : "Work";
	char buffer[128];
	snprintf(buffer, sizeof(buffer), "%s phase: %d min %d sec remaining%s", phase.c_str(), remaining/60, remaining%60, _timer_paused?" (paused)":"");
	send_msg(CMD_PRIVMSG(channel.c_str(), buffer));
}

void IrcBot::send_help(const std::string &channel)
{
	const char *helpMsg =
		"Available commands: "
		"!start – begin Pomodoro cycle | "
		"!pause – pause/resume timer | "
		"!status – show phase & time left | "
		"!quote – random quote | "
		"!joke – random joke | "
		"!help – this message";
	send_msg(CMD_PRIVMSG(channel.c_str(), helpMsg));
}

bool IrcBot::tick()
{
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(_socket_fd, &readfds);
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	int sel = select(_socket_fd + 1, &readfds, NULL, NULL, &tv);
	if (sel < 0)
	{
		close(_socket_fd);
		on_disconnected();
		throw std::runtime_error(strerror(errno));
	}

	check_timer();

	if (sel == 0)
	{
		return true;
	}

	int readed_count = 0;
	char buff[1025];

	readed_count = read(_socket_fd, buff, 1024);
	if (readed_count == 0)
	{
		close(_socket_fd);
		on_disconnected();
		return false;
	}
	else if (readed_count < 0)
	{
		close(_socket_fd);
		on_disconnected();
		throw std::runtime_error(strerror(errno));
	}

	buff[readed_count] = '\0';
	std::string msg(buff);
	std::cout << msg << std::flush;

	check_timer();

	if (msg.rfind("PING :", 0) == 0)
	{
		send_msg("PONG :" + msg.substr(6));
		return true;
	}

	if (_joined == false && msg.find(" 001 ") != std::string::npos)
	{
		for (std::vector<std::string>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		{
			send_msg(CMD_JOIN((*it).c_str()));
			send_msg(CMD_PRIVMSG((*it).c_str(), "Type !help to see all the cool things I can do!"));
		}
		_joined = true;
	}

	std::string chan = extract_channel_from_privmsg(msg);
	if (chan.empty())
		return true;

	if (std::find(_channels.begin(), _channels.end(), chan) == _channels.end())
		return true;

	if (msg.find("!quote") != std::string::npos)
		send_random_quote(chan);
	else if (msg.find("!joke") != std::string::npos)
		send_random_joke(chan);
	else if (msg.find("!start") != std::string::npos)
		start_timer(chan);
	else if (msg.find("!pause") != std::string::npos)
		toggle_pause(chan);
	else if (msg.find("!status") != std::string::npos)
		report_status(chan);
	else if (msg.find("!help") != std::string::npos)
		send_help(chan);
	return true;
}
