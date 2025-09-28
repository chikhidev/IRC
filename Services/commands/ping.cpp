#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

void Services::ping(Client &client, std::vector<std::string> &params) {
    (void)params;
    server->dmClient(client, RPL_NEUTRAL, "PONG :ircserv");
}
