#include "Services.hpp"
#include "../Server/Server.hpp"

/*
* Handle the PASS command
*/
void Services::handlePass(int client_fd, std::string &params)
{
    std::string password = params;
    if (server->isPasswordMatching(password))
    {
        server->registerClient(client_fd);
        server->dmClient(client_fd, "001 :Welcome to the IRC server!\r\n");
        return ;
    }

    server->dmClient(client_fd, "464 :Password incorrect\r\n");
    server->removeClient(client_fd);
}
