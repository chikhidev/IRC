#ifndef SERVICES_HPP
#define SERVICES_HPP

#include "../shared/libs.hpp"

/*
 * Services class for handling various IRC services ONLY!
*/

class Server;
class Client;
class Channel;

class Services {

    Server *server;
    std::map<std::string, void (Services::*)(Client&, std::vector<std::string>&)> command_map;

public:
    Services(Server *);
    ~Services();

    void handleCommand(int);
    void processCommandLine(Client&, int, std::string&);

    bool isAuth(Client&, std::string&);
    bool isRegistered(Client&, std::string&);

    void pass(Client&, std::vector<std::string>&);
    void nick(Client&, std::vector<std::string>&);
    void user(Client&, std::vector<std::string>&);
    void quit(Client&, std::vector<std::string>&);
    void join(Client&, std::vector<std::string>&);
    void part(Client&, std::vector<std::string>&);
    void names(Client&, std::vector<std::string>&);
    void topic(Client&, std::vector<std::string>&);
    void prvmsg(Client&, std::vector<std::string>&);

    void mode(Client&, std::vector<std::string>&);
        void handlePass(Channel&, Client&, std::vector<std::string>&);
        void handleMembersLimit(Channel&, Client&, std::vector<std::string>&);
        void handleOperator(Channel&, Client&, std::vector<std::string>&);
};

#endif