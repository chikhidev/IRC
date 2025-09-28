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
        server->dmClient(client, ERR_NEEDMOREPARAMS, "NAMES :Not enough parameters");
        return;
    }

    std::string channel_name = params[0];

    if (channel_name[0] != '#') {
        server->dmClient(client, ERR_NOSUCHCHANNEL, channel_name + " :No such channel");
        server->dmClient(client, RPL_ENDNAMES, channel_name + " :End of /NAMES list");
        return;
    }

    Channel *channel = server->getChannel(channel_name);
    if (!channel) {
        server->dmClient(client, ERR_NOSUCHCHANNEL, channel_name + " :No such channel");
        server->dmClient(client, RPL_ENDNAMES, channel_name + " :End of /NAMES list");
        return;
    }

    if (!channel->isMember(client)) {
        server->dmClient(client, ERR_NOTONCHANNEL, channel_name + " :You're not on that channel");
        server->dmClient(client, RPL_ENDNAMES, channel_name + " :End of /NAMES list");
        return;
    }

    channel->listMembers(client);
}
/*--------------------------------------------------------------*/