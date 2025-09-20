#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"

/*
* Handle the JOIN command: <channel>
*/
void Services::join(Client &client, std::vector<std::string> &params) {
    if (params.size() != 1) {
        server->dmClient(client, 461, "JOIN :Bad parameters");
        return;
    }

    std::string channel_name = params[0];
    if (channel_name[0] == '#') {
        channel_name.erase(0, 1); // Remove the '#' character
    }

    try {
        
        if (!server->channelExists(channel_name)) {
            server->createChannel(channel_name, client);
        } else {
            server->addClientToChannel(channel_name, client);
        }

        Channel &_channel = server->getChannel(channel_name);

        _channel.broadcastToMembers(client, "JOIN :#" + channel_name);

        std::string topic = _channel.getTopic();
        if (topic.empty()) {
            server->dmClient(client, 331, "#" + channel_name + " :No topic is set");
        } else {
            server->dmClient(client, 332, "#" + channel_name + " :" + topic);
        }
        
    } catch (const std::exception &e) {
        server->dmClient(client, 403, "JOIN :" + std::string(e.what()));
    }
}
