#include "../Server/Server.hpp"
#include "../Channel/Channel.hpp"
#include "Client.hpp"

Client::Client(): addr_len(sizeof(addr)), fd(-1), _isAuthenticated(false), _isRegistered(false), _connected(true), _sent_first_command(false), server(NULL) {
    memset(&addr, 0, sizeof(addr));
}

Client::Client(int socket_fd, sockaddr_in address, socklen_t length, Server* srv) {
    fd = socket_fd;
    memcpy(&addr, &address, sizeof(address));
    addr_len = length;
    _isRegistered = false;
    _isAuthenticated = false;
    _connected = true;
    _sent_first_command = false;
    server = srv;
}

Client::~Client() {
    if (!nickname.empty() && server) {
        server->removeUniqueNick(nickname);
    }
}

void Client::setAddr(sockaddr_in address) {
    memcpy(&addr, &address, sizeof(address));
}

void Client::setAddrLen(socklen_t length) {
    addr_len = length;
}

int Client::getFd() const {
    return fd;
}

sockaddr_in Client::getAddr() {
    return addr;
}

socklen_t Client::getAddrLen() {
    return addr_len;
}

void Client::setFd(int socket_fd) {
    fd = socket_fd;
}

bool Client::isRegistered() const {
    return _isRegistered;
}

void Client::setRegistered(bool status) {
    _isRegistered = status;
}

void Client::setAuthenticated(bool status) {
    _isAuthenticated = status;
}

void Client::setNick(const std::string &nick) {
    nickname = nick;
}

void Client::setUsername(const std::string &user) {
    username = user;
}

void Client::setRealname(const std::string &real) {
    realname = real;
}

bool Client::isAuthenticated() const {
    return _isAuthenticated;
}

std::string Client::getNick() const {
    return nickname;
}

std::string Client::getUsername() const {
    return username;
}

std::string Client::getRealname() const {
    return realname;
}

bool Client::hasNick() const {
    return !nickname.empty();
}

void Client::disconnect() {
    _connected = false;
}

bool Client::isConnected() const {
    return _connected;
}

void Client::setCommandTerminators(const std::string &terminators) {
    command_terminators = terminators;
}

std::string Client::getCommandTerminators() const {
    if (command_terminators.empty()) {
        return "\r\n";
    }
    return command_terminators;
}

bool Client::hasSentFirstCommand() const {
    return _sent_first_command;
}

void Client::setSentFirstCommand() {
    _sent_first_command = true;
}

/*
* send a message to another client, perspective of this client
*/
void Client::sendMessage(Client& receiver, const std::string& message) {
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    std::string formatted_client = nickname + "!" + username + "@localhost";

    std::string formatted_message = formatted_client + " " + message + getCommandTerminators();
    server->sendMessage(receiver, formatted_message);
}


void Client::addToJoinedChannels(const std::string& channel_name) {
    joined_channels.push_back(channel_name);
}

void Client::removeFromJoinedChannels(const std::string& channel_name) {
    for (std::vector<std::string>::iterator it = joined_channels.begin(); it != joined_channels.end(); ++it) {
        if (*it == channel_name) {
            joined_channels.erase(it);
            return;
        }
    }
}

void Client::quitAllChannels() {
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    for (int i = 0; i < joined_channels.size(); i++) {
        if (server->channelExists(joined_channels[i])) {
            std::cout << "[CLIENT] Client " << fd << " quitting channel " << joined_channels[i] << std::endl;
            Channel& channel = server->getChannel(joined_channels[i]);
            if (channel.isMember(*this)) {
                std::string quit_message = "QUIT :Client disconnected";
                channel.broadcastToMembers(*this, quit_message);
                channel.removeMember(*this);
            }
        }
    }

    joined_channels.clear();
}

bool Client::operator==(const Client &other) const {
    return fd == other.fd;
}

bool Client::operator!=(const Client &other) const {
    return !(fd == other.fd);
}
