#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"

void Services::prvmsg(Client &client, std::vector<std::string> &params)
{
    if (params.size() < 2)
    {
        server->dmClient(client, ERR_NEEDMOREPARAMS, client.getNick() + " PRIVMSG :Not enough parameters");
        return;
    }

    std::string target = params[0];
    std::string message = params[1];

    if (target.empty() || message.empty())
    {
        server->dmClient(client, ERR_NEEDMOREPARAMS, client.getNick() + " PRIVMSG :Not enough parameters");
        return;
    }

    if (message[0] != ':')
    {
        server->dmClient(client, ERR_NOTEXTTOSEND, ":No text to send");
        return;
    }

    message.erase(0, 1);
    for (size_t i = 2; i < params.size(); ++i)
    {
        message += " " + params[i];
    }

    // Check if the target is a channel
    if (target[0] == '#')
    {
        Channel *channel = server->getChannel(target);

        if (!channel)
        {
            server->dmClient(client, ERR_NOSUCHCHANNEL, client.getNick() + " " + target + " :No such channel");
            return;
        }

        if (!channel->isMember(client))
        {
            server->dmClient(client, ERR_NOTONCHANNEL, client.getNick() + " " + target + " :You're not on that channel");
            return;
        }

        // Broadcast the message to all members of the channel
        channel->broadcastToMembers(client, "PRIVMSG " + target + " :" + message);

        return;
    }
    else
    {
        // Target is a user
        Client *target_client = server->getClientByNick(target);
        if (!target_client)
        {
            server->dmClient(client, ERR_NOSUCHNICK, client.getNick() + " " + target + " :No such nick/channel");
            return;
        }

        // Send the private message to the target client
        // server->dmClient(target_client, 250, "PRIVMSG " + target + " :" + message);
        client.sendMessage(*target_client, "PRIVMSG " + target + (message.empty() ? "" : " :" + message));
        return;
    }
}