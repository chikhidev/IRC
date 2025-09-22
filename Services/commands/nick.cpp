#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

/*
* Handle the NICK command
*/
void Services::nick(Client &client, std::vector<std::string> &params)
{
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    if (params.size() > 1)
    {
        server->dmClient(client, 432, "NICK: Too many parameters");
        return;
    }

    if (params.empty())
    {
        server->dmClient(client, 431, "NICK: No nickname given");
        return;
    }

    if (params[0].length() > NICK_LIMIT)
    {
        server->dmClient(client, 432, "NICK: Erroneous nickname");
        return;
    }

    // Check if nickname is already in use
    Client *existing_client = server->getClientByNick(params[0]);
    if (existing_client && existing_client != &client)
    {
        server->dmClient(client, 433, "NICK: Nickname is already in use");
        return;
    }

    server->removeUniqueNick(client.getNick());
    client.setNick(params[0]);
    server->addUniqueNick(params[0], client);
    std::string response = client.getNick() + " NICK " + params[0];
    server->dmClient(client, 0, response);
}
