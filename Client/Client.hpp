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

public:

    Client();
    Client(int, sockaddr_in, socklen_t);
    ~Client();

    int getFd() const;
    sockaddr_in getAddr();
    socklen_t getAddrLen();
    void setAddr(sockaddr_in);
    void setAddrLen(socklen_t);

    void setFd(int);
    bool isRegisteredClient() const;
};


#endif