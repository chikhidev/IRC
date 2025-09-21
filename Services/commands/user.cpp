#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

/*
* Handle the USER command: <username> <hostname (ignored)> <servername (ignored)> :<realname>
*/
void Services::user(Client& client, std::vector<std::string>& params) {
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    size_t params_size = params.size();

    if (params_size < 4) {
        server->dmClient(client, 461, "USER :Not enough parameters");
        return;
    }

    std::string username = params[0];
    std::string realname = params[3];

    for (size_t i = 4; i < params_size; i++) {
        realname += " " + params[i];
    }

    if (realname[0] == ':')
        realname.erase(0, 1);

    client.setUsername(username);
    client.setRealname(realname);
    client.setRegistered(true);
    server->dmClient(client, 001, "Welcome to the IRC network, " + client.getNick() + "!" + client.getUsername() + "@localhost");
}
