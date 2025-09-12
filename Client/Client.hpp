#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../shared/libs.hpp"

class Client {
    sockaddr_in addr;
    socklen_t addr_len;
    int fd;
    std::string nickname;
    std::string username;
    std::string realname;
    bool isRegistered;
    bool isOperator;

public:

    Client();
    ~Client() throw();

    int getFd() const;
    sockaddr_in* getAddr();
    socklen_t* getAddrLen();

    void setFd(int);

};

#endif