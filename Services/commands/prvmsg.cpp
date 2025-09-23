#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"

void Services::prvmsg(Client &client, std::vector<std::string> &params) {
    if (params.size() < 2) {
        server->dmClient(client, 461, "PRIVMSG :Not enough parameters");
        return;
    }

    std::string target = params[0];
    std::string message = params[1];

    for (size_t i = 2; i < params.size(); ++i) {
        message += " " + params[i];
    }

    if (message[0] != ':') {
        server->dmClient(client, 412, "PRIVMSG :Badly formed message");
        return;
    }

    if (target.empty() || message.empty()) {
        server->dmClient(client, 461, "PRIVMSG :Not enough parameters");
        return;
    }

    // Check if the target is a channel
    if (target[0] == '#') {
        Channel *channel = server->getChannel(target);

        if (!channel) {
            server->dmClient(client, 403, target + " :No such channel");
            return;
        }

        if (!channel->isMember(client)) {
            server->dmClient(client, 442, target + " :You're not on that channel");
            return;
        }

        // Broadcast the message to all members of the channel
        channel->broadcastToMembers(client, "PRIVMSG " + target + " " + message);
        return;
    } else {
        // Target is a user
        Client *target_client = server->getClientByNick(target);
        if (!target_client) {
            server->dmClient(client, 401, target + " :No such nick/channel");
            return;
        }

        // Send the private message to the target client
        // server->dmClient(target_client, 250, "PRIVMSG " + target + " :" + message);
        client.sendMessage(*target_client, "PRIVMSG " + target + (message.empty() ? "" : " " + message));
        return;
    }

    // If the target is neither a channel nor a user
    server->dmClient(client, 401, target + " :No such nick/channel");
}