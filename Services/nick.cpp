#include "Services.hpp"
#include "../Server/Server.hpp"

/*
* Handle the NICK command
*/
void Services::handleNick(int client_fd, std::string &params)
{
    server->setClientNickname(client_fd, params);
}
