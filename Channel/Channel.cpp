#include "Channel.hpp"
#include "../Client/Client.hpp"
#include "../Server/Server.hpp"

Channel::Channel() : name(""), _operator(NULL), server(NULL) {
    std::cout << "Default Channel constructor called" << std::endl;
    initModes();
}

Channel::Channel(const std::string &channel_name, Client &creator, Server *srv) : server(srv) { 
    name = channel_name;
    _operator = &creator;
    members[creator.getFd()] = &creator;
    initModes();
}

void Channel::initModes() {
    modes['i'] = false; // Invite-only
    modes['t'] = true; // Topic settable by channel operator only
    modes['k'] = false; // Key (password) required to join
    modes['o'] = false; // Operator status
    modes['l'] = false; // User limit
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
    client.addToJoinedChannels(name);
}

void Channel::removeMember(Client &client) {

    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    std::map<int, Client*>::iterator it = members.find(client.getFd());
    if (it != members.end()) {

        if (it->second == _operator) {
            broadcastToMembers(client, "NOTICE #" + name + " :The channel operator has left the channel, the channel is gone");
            server->removeChannel(name);
            return;
        }

        members.erase(it);
        client.removeFromJoinedChannels(name);

        if (members.empty()) {
            std::cout << "[CHANNEL] Channel " << name << " is empty. Removing it from server." << std::endl;
            server->removeChannel(name);
        }

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
* Check if a client is an operator of the channel
*/
bool Channel::isOperator(const Client &client) const {
    return _operator && _operator == &client;
}

/*
* Broadcast a message to all members of the channel
*/
void Channel::broadcastToMembers(Client &sender, const std::string &message) {
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    // check if sender is a member
    if (!isMember(sender)) {
        throw std::runtime_error("Not a member of the channel");
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
void Channel::listMembers(Client &client) const {
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    std::cout << "Members of channel " << name << ":" << std::endl;

    std::string response = name + " :@" + _operator->getNick() + " ";

    for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
        if (it->second != _operator) {
            response += it->second->getNick() + " ";
        }
    }

    server->dmClient(client, 353, response);
    server->dmClient(client, 366, name + " :End of /NAMES list.");
}

std::string Channel::getTopic() const {
    return topic;
}

bool Channel::isEmpty() const {
    return members.empty();
}

bool Channel::mode(char mode) const {
    std::map<char, bool>::const_iterator it = modes.find(mode);
    if (it != modes.end()) {
        return it->second;
    }
    throw std::runtime_error("Invalid mode character");
}


void Channel::updateMode(char mode, bool value) {
    std::map<char, bool>::iterator it = modes.find(mode);
    if (it != modes.end()) {
        it->second = value;
    } else {
        throw std::runtime_error("Invalid mode character");
    }
}


void Channel::setTopic(const std::string &new_topic) {
    topic = new_topic;
}