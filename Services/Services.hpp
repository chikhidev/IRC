#ifndef SERVICES_HPP
#define SERVICES_HPP

#include "../shared/libs.hpp"

/*
 * Services class for handling various IRC services ONLY!
*/

class Server;

class Services {

    Server *server;
    std::map<std::string, void (Services::*)(int, std::string&)> command_map;
public:
    Services(Server *);
    ~Services();

    void handleCommand(int, std::string&);

    bool isAuth(int, std::string&);

    void handlePass(int, std::string&);
    void handleNick(int, std::string&);
};

#endif