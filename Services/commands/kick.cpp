#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"


void Services::kick(Client &client, std::vector<std::string> &params)
{
    if (params.size() < 2) {
        server->dmClient(client, 461, "KICK :Not enough parameters");
        return;
    }

    std::string channel_name = params[0];
    std::string user_to_kick = params[1];
    std::string reason;

    if (params.size() > 2) {
        reason = params[2];

        if (reason[0] != ':') {
            server->dmClient(client, 461, "KICK :reason badly formatted");
            return;
        }

        for (size_t i = 3; i < params.size(); ++i) {
            reason += " " + params[i];
        }

    }

    if (channel_name[0] != '#') {
        server->dmClient(client, 403, channel_name + " :No such channel");
        return;
    }

    Channel *channel = server->getChannel(channel_name);
    if (!channel) {
        server->dmClient(client, 403, channel_name + " :No such channel");
        return;
    }

    if (!channel->isOperator(client)) {
        server->dmClient(client, 482, channel_name + " :You're not channel operator");
        return;
    }

    Client *target_client = server->getClientByNick(user_to_kick);
    if (!target_client) {
        server->dmClient(client, 401, user_to_kick + " :No such nick/channel");
        return;
    }

    if (!channel->isMember(*target_client)) {
        server->dmClient(client, 441, user_to_kick + " " + channel_name + " :They aren't on that channel");
        return;
    }

    if (target_client == &client) {
        server->dmClient(client, 482, channel_name + " :You cannot kick yourself");
        return;
    }

    try {
        channel->removeMember(*target_client);
    } catch (const std::exception &e) {
        server->dmClient(client, 403, channel_name + e.what());
        return;
    }

    std::string kick_msg = "KICK " + channel_name + " " + user_to_kick + " " + reason;
    channel->broadcastToMembers(client, kick_msg);
}
