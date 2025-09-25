#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"


void Services::cap(Client &client, std::vector<std::string> &params) {
    (void)params;
    server->dmClient(client, 0, "CAP * LS :");
}

