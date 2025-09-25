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

    void dealWithClient(int);
    bool processCommandLine(Client&, int, std::string&);
    void dealWithSleepModeClient(int);

    bool isAuth(Client&, std::string&);
    bool isRegistered(Client&, std::string&);


    /*
        * IRC COMMANDS
        * As you may notice, all commands take a Client reference and a vector of strings as parameters
        * The Client reference represents the client who sent the command
        * The vector of strings contains the command and its parameters, split by spaces
        * For example, if a client sends "NICK myNick", the vector will contain ["NICK", "myNick"]
        * This design allows for easy extension and handling of new commands
    */

    void pass(Client&, std::vector<std::string>&);
    void nick(Client&, std::vector<std::string>&);
    void user(Client&, std::vector<std::string>&);
    void quit(Client&, std::vector<std::string>&);
    void join(Client&, std::vector<std::string>&);
    void part(Client&, std::vector<std::string>&);
    void names(Client&, std::vector<std::string>&);
    void topic(Client&, std::vector<std::string>&);
    void prvmsg(Client&, std::vector<std::string>&);
    void kick(Client&, std::vector<std::string>&);
    void invite(Client&, std::vector<std::string>&);
    void ping(Client&, std::vector<std::string>&);
    void cap(Client&, std::vector<std::string>&);

    void mode(Client&, std::vector<std::string>&); // MODE command handler, with sub-handlers
        bool handlePass(Channel&, Client&, bool, std::string*);
        bool handleMembersLimit(Channel&, Client&, bool, std::string*);
        bool handleOperator(Channel&, Client&, bool, std::string*);
};

#endif