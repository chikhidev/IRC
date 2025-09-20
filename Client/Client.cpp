#include "Client.hpp"

Client::Client(): addr_len(sizeof(addr)), fd(-1), _isAuthenticated(false), _isRegistered(false), _connected(true), _sent_first_command(false) {
    memset(&addr, 0, sizeof(addr));
}

Client::Client(int socket_fd, sockaddr_in address, socklen_t length) {
    fd = socket_fd;
    memcpy(&addr, &address, sizeof(address));
    addr_len = length;
    _isRegistered = false;
    _isAuthenticated = false;
    _connected = true;
    _sent_first_command = false;
}

Client::~Client() {}

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