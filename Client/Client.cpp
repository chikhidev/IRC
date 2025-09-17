#include "Client.hpp"

Client::Client(): addr_len(sizeof(addr)), fd(-1), isRegistered(false) {
    memset(&addr, 0, sizeof(addr));
}

Client::Client(int socket_fd, sockaddr_in address, socklen_t length) {
    fd = socket_fd;
    memcpy(&addr, &address, sizeof(address));
    addr_len = length;
    isRegistered = false;
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

bool Client::isRegisteredClient() const {
    return isRegistered;
}

void Client::setRegistered(bool status) {
    isRegistered = status;
}

void Client::setNickname(const std::string &nick) {
    nickname = nick;
}