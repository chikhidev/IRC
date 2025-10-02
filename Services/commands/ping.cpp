#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

void Services::ping(Client &client, std::vector<std::string> &params)
{
    (void)params;
    std::string pong_msg = ":ircserv PONG ircserv :ircserv\r\n";
    server->sendMessage(client, pong_msg);
}
