#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

void Services::ping(Client &client, std::vector<std::string> &params) {
    if (params.size() != 1) {
        server->dmClient(client, 409, "PING :No origin specified");
        return;
    }

    if (params[0] != "ircserv") {
        server->dmClient(client, 409, "PING :No origin specified");
        return;
    }

    server->dmClient(client, 0, "PONG :ircserv");
}
