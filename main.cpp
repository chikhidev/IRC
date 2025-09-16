#include "Server/Server.hpp"

int main(int ac, char**av) {

    if (ac != 3) {
        std::cerr << "Usage: ircserv <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(av[1]);
    std::string password = av[2];

    try {
        Server server(port);
        server.setPassword(password);

        server.start();


    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

