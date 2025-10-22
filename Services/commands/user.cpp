#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

/*
 * Handle the USER command: <username> <hostname (ignored)> <servername (ignored)> :<realname>
 */
void Services::user(Client &client, std::vector<std::string> &params)
{
    if (!server)
    {
        throw std::runtime_error("Server reference is null");
    }

    size_t params_size = params.size();

    if (params_size < 4)
    {
        server->dmClient(client, ERR_NEEDMOREPARAMS, ":Not enough parameters");
        return;
    }

    std::string username = params[0];
    std::string realname = params[3];

    if (realname[0] != ':')
    {
        server->dmClient(client, ERR_NEEDMOREPARAMS, ":Not enough parameters");
        return;
    }

    realname.erase(0, 1);

    for (size_t i = 4; i < params_size; i++)
    {
        realname += " " + params[i];
    }

    client.setUsername(username);
    client.setRealname(realname);
    
    if (client.hasNick())
    {
        client.setRegistered(true);
        server->dmClient(client, RPL_WELCOME, ":Welcome to the IRC network, " + client.getNick() + "!" + client.getUsername() + "@localhost");
    }
}
