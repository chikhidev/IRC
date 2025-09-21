#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

/*
* Handle the QUIT command: [<message>]
*/
void Services::quit(Client &client, std::vector<std::string> &params) {
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    std::string quit_message;
    if (!params.empty()) {
        quit_message = params[0];
    }

    server->dmClient(client, 221, "Goodbye! " + quit_message);
    std::cout << "[SERVICE] Client with fd " << client.getFd() << " has quit." << std::endl;

    server->removeClient(client);
}