#ifndef SERVICES_HPP
#define SERVICES_HPP

#include "../shared/libs.hpp"

/*
 * Services class for handling various IRC services ONLY!
*/

class Server;
class Client;

class Services {

    Server *server;
    std::map<std::string, void (Services::*)(Client&, std::string&)> command_map;

public:
    Services(Server *);
    ~Services();

    void handleCommand(int, std::string&);

    bool isAuth(Client&, std::string&);
    bool isRegistered(Client&, std::string&);

    void handlePass(Client&, std::string&);
    void handleNick(Client&, std::string&);
    void handleUser(Client&, std::string&);
    void handleQuit(Client&, std::string&);
};

#endif