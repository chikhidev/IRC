#include "Services.hpp"
#include "../Server/Server.hpp"
#include "../Client/Client.hpp"

Services::Services(Server *srv) : server(srv)
{
    command_map["PASS"] = &Services::handlePass;
    command_map["NICK"] = &Services::handleNick;
    command_map["USER"] = &Services::handleUser;
    command_map["QUIT"] = &Services::handleQuit;

    command_map["pass"] = &Services::handlePass;
    command_map["nick"] = &Services::handleNick;
    command_map["user"] = &Services::handleUser;
}


Services::~Services() {}

/*
* Check if the client is authenticated before processing certain commands
*/
bool Services::isAuth(Client &client, std::string &command)
{
    if (command != "PASS" && !client.isAuthenticated())
    {
        server->dmClient(client, 451, "You must be authenticated to use this command");
        return false;
    }
    return true;
}

/*
* Check if the client is registered before processing certain commands
*/
bool Services::isRegistered(Client &client, std::string &command)
{
    if (command != "PASS" &&
        command != "NICK" &&
        command != "USER" &&
        !client.isRegistered())
    {
        server->dmClient(client, 451, "You must be registered to use this command");
        return false;
    }
    return true;
}

/*
* Handle incoming commands from clients
*/
void Services::handleCommand(int client_fd, std::string &msg)
{
    std::string terminators;
    for (int i = msg.size() - 1; i >= 0; --i)
    {
        if ((msg[i] >= 32 && msg[i] <= 126))
            break;
        terminators = msg[i] + terminators;
        msg.erase(i, 1);
    }

    if (msg.empty())
        return;

    std::istringstream iss(msg);
    std::string command;
    std::getline(iss, command, ' ');
    std::string params;
    std::getline(iss, params);
    
    Client &client = server->clients[client_fd];
    std::map<std::string, void (Services::*)(Client&, std::string&)>::iterator it = command_map.find(command);

    if (it != command_map.end())
    {
        if (!client.hasSentFirstCommand())
        {
            std::cout << "[SERVICE] Client " << client_fd << " sent their first command." << std::endl;
            client.setCommandTerminators(terminators);
            client.setSentFirstCommand();
        }

        std::cout << "[SERVICE] Handling command: [" << command << "] with params: " << params << std::endl;

        if (!client.isConnected()) return;

        if (!isAuth(client, command)) return;

        if (!isRegistered(client, command)) return;

        (this->*(it->second))(client, params);
        return;
    }

    std::cout << "[SERVICE] Unknown command: " << command << std::endl;
    server->dmClient(client, 421, command + " :Unknown command");
}

