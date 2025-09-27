#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

void Services::whois(Client &client, std::vector<std::string> &params)
{
    (void)(params); // to avoid unused parameter warning
    server->dmClient(client, 311, client.getNick() + " localhost * :" + client.getRealname());
}
