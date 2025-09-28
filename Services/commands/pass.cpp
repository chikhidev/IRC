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
        server->dmClient(client, ERR_NEEDMOREPARAMS, "Not enough parameters");
        return;
    }

    std::string password = params[0];
    if (password.empty())
    {
        server->dmClient(client, ERR_NEEDMOREPARAMS, "Not enough parameters");
        return;
    }

    if (client.isAuthenticated()) {
        server->dmClient(client, ERR_ALREADYAUTH, "You may not authenticate again");
        return;
    }

    if (server->isPasswordMatching(password))
    {
        client.setAuthenticated(true);
        server->dmClient(client, RPL_WELCOME, "Welcome to the IRC server!");
        return ;
    }

    client.disconnect();

    server->dmClient(client, ERR_PASSWDMISMATCH, "Password incorrect, closing link");
    server->addToDeleteQueue(client);
}
