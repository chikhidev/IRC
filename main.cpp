#include "Server/Server.hpp"

/*
* Main entry point for the IRC server
* Any MACRO you see you will find it in shared/libs.hpp, the included libraries are also there
* The program is seperated into 3 main modules:
* - Server: handles all server related operations (networking, clients management, channels management, relations etc.)
* - The server uses epoll for efficient I/O multiplexing
* - Client: represents a connected client and its state
* - Services: handles all IRC commands and services (authentication, messaging, channel operations, etc
* Relations:
*   - Server has many Clients
*   - Server has many Channels
*   - Client can join many Channels
*   - Channel has many Clients
*   - Server has one Services instance to pass client requests to it
*   - Services has server instance to operate on it
*   - Client has server instance to query it
*   Due to any error the program will throw an exception and exit
*/

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
        server.loop();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

    