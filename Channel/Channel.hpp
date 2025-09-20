#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "../shared/libs.hpp"

class Client;

class Channel {
    std::string name;
    std::map<int, Client*> members;
    Client* _operator;

public:
    Channel();
    Channel(const std::string &, Client &);
    ~Channel();

    std::string getName() const;

};

#endif