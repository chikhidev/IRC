#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

/*
* Handle the NICK command
*/
void Services::nick(Client &client, std::string &params)
{
    client.setNick(params);
    std::string response = ":" + params + " NICK " + params;
    server->dmClient(client, 0, response);
}
