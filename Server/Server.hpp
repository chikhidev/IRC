#ifndef SERVER_HPP
#define SERVER_HPP

#include "../shared/libs.hpp"

class Services;
class Channel;
class Client;

class Server
{
    int port;
    std::string password;
    int fd;
    sockaddr_in addr;
    bool *running;

    int epoll_fd;
    struct epoll_event event;
    struct epoll_event events[MAX_CONNECTIONS];
    int event_count;

    // ASSOCIATIONS
    Services *services;
    std::map<int, Client *> clients;
    std::map<std::string, Client *> unique_nicks;
    std::map<std::string, Channel *> channels;
    std::queue<int> clients_to_delete;

    // PRIVATE METHODS
    void addEpollFd(int);
    void removeEpollFd(int);
    void createClient();
    void makeNonBlocking(int);

public:
    Server(int);
    ~Server();

    int getFd() const;
    sockaddr_in getAddr() const;

    void setPassword(std::string &);
    bool isPasswordMatching(const std::string &) const;

    void loop();

    void sendToAllClients(const std::string &);
    void broadcastToAllClients(const std::string &);
    void dmClient(Client &, int status, const std::string &);
    void sendMessage(Client &, const std::string &);

    bool isClientRegistered(int) const;
    void registerClient(int);
    void removeClient(Client &);
    void addToDeleteQueue(Client &);
    void addToDeleteQueue(int);
    void processDeletionQueue();

    void createChannel(const std::string &, Client &);
    void removeChannel(const std::string &);

    Client *getClient(int);
    Client *getClientByNick(const std::string &);
    Channel *getChannel(const std::string &);

    void addUniqueNick(const std::string &, Client &);
    void removeUniqueNick(const std::string &);
    void updateClientNickInAllChannels(const std::string &old_nick, const std::string &new_nick, Client &client);

    void log(const std::string &message) const;

    size_t getDiffTime(size_t) const;

    void stop(int);
};

#endif