#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

/*
* Handle the PASS command
*/
void Services::pass(Client &client, std::vector<std::string> &params)
{
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }
    
    if (params.empty())
    {
        server->dmClient(client, 461, "Not enough parameters");
        return;
    }

    std::string password = params[0];
    if (password.empty())
    {
        server->dmClient(client, 461, "Not enough parameters");
        return;
    }

    if (client.isAuthenticated()) {
        server->dmClient(client, 462, "You may not authenticate again");
        return;
    }

    if (server->isPasswordMatching(password))
    {
        client.setAuthenticated(true);
        server->dmClient(client, 001, "Welcome to the IRC server!");
        return ;
    }

    server->dmClient(client, 464, "Password incorrect, closing link");
    server->removeClient(client);
}
