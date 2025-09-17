#include "Services.hpp"
#include "../Server/Server.hpp"

Services::Services(Server *srv) : server(srv)
{
    command_map["PASS"] = &Services::handlePass;
    command_map["NICK"] = &Services::handleNick;
}


Services::~Services() {}

/*
* Check if the client is authenticated before processing certain commands
*/
bool Services::isAuth(int client_fd, std::string &command)
{
    if (command != "PASS" && !server->clients[client_fd].isRegisteredClient())
    {
        server->dmClient(client_fd, "451 :You have not registered\r\n");
        return false;
    }
    return true;
}

/*
* Handle incoming commands from clients
*/
void Services::handleCommand(int client_fd, std::string &msg)
{
    msg.erase(msg.find_last_not_of("\r\n") + 1);

    if (msg.empty())
        return;

    std::istringstream iss(msg);
    std::string command;
    std::getline(iss, command, ' ');
    std::string params;
    std::getline(iss, params);
    
    std::map<std::string, void (Services::*)(int, std::string&)>::iterator it = command_map.find(command);
    if (it != command_map.end())
    {
        if (isAuth(client_fd, command))
            (this->*(it->second))(client_fd, params);
        return;
    }

    std::cout << "[SERVICE] Unknown command: " << command << std::endl;
    server->dmClient(client_fd, "421 " + command + " :Unknown command\r\n");
}

