#ifndef SERVER_HPP
#define SERVER_HPP

#include "../shared/libs.hpp"
#include "../Client/Client.hpp"

class Server {
    int port;
    std::string password;
    int fd;
    sockaddr_in addr;
    std::map<std::string, Client> clients;
    struct pollfd *poll_fds;
    int poll_count;

public:
    Server(int);
    ~Server() throw();

    int getFd() const;
    sockaddr_in getAddr() const;

    void setPassword(std::string&);

    void start();
    void createClient(int, sockaddr_in, socklen_t);

};


#endif