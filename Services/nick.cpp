#include "Services.hpp"
#include "../Server/Server.hpp"

/*
* Handle the NICK command
*/
void Services::handleNick(Client &client, std::string &params)
{
    client.setNickname(params);
    std::string response = ":" + params + " NICK " + params + "\r\n";
    server->dmClient(client, response);
}
