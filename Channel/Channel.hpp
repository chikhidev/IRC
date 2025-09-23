#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "../shared/libs.hpp"

class Server;
class Client;

class Channel {
    Server *server;
    std::string name;
    std::map<int, Client*> members;
    std::string topic;
    std::map<std::string, Client*> operators;
    std::map<char, bool> modes;
    std::string password;
    size_t user_limit;
    std::map<std::string, Client*> invited;



    
    void initModes();

public:
    Channel();
    Channel(const std::string &, Client &, Server *);
    ~Channel();

    std::string getName() const;
    std::string getTopic() const;

    void setTopic(const std::string &);
    
    void addMember(Client &);
    void removeMember(Client &);
    bool isMember(const Client &) const;
    bool isOperator(const Client &) const;

    void broadcastToMembers(Client &, const std::string &);
    void listMembers(Client &) const;

    bool isEmpty() const;

    bool mode(char) const;
    void updateMode(char, bool);
    void updatePassword(const std::string &);
    void updateUserLimit(size_t);
    void addOperator(Client &);
    void removeOperator(Client &);

    bool isFull() const;
    bool isInvited(Client &) const;
    void addInvited(Client &);
    void removeInvited(Client &);

    bool isMatchingPassword(const std::string &) const;
    size_t getOperatorsCount() const;
};

#endif