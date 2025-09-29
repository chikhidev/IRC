#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

/*
 * Handle the QUIT command: [<message>]
 */
void Services::quit(Client &client, std::vector<std::string> &params)
{
    if (!server)
    {
        throw std::runtime_error("Server reference is null");
    }

    std::string quit_message;
    if (!params.empty())
    {
        quit_message = params[0];
    }

    // Send ERROR message before closing connection
    std::string error_msg = "ERROR :Closing Link: " + client.getNick() + "@localhost (Quit: " + quit_message + ")\r\n";
    server->sendMessage(client, error_msg);
    server->log("Client with fd " + glob::to_string(client.getFd()) + " has quit. " + (quit_message.empty() ? "" : "Message: " + quit_message));

    client.disconnect();

    server->addToDeleteQueue(client);
}