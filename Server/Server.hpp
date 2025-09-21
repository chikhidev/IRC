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

    // ASSOCIATIONS
    Services *services;
    std::map<int, Client*> clients;
    std::map<std::string, Client*> unique_nicks;
    std::map<std::string, Channel*> channels;

    // PRIVATE METHODS
    void addPollFd(int);
    void removePollFd(int);
    void registerClient(int, Client);
    void createClient();

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
    void sendMessage(Client&, const std::string&);

    bool isClientRegistered(int) const;
    void registerClient(int);
    void removeClient(int);
    void removeClient(Client&);

    void createChannel(const std::string &, Client&);
    void removeChannel(const std::string &);
    void addClientToChannel(const std::string &, Client &);
    void removeClientFromChannel(const std::string &, Client &);
    
    Client& getClient(int);
    Channel& getChannel(const std::string&);

    bool channelExists(const std::string&) const;

    Client* existingNick(const std::string &);
    void addUniqueNick(const std::string &, Client &);
    void removeUniqueNick(const std::string &);

    void log(const std::string &message) const;
};


#endif