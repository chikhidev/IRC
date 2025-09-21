#include "Services.hpp"
#include "../Server/Server.hpp"
#include "../Client/Client.hpp"

Services::Services(Server *srv) : server(srv)
{
    command_map["PASS"] = &Services::pass;
    command_map["NICK"] = &Services::nick;
    command_map["USER"] = &Services::user;
    command_map["QUIT"] = &Services::quit;
    command_map["JOIN"] = &Services::join;
    command_map["PART"] = &Services::part;
    command_map["NAMES"] = &Services::names;
}


Services::~Services() {}

/*
* Check if the client is authenticated before processing certain commands
*/
bool Services::isAuth(Client &client, std::string &command)
{
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    if (command != "PASS" && command != "QUIT" && !client.isAuthenticated())
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
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    if (command != "PASS" &&
        command != "NICK" &&
        command != "USER" &&
        command != "QUIT")
    {
        if (!client.isRegistered()) {
            server->dmClient(client, 451, "Not registered");
            return false;
        }
        if (!client.hasNick()) {
            server->dmClient(client, 431, "No nickname given");
            return false;
        }
    }

    return true;
}

/*
* Params split by space
*/
std::vector<std::string> split_params(const std::string &params) {
    std::vector<std::string> result;
    std::istringstream iss(params);
    std::string token;
    while (std::getline(iss, token, ' ')) {
        if (!token.empty()) {
            result.push_back(token);
        }
    }
    return result;
}

/*
* Handle incoming commands from clients
*/
void Services::handleCommand(int client_fd, std::string &msg)
{
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

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

    Client &client = server->getClient(client_fd);

    if (!client.isConnected()) return;

    std::vector<std::string> param_list = split_params(params);
    std::map<std::string, void (Services::*)(Client&, std::vector<std::string>&)>::iterator it = command_map.find(command);

    if (it != command_map.end())
    {
        if (!client.hasSentFirstCommand())
        {
            std::cout << "[SERVICE] Client " << client_fd << " sent their first command." << std::endl;
            client.setCommandTerminators(terminators);
            client.setSentFirstCommand();
        }

        std::cout << "[SERVICE] size of params: " << param_list.size() << std::endl;
        std::cout << "[SERVICE] infos about the client " << client_fd << " :" << std::endl;
        std::cout << "[SERVICE]  - Nick: " << client.getNick() << std::endl;
        std::cout << "[SERVICE]  - Username: " << client.getUsername() << std::endl;
        std::cout << "[SERVICE]  - Realname: " << client.getRealname() << std::endl;
        std::cout << "[SERVICE]  - Authenticated: " << (client.isAuthenticated() ? "Yes" : "No") << std::endl;
        std::cout << "[SERVICE]  - Registered: " << (client.isRegistered() ? "Yes" : "No") << std::endl;

        std::cout << "[SERVICE] Handling command: [" << command << "] with params: [";
        for (const auto &param : param_list) {
            std::cout << param << " ";
        }
        std::cout << "]" << std::endl;
        std::cout << std::endl;

        if (!isAuth(client, command)) return;

        if (!isRegistered(client, command)) return;

        (this->*(it->second))(client, param_list);
        return;
    }

    std::cout << "[SERVICE] Unknown command: " << command << std::endl;
    server->dmClient(client, 421, command + " :Unknown command");
}

