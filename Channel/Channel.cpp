#include "Channel.hpp"
#include "../Client/Client.hpp"

Channel::Channel() : name(""), _operator(NULL) {
    std::cout << "Default Channel constructor called" << std::endl;
}

Channel::Channel(const std::string &channel_name, Client &creator) {
    name = channel_name;
    _operator = &creator;
}

Channel::~Channel() {}

std::string Channel::getName() const {
    return name;
}
