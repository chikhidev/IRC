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
#include <signal.h>

#define MAX_CONNECTIONS 512
#define MAX_COMMAND_LENGTH 1024
#define NICK_LIMIT 9
#define CHANNEL_NAME_LIMIT 50
#define PING_INTERVAL 100
#define CLIENT_TIMEOUT 150
#define EPOLL_TIMEOUT 50000

namespace glob {
    std::string to_string(int);
    bool *server_running();
    void stop_running(int);
}

#endif