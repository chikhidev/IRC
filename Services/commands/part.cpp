#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"

/*
* Handle the PART command: <channel>
*/
void Services::part(Client &client, std::vector<std::string> &params) {
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    if (params.size() < 1 || params.size() > 2) {
        server->dmClient(client, 461, "PART :Bad parameters");
        return;
    }

    std::string channel_name = params[0];
    if (channel_name[0] != '#') {
        server->dmClient(client, 403, "PART :No such channel");
        return;
    }

    std::string reason;
    if (params.size() == 2) {
        reason = params[1];
        if (reason.empty() || reason[0] != ':') {
            server->dmClient(client, 461, "PART :Bad parameters");
            return;
        }
    }



    try {
        Channel *_channel = server->getChannel(channel_name);
        if (!_channel) {
            server->dmClient(client, 403, "PART :No such channel");
            return;
        }


        if (!_channel->isMember(client)) {
            server->dmClient(client, 442, "PART :You're not on that channel");
            return;
        }

        _channel->broadcastToMembers(client, "PART :" + channel_name + (reason.empty() ? "" : " " + reason));

        if (_channel->isOperator(client) && _channel->getOperatorsCount() == 1) {
            _channel->broadcastToMembers(client, "NOTICE " + _channel->getName() + " :The channel operator has left the channel, the channel will be removed.");
            server->removeChannel(_channel->getName());
            return;
        }

        _channel->removeMember(client);

        if (_channel->isEmpty()) {
            server->log("Channel " + channel_name + " is empty. Removing it from server.");
            server->removeChannel(channel_name);
        }

    } catch (const std::exception &e) {
        server->dmClient(client, 403, "PART :" + std::string(e.what()));
    }
}
