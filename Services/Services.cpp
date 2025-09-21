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

    if (
        command != "PASS" && command != "QUIT"
        && !client.isAuthenticated())
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

std::string cmd_shot(int fd) {
    char buffer[512] = {0};
    int readed_bytes = read(fd, buffer, 512);
    std::string msg(buffer, readed_bytes);
    return msg;
}

/*
* Strip trailing non-printable characters from a command string
*/
std::string stripTrailingTerminators(std::string &cmd, Client &client)
{
    size_t end = cmd.size();
    std::string terminators;

    while (end > 0 && !isprint(static_cast<unsigned char>(cmd[end - 1])))
    {
        --end;
    }
    terminators = cmd.substr(end);
    cmd.erase(end);

    if (!client.hasSentFirstCommand())
    {
        client.setCommandTerminators(terminators);
        client.setSentFirstCommand();
    }

    return cmd;
}

void debug_client(Client &client, int client_fd, const std::string &command, const std::vector<std::string> &param_list) {
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
}

/*
* Process a single command line from a client
*/
void Services::processCommandLine(Client &client, int client_fd, std::string &command_line)
{
    std::string _cmd = stripTrailingTerminators(command_line, client);

    if (_cmd.empty())
        return;

    std::istringstream iss(_cmd);
    std::string command;
    std::getline(iss, command, ' ');
    std::string params;
    std::getline(iss, params);

    if (!client.isConnected()) return;

    std::cout << "[SERVICE] Received command from fd " << client_fd << ": " << command_line << std::endl;

    std::vector<std::string> param_list = split_params(params);
    std::map<std::string, 
        void (Services::*)(Client&, std::vector<std::string>&)>
        ::iterator it = command_map.find(command);

    if (it != command_map.end())
    {
        if (!isAuth(client, command)) return;
        if (!isRegistered(client, command)) return;
        debug_client(client, client_fd, command, param_list);
        (this->*(it->second))(client, param_list);
        client.clearCommandStream();
        return;
    }

    std::cout << "[SERVICE] Unknown command: " << command << std::endl;
    server->dmClient(client, 421, command + " :Unknown command");
    client.clearCommandStream();
}

/*
* Handle incoming commands from clients
*/
void Services::handleCommand(int client_fd)
{
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    Client &client = server->getClient(client_fd);
    std::string payload = cmd_shot(client_fd);
    std::stringstream &cmdStream = client.getCommandStream();
    
    if (!payload.empty() && payload.back() != '\n')
    {
        cmdStream << payload;

        if (cmdStream.str().length() > MAX_COMMAND_LENGTH) {
            server->dmClient(client, 414, "Command too long");
            client.clearCommandStream();
        }

        return; // here we stop to give other clients their chances to
    }

    payload = cmdStream.str() + payload;

    // here we check if multilined payload
    std::vector<std::string> commands;
    std::istringstream iss(payload);
    std::string line;
    while (std::getline(iss, line))
    {
        commands.push_back(line);
    }

    for (size_t i = 0; i < commands.size(); ++i)
    {
        processCommandLine(client, client_fd, commands[i]);
    }
}

