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
    bool _isAuthenticated;
    bool _isRegistered;

public:

    Client();
    Client(int, sockaddr_in, socklen_t);
    ~Client();

    int getFd() const;
    sockaddr_in getAddr();
    socklen_t getAddrLen();
    std::string getNick() const;
    std::string getUsername() const;
        
    void setAddr(sockaddr_in);
    void setAddrLen(socklen_t);
    void setFd(int);
    void setRegistered(bool);
    void setAuthenticated(bool);
    void setNickname(const std::string &);
    void setUsername(const std::string &);
    void setRealname(const std::string &);

    bool hasNick() const;
    bool isRegistered() const;
    bool isAuthenticated() const;

};


#endif