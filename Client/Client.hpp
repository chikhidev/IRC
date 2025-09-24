#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../shared/libs.hpp"

class Server;
class Channel;

class Client {
    Server* server;
    sockaddr_in addr;
    socklen_t addr_len;
    int fd;
    std::string nickname;
    std::string username;
    std::string realname;
    bool _connected;
    bool _isAuthenticated;
    bool _isRegistered;
    bool _sent_first_command;
    std::string command_terminators;
    std::vector<std::string> joined_channels;
    std::stringstream command_buffer;
    int last_active_time;

public:

    Client();
    Client(int, sockaddr_in, socklen_t, Server*);
    ~Client();

    int getFd() const;
    sockaddr_in getAddr();
    socklen_t getAddrLen();
    std::string getNick() const;
    std::string getUsername() const;
    std::string getRealname() const;
    std::string getCommandTerminators() const;
    
    void setAddr(sockaddr_in);
    void setAddrLen(socklen_t);
    void setFd(int);
    void setRegistered(bool);
    void setAuthenticated(bool);
    void setNick(const std::string &);
    void setUsername(const std::string &);
    void setRealname(const std::string &);
    void disconnect();
    void setCommandTerminators(const std::string &);
    void setSentFirstCommand();
    void sendMessage(Client&, const std::string&);
    void addToJoinedChannels(const std::string&);
    void removeFromJoinedChannels(const std::string&);
    void quitAllChannels();

    bool hasNick() const;
    bool isRegistered() const;
    bool isAuthenticated() const;
    bool isConnected() const;
    bool hasSentFirstCommand() const;

    bool operator==(const Client &other) const;
    bool operator!=(const Client &other) const;

    std::stringstream& getCommandStream();
    void clearCommandStream();

    size_t getLastActiveTime() const;
    void setLastActiveTime(size_t);
};


#endif