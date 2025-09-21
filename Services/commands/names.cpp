#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"

/*
* Handle listing names of users in a channel
*/
void Services::names(Client &client, std::vector<std::string> &params)
{
    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    if (params.empty()) {
        server->dmClient(client, 461, "NAMES :Not enough parameters");
        return;
    }

    std::string channel_name = params[0];

    if (channel_name[0] != '#') {
        server->dmClient(client, 403, channel_name + " :No such channel");
        server->dmClient(client, 366, channel_name + " :End of /NAMES list");
        return;
    }

    channel_name.erase(0, 1);

    if (!server->channelExists(channel_name)) {
        server->dmClient(client, 403, channel_name + " :No such channel");
        server->dmClient(client, 366, channel_name + " :End of /NAMES list");
        return;
    }

    Channel &channel = server->getChannel(channel_name);

    if (!channel.isMember(client)) {
        server->dmClient(client, 442, channel_name + " :You're not on that channel");
        server->dmClient(client, 366, channel_name + " :End of /NAMES list");
        return;
    }

    channel.listMembers(client);
}
/*--------------------------------------------------------------*/