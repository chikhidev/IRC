#include "Client.hpp"

Client::Client(): addr_len(sizeof(addr)), fd(-1), _isAuthenticated(false), _isRegistered(false) {
    memset(&addr, 0, sizeof(addr));
}

Client::Client(int socket_fd, sockaddr_in address, socklen_t length) {
    fd = socket_fd;
    memcpy(&addr, &address, sizeof(address));
    addr_len = length;
    _isRegistered = false;
    _isAuthenticated = false;
}

Client::~Client() {
    // Don't close the fd here since the Server class manages the socket
    // The socket will be closed by the Server when removing the client
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

void Client::setNickname(const std::string &nick) {
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

bool Client::hasNick() const {
    return !nickname.empty();
}