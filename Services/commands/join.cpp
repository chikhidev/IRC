#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"

/*
 * Handle the JOIN command: <channel>
 */
void Services::join(Client &client, std::vector<std::string> &params)
{

    if (!server)
    {
        throw std::runtime_error("Server reference is null");
    }

    if (params.size() < 1)
    {
        server->dmClient(client, ERR_NEEDMOREPARAMS, "JOIN :Bad parameters");
        return;
    }

    std::string channel_name = params[0];

    if (channel_name.length() > CHANNEL_NAME_LIMIT)
    {
        server->dmClient(client, ERR_INPUTTOOLONG, "JOIN :Too long channel name");
        return;
    }

    if (channel_name == "0")
    {
        client.quitAllChannels();
        server->dmClient(client, RPL_ENDOFNAMES, "* :End of /NAMES list.");
        return;
    }

    if (channel_name[0] != '#')
    {
        server->dmClient(client, ERR_NOSUCHCHANNEL, "JOIN :No such channel");
        return;
    }

    try
    {
        Channel *existing_channel = server->getChannel(channel_name);

        if (!existing_channel)
        {
            server->createChannel(channel_name, client);
            Channel *new_channel = server->getChannel(channel_name);

            // Send JOIN confirmation to the client themselves first
            server->sendMessage(client, ":" + client.getNick() + "!" + client.getUsername() + "@localhost JOIN " + channel_name + "\r\n");
            // Then broadcast to other members
            new_channel->broadcastToMembers(client, "JOIN :" + channel_name);
            server->dmClient(client, RPL_NOTOPIC, channel_name + " :No topic is set");
            new_channel->listMembers(client);

            return;
        }

        if (existing_channel->isMember(client))
        {
            server->dmClient(client, ERR_USERONCHANNEL, "JOIN :You're already on that channel");
            return;
        }

        if (existing_channel->isFull())
        {
            server->dmClient(client, ERR_CHANNELISFULL, "JOIN :Channel is full");
            return;
        }

        bool is_channel_key_protected = existing_channel->mode('k');

        if (
            is_channel_key_protected &&
            params.size() < 2)
        {
            server->dmClient(client, ERR_NEEDMOREPARAMS, "JOIN :This channel requires a key");
            return;
        }

        if (
            is_channel_key_protected &&
            !existing_channel->isMatchingPassword(params[1]))
        {
            server->dmClient(client, ERR_BADCHANNELKEY, "JOIN :Bad channel key");
            return;
        }

        bool is_client_invited = existing_channel->isInvited(client);

        if (
            existing_channel->mode('i') &&
            !is_client_invited)
        {
            server->dmClient(client, ERR_INVITEONLYCHAN, "JOIN :Cannot join channel (+i)");
            return;
        }

        if (is_client_invited)
        {
            existing_channel->removeInvited(client);
        }

        // NOW WE ADD HIIIIIIIIIIIIIIIIIIIIIIIM (OR HER)
        existing_channel->addMember(client);

        // Send JOIN confirmation to the client themselves first
        server->sendMessage(client, ":" + client.getNick() + "!" + client.getUsername() + "@localhost JOIN " + channel_name + "\r\n");
        // Then broadcast to other members
        existing_channel->broadcastToMembers(client, "JOIN :" + channel_name);

        std::string topic = existing_channel->getTopic();
        if (topic.empty())
        {
            server->dmClient(client, RPL_NOTOPIC, channel_name + " :No topic is set");
        }
        else
        {
            server->dmClient(client, RPL_TOPIC, channel_name + " :" + topic);
        }

        existing_channel->listMembers(client);
    }
    catch (const std::exception &e)
    {
        server->dmClient(client, ERR_NOSUCHCHANNEL, "JOIN :" + std::string(e.what()));
    }
}
