#include "Channel.hpp"
#include "../Client/Client.hpp"
#include "../Server/Server.hpp"

Channel::Channel() : name(""), _operator(NULL), server(NULL) {
    std::cout << "Default Channel constructor called" << std::endl;
}

Channel::Channel(const std::string &channel_name, Client &creator, Server *srv) : server(srv) { 
    name = channel_name;
    _operator = &creator;
    members[creator.getFd()] = &creator;
}

Channel::~Channel() {}

std::string Channel::getName() const {
    return name;
}

void Channel::addMember(Client &client) {
    if (isMember(client)) {
        throw std::runtime_error("Client is already a member of the channel");
    }
    members[client.getFd()] = &client;
}

void Channel::removeMember(Client &client) {
    std::map<int, Client*>::iterator it = members.find(client.getFd());
    if (it != members.end()) {
        members.erase(it);
    } else {
        throw std::runtime_error("Client is not a member of the channel");
    }
}

/*
* Check if a client is a member of the channel
*/
bool Channel::isMember(const Client &client) const {
    return members.find(client.getFd()) != members.end();
}

/*
* Broadcast a message to all members of the channel
*/
void Channel::broadcastToMembers(Client &sender, const std::string &message) {
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    for (std::map<int, Client*>::iterator it = members.begin(); it != members.end(); ++it) {
        if (it->first != sender.getFd()) {
            sender.sendMessage(*(it->second), message);
        }
    }
}

/*
* List all members of the channel
*/
void Channel::listMembers() const {
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    std::cout << "Members of channel " << name << ":" << std::endl;
    for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
        server->dmClient(*(it->second), 0, "Members of channel " + name + ":");
    }
}

std::string Channel::getTopic() const {
    return topic;
}