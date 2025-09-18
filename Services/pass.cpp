#include "Services.hpp"
#include "../Server/Server.hpp"

/*
* Handle the PASS command
*/
void Services::handlePass(Client &client, std::string &params)
{
    std::string password = params;
    if (server->isPasswordMatching(password))
    {
        if (client.isAuthenticated()) {
            server->dmClient(client, 462, "You may not authenticate again\r\n");
            return;
        }

        client.setAuthenticated(true);
        server->dmClient(client, 001, "Welcome to the IRC server!\r\n");
        return ;
    }

    server->dmClient(client, 464, "Password incorrect\r\n");
    server->removeClient(client);
}
