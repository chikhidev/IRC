#ifndef LIBS_HPP
#define LIBS_HPP

#include <iostream>
#include <string.h>
#include <sstream>
#include <unistd.h>
#include <stdexcept>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <map>
#include <queue>
#include <algorithm>
#include <fcntl.h>
#include <errno.h>

#define EPOLL_TIMEOUT 1000 // milliseconds
#define MAX_CONNECTIONS 512
#define MAX_COMMAND_LENGTH 1024
#define NICK_LIMIT 9
#define PING_INTERVAL 10
#define CLIENT_TIMEOUT 15

namespace str {
    std::string to_string(int);
}

#endif