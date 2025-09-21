#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"

/*
* Handle the PART command: <channel>
*/
void Services::part(Client &client, std::vector<std::string> &params) {
    if (params.size() != 1) {
        server->dmClient(client, 461, "PART :Bad parameters");
        return;
    }

    std::string channel_name = params[0];
    if (channel_name[0] != '#') {
        server->dmClient(client, 403, "PART :No such channel");
        return;
    }
    channel_name.erase(0, 1);

    try {
        if (!server->channelExists(channel_name)) {
            server->dmClient(client, 403, "PART :No such channel");
            return;
        }

        Channel &_channel = server->getChannel(channel_name);
        _channel.removeMember(client);
        _channel.broadcastToMembers(client, "PART :#" + channel_name);

        if (_channel.isEmpty()) {
            server->removeChannel(channel_name);
        }

    } catch (const std::exception &e) {
        server->dmClient(client, 403, "PART :" + std::string(e.what()));
    }
}
