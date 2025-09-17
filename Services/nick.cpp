#include "Services.hpp"
#include "../Server/Server.hpp"

/*
* Handle the NICK command
*/
void Services::handleNick(int client_fd, std::string &params)
{
    server->clients[client_fd].setNickname(params);
    std::string response = ":" + params + " NICK " + params + "\r\n";
    server->dmClient(client_fd, response);
}
