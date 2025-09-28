#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"

/*
* handle invite command
* ONLY operators can invite
* checks the client if already exists in the invited list of the channel
* if not, adds the client to the invited list
* send message to the inviting and the invited client
* (to prevent invite spam, we are gonna save the invited clients in a list only when channel is +i)
*/
void Services::invite(Client &client, std::vector<std::string>& params)
{
    if (params.size() < 2)
    {
        server->dmClient(client, ERR_NEEDMOREPARAMS, "INVITE :Not enough parameters");
        return;
    }

    std::string targetNickname = params[0];
    std::string channelName = params[1];

    Client *targetClient = server->getClientByNick(targetNickname);
    if (!targetClient)
    {
        server->dmClient(client, ERR_NOSUCHNICK, targetNickname + " :No such nick/channel");
        return;
    }

    Channel *channel = server->getChannel(channelName);
    if (!channel)
    {
        server->dmClient(client, ERR_NOSUCHCHANNEL, "INVITE :No such channel");
        return;
    }

    if (!channel->isOperator(client))
    {
        server->dmClient(client, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
        return;
    }

    if (channel->isMember(*targetClient))
    {
        server->dmClient(client, ERR_USERONCHANNEL, targetNickname + " " + channelName + " :is already on channel");
        return;
    }

    bool is_target_invited = channel->isInvited(*targetClient);
    if (is_target_invited)
    {
        server->dmClient(client, ERR_INVITEONLYCHAN, targetNickname + " " + channelName + " :is already invited");
        return;
    }

    if (channel->mode('i'))
    {
        channel->addInvited(*targetClient);
    }

    server->dmClient(client, RPL_INVITING, targetNickname + " " + channelName);
    client.sendMessage(*targetClient, "INVITE " + targetNickname + " " + channelName);
}
