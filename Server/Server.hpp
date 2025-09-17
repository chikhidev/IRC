#ifndef SERVER_HPP
#define SERVER_HPP

#include "../shared/libs.hpp"
#include "../Client/Client.hpp"

class Services;

class Server {
    int port;
    std::string password;
    int fd;
    sockaddr_in addr;
    struct pollfd *poll_fds;
    int poll_count;

    Services *services;

    void addPollFd(int);
    void removePollFd(int);
    void registerClient(int, Client);
    void createClient(int);

public:
    std::map<int, Client> clients;

    Server(int);
    ~Server();

    int getFd() const;
    sockaddr_in getAddr() const;

    void setPassword(std::string&);
    bool isPasswordMatching(const std::string &) const;

    void loop();

    void sendToAllClients(const std::string &);
    void dmClient(int, const std::string &);

    bool isClientRegistered(int) const;
    void registerClient(int);
    void removeClient(int);
};


#endif