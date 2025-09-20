#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

/*
* Handle the NICK command
*/
void Services::nick(Client &client, std::vector<std::string> &params)
{
    if (params.empty())
    {
        server->dmClient(client, 431, "No nickname given");
        return;
    }

    if (params.size() > 1)
    {
        server->dmClient(client, 432, "Erroneous nickname");
        return;
    }

    if (params[0].length() > NICK_LIMIT)
    {
        server->dmClient(client, 432, "Erroneous nickname");
        return;
    }

    client.setNick(params[0]);
    std::string response = ":" + params[0] + " NICK " + params[0];
    server->dmClient(client, 0, response);
}
