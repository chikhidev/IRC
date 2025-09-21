#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "../shared/libs.hpp"

class Server;
class Client;

class Channel {
    Server *server;
    std::string name;
    std::map<int, Client*> members;
    Client* _operator;
    std::string topic;
    std::map<char, bool> modes;
    std::string password;

    void initModes();

public:
    Channel();
    Channel(const std::string &, Client &, Server *);
    ~Channel();

    std::string getName() const;
    std::string getTopic() const;
    
    void addMember(Client &);
    void removeMember(Client &);
    bool isMember(const Client &) const;

    void broadcastToMembers(Client &, const std::string &);
    void listMembers(Client &) const;

    bool isEmpty() const;
};

#endif