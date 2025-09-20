#ifndef SERVER_HPP
#define SERVER_HPP

#include "../shared/libs.hpp"

class Services;
class Channel;
class Client;

class Server {
    int port;
    std::string password;
    int fd;
    sockaddr_in addr;
    struct pollfd *poll_fds;
    int poll_count;

    Services *services;
    std::map<int, Client> clients;
    std::map<std::string, Channel> channels;

    // PRIVATE METHODS
    void addPollFd(int);
    void removePollFd(int);
    void registerClient(int, Client);
    void createClient(int);

public:

    Server(int);
    ~Server();

    int getFd() const;
    sockaddr_in getAddr() const;

    void setPassword(std::string&);
    bool isPasswordMatching(const std::string &) const;

    void loop();

    void sendToAllClients(const std::string &);
    void dmClient(Client&, int status, const std::string &);

    bool isClientRegistered(int) const;
    void registerClient(int);
    void removeClient(int);
    void removeClient(Client&);

    void createChannel(const std::string &, Client&);
    void removeChannel(const std::string &);
    
    Client& getClient(int);
    Channel& getChannel(const std::string&);
};


#endif