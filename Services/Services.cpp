#include "Services.hpp"
#include "../Server/Server.hpp"
#include "../Client/Client.hpp"


/*
* Set-up the services command map
*/
Services::Services(Server *srv) : server(srv)
{
    command_map["PASS"] = &Services::pass;
    command_map["NICK"] = &Services::nick;
    command_map["USER"] = &Services::user;
    command_map["QUIT"] = &Services::quit;
    command_map["JOIN"] = &Services::join;
    command_map["PART"] = &Services::part;
    command_map["NAMES"] = &Services::names;
    command_map["TOPIC"] = &Services::topic;
    command_map["MODE"] = &Services::mode;
    command_map["PRIVMSG"] = &Services::prvmsg;
    command_map["KICK"] = &Services::kick;
    command_map["INVITE"] = &Services::invite;
    command_map["PING"] = &Services::ping;
    command_map["CAP"] = &Services::cap;
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
        command != "PASS" && command != "QUIT" && command != "CAP"
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
        command != "CAP" &&
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
* Capture a command shot from a client
*/
std::string cmd_shot(int fd) {
    char buffer[512] = {0};
    int readed_bytes = read(fd, buffer, 512);
    if (readed_bytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return "";
        }
        // Anything else is an error
        throw std::runtime_error("Read error on fd " + glob::to_string(fd) + ": " + strerror(errno));
    } else if (readed_bytes == 0) {
        // Connection closed by the client
        throw std::runtime_error("Client disconnected on fd " + glob::to_string(fd));
    }
    std::string msg(buffer, readed_bytes);
    return msg;
}

/*
* Strip trailing non-printable characters from a command string
* and store them as the client's command terminators if not already set.
* Read Client::setCommandTerminators() for more info
*/
std::string stripTrailingTerminators(std::string &cmd, Client &client)
{
    size_t end = cmd.size();
    std::string terminators;

    while (end > 0 && (cmd[end - 1] == '\r' || cmd[end - 1] == '\n' || cmd[end - 1] == '\t')) {
        --end;
    }
    terminators = cmd.substr(end);

    if (terminators.empty())
        terminators = "\n";

    if (terminators[terminators.size() - 1] != '\n')
        terminators += '\n';

    cmd.erase(end);

    if (!client.hasSentFirstCommand())
    {
        client.setCommandTerminators(terminators);
        client.setSentFirstCommand();
    }

    return cmd;
}

/*
* Process a single command line from a client
* Returns true if processing should continue, false otherwise
*/
bool Services::processCommandLine(Client &client, int client_fd, std::string &command_line)
{
    std::string _cmd = stripTrailingTerminators(command_line, client);

    if (_cmd.empty())
        return true;

    std::istringstream iss(_cmd);
    std::string command;
    std::getline(iss, command, ' ');
    std::string params;
    std::getline(iss, params);

    if (!client.isConnected()) return false;

    server->log("Received command from fd " + glob::to_string(client_fd) + ": " + command_line);

    client.setLastActiveTime(time(NULL));
    client.setIsPinged(false);

    std::vector<std::string> param_list = split_params(params);
    std::map<std::string, 
        void (Services::*)(Client&, std::vector<std::string>&)>
        ::iterator it = command_map.find(command);

    if (it != command_map.end())
    {
        if (!isAuth(client, command)) return false;
        if (!isRegistered(client, command)) return false;

        try {
            (this->*(it->second))(client, param_list);
        } catch (const std::exception &e) {
            server->log("Exception while processing command " + command + ": " + e.what());
        }

        Client* existing_client = server->getClient(client_fd);
        if (existing_client == NULL) return false;
        
        existing_client->clearCommandStream();
        return true;
    }

    server->log("Unknown command: " + command);
    Client* existing_client = server->getClient(client_fd);
    if (existing_client == NULL) return false;
    server->dmClient(*existing_client, 421, command + " :Unknown command");
    existing_client->clearCommandStream();
    return false;
}

/*
* Handle incoming payload from a client
* This function accumulates data until a full command is received
* and then processes each command line individually.
*/
void Services::dealWithClient(int client_fd)
{
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    Client *client = server->getClient(client_fd);
    if (!client) {
        server->log("No client found for fd " + glob::to_string(client_fd));
        server->addToDeleteQueue(*client);
        return;
    }

    std::string payload;

    try {
        payload = cmd_shot(client_fd);
    } catch (const std::exception &e) {
        server->log("Error reading from client fd " + glob::to_string(client_fd) + ": " + e.what());
        server->addToDeleteQueue(*client);
        return;
    }

    if (payload.empty()) {
        return;
    }

    std::stringstream &cmdStream = client->getCommandStream();
    
    if (!payload.empty() && payload[payload.size() - 1] != '\n')
    {
        cmdStream << payload;

        if (cmdStream.str().length() > MAX_COMMAND_LENGTH) {
            server->dmClient(*client, 414, "Command too long");
            client->clearCommandStream();
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

    for (size_t i = 0; i < commands.size(); ++i) {
        if (processCommandLine(*client, client_fd, commands[i]) == false) {
            return ;
        }    
    }
}

/*
* Handle sleepy clients
* This function checks if a client has been idle for too long
* Two thresholds are used:
* - PING_INTERVAL: if exceeded, a PING message is sent to the client if the client didn't send any command since PING_INTERVAL
* - CLIENT_TIMEOUT: if exceeded, the client is disconnected due to inactivity
*/
void Services::dealWithSleepModeClient(int client_fd) {

    Client *client = server->getClient(client_fd);
    if (!client) {
        server->log("No client found for fd " + glob::to_string(client_fd));
        return;
    }

    size_t last_action = server->getDiffTime(client->getLastActiveTime());

    if (last_action >= CLIENT_TIMEOUT) {
        server->log("Client " + glob::to_string(client_fd) + " timed out");
        server->dmClient(*client,  CLIENT_TIMEOUT, "ERROR :PING timeout:" + glob::to_string(CLIENT_TIMEOUT) + " seconds");
        server->addToDeleteQueue(*client);
        return;
    }

    if (last_action >= PING_INTERVAL && !client->isPinged()) {
        server->log("Client " + glob::to_string(client_fd) + " notify PING");
        server->dmClient(*client,  PING_INTERVAL, "PING :ircserv");
        client->setIsPinged(true);
        return;
    }
}

