#include "Channel.hpp"
#include "../Client/Client.hpp"
#include "../Server/Server.hpp"

Channel::Channel(const std::string &channel_name, Client &creator, Server *srv) : server(srv) { 
    name = channel_name;
    operators[creator.getNick()] = &creator;
    creator.addToJoinedChannels(name);
    initModes();
    user_limit = 10;
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
    members[client.getFd()] = &client;
    client.addToJoinedChannels(name);
}


void Channel::removeMember(Client &client) {
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    bool is_operator = false;
    bool found = false;

    // Check and remove from members map
    std::map<int, Client*>::iterator members_it = members.find(client.getFd());
    if (members_it != members.end()) {
        members.erase(members_it);
        found = true;
    }
    
    // Check and remove from operators map
    std::map<std::string, Client*>::iterator operators_it = operators.find(client.getNick());
    if (operators_it != operators.end() && operators_it->second == &client) {
        operators.erase(operators_it);
        found = true;
        is_operator = true;
    }
    
    if (found) {
        if (is_operator && operators.size() == 0) {
            broadcastToMembers(client, "NOTICE " + name + " :The channel operator has left the channel, the channel will be removed.");
            client.removeFromJoinedChannels(name);
            server->removeChannel(name);
            return;
        }
        client.removeFromJoinedChannels(name);
    }
}

// void Channel::removeMember(Client &client) {

//     if (!server) {
//         throw std::runtime_error("Server reference is null");
//     }

//     std::map<int, Client*>::iterator it = members.find(client.getFd());
//     if (it != members.end()) {
//         members.erase(it);
//         client.removeFromJoinedChannels(name);
//     } else {
//         throw std::runtime_error("Client is not a member of the channel");
//     }
// }

/*
* Check if a client is a member of the channel
*/
bool Channel::isMember(const Client &client) const {
    return
        members.find(client.getFd()) != members.end() ||
        operators.find(client.getNick()) != operators.end();
}

/*
* Check if a client is an operator of the channel
*/
bool Channel::isOperator(const Client &client) const {
    std::map<const std::string, Client*>::const_iterator it = operators.find(client.getNick());
    if (it != operators.end()) {
        server->log("Client " + client.getNick() + " is an operator of channel " + name);
    }
    if (it->second != &client) {
        server->log("Client " + client.getNick() + " operator instance does not match");
    }
    return it != operators.end() && it->second == &client;
}

/*
* Add a client as an operator of the channel
*/
void Channel::addOperator(Client &client) {
    if (!isMember(client)) {
        throw std::runtime_error("Client is not a member of the channel");
    }

    std::map<std::string, Client*>::iterator it = operators.find(client.getNick());
    if (it != operators.end()) {
        throw std::runtime_error("Client is already an operator");
    }

    operators[client.getNick()] = &client;
}

/*
* Remove a client as an operator of the channel
*/
void Channel::removeOperator(Client &client) {
    std::map<std::string, Client*>::iterator it = operators.find(client.getNick());
    if (it == operators.end()) {
        throw std::runtime_error("Client is not an operator");
    }

    operators.erase(it);
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
        // throw std::runtime_error("Not a member of the channel");
        server->dmClient(sender, 442, name + " :You're not on that channel");
        return;
    }

    //send to operators
    for (std::map<const std::string, Client*>::iterator it = operators.begin(); it != operators.end(); ++it) {
        if (it->second->getFd() != sender.getFd()) {
            sender.sendMessage(*(it->second), message);
            return;
        }
    }

    // send message to all members except sender
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

    server->log("Members of channel " + name + ":");

    std::string response = name + " :";

    for (std::map<std::string, Client*>::const_iterator it = operators.begin(); it != operators.end(); ++it) {
        response += "@" + it->second->getNick() + " ";
    }

    for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
        response += it->second->getNick() + " ";
    }

    server->dmClient(client, 353, response);
    server->dmClient(client, 366, name + " :End of /NAMES list.");
}

std::string Channel::getTopic() const {
    return topic;
}

bool Channel::isEmpty() const {
    return members.empty() && operators.empty();
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

void Channel::updatePassword(const std::string &new_password) {
    password = new_password;
}

void Channel::updateUserLimit(size_t limit) {
    user_limit = limit;
}


bool Channel::isFull() const {
    return modes.at('l') &&
        (members.size() + operators.size()) >= user_limit;
}



bool Channel::isInvited(Client &client) const {
    return invited.find(client.getNick()) != invited.end();
}

void Channel::addInvited(Client &client) {
    invited[client.getNick()] = &client;
}

void Channel::removeInvited(Client &client) {
    invited.erase(client.getNick());
}


bool Channel::isMatchingPassword(const std::string &key) const {
    if (!modes.at('k')) return true;
    return !password.empty() && password == key;
}

size_t Channel::getOperatorsCount() const {
    return operators.size();
}

