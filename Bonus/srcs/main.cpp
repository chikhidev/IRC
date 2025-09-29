#include "../includes/irc_bot.hpp"
#include <exception>
#include <iostream>
#include <csignal>

static volatile sig_atomic_t g_stop = 0;

static void sigint_handler(int /*signum*/)
{
    g_stop = 1;
}

int main(int argc, char **argv)
{

	if (argc < 4)
	{
		std::cerr << "Usage: <host> <port> <channels(comma separated)> [password]" << std::endl;
		return (1);
	}

	std::string host = argv[1];
	std::string port = argv[2];
	std::string channelArg = argv[3];
	std::string password = argc == 5 ? argv[4] : "";

	std::vector<std::string> channels;
	size_t start = 0, comma;
	while ((comma = channelArg.find(',', start)) != std::string::npos)
	{
		channels.push_back(channelArg.substr(start, comma - start));
		start = comma + 1;
	}
	channels.push_back(channelArg.substr(start));

	IrcBot bot(host, port, channels, password);

    std::signal(SIGINT, sigint_handler);

	try {
		bot.bot_connect();

		bool running = true;
		while (running && !g_stop)
		{
			running = bot.tick();
		}
	} catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}
