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
    server->log("Client with fd " + std::to_string(client.getFd()) + " has quit. " + (quit_message.empty() ? "" : "Message: " + quit_message));

    server->removeClient(client);
}