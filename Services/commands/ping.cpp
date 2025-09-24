#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

void Services::ping(Client &client, std::vector<std::string> &params) {
    if (params.size() < 1) {
        server->dmClient(client, 409, "PING :No origin specified");
        return;
    }

    client.setLastPingTime(time(NULL));
    server->dmClient(client,  0, ":" + server->getServerName());
}
