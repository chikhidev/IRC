#include "Client.hpp"

Client::Client(): addr_len(sizeof(addr)), fd(-1), isRegistered(false), isOperator(false) {
    memset(&addr, 0, sizeof(addr));
}

Client::~Client() throw() {
    if (fd != -1) {
        close(fd);
    }
}

int Client::getFd() const {
    return fd;
}

sockaddr_in* Client::getAddr() {
    return (sockaddr_in*)&addr;
}

socklen_t* Client::getAddrLen() {
    return &addr_len;
}

void Client::setFd(int socket_fd) {
    fd = socket_fd;
}

