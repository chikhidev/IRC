#ifndef SERVER_HPP
#define SERVER_HPP

#include "../shared/libs.hpp"
#include "../Client/Client.hpp"

class Server {
    int port;
    std::string password;
    int fd;
    sockaddr_in addr;
    std::map<int, Client> clients;
    struct pollfd *poll_fds;
    int poll_count;

    void addPollFd(int);
    void removePollFd(int);
    void removeClient(int);
    void registerClient(int, Client);
    void createClient(int);

    // Command handling
    void commandHandler(int, std::string);

public:
    Server(int);
    ~Server();

    int getFd() const;
    sockaddr_in getAddr() const;

    void setPassword(std::string&);

    void start();
};


#endif