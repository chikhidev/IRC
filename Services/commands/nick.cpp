#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"

/*
 * Handle the NICK command
 */
void Services::nick(Client &client, std::vector<std::string> &params)
{
    if (!server)
    {
        throw std::runtime_error("Server reference is null");
    }

    if (params.empty())
    {
        server->dmClient(client, ERR_NONICKNAMEGIVEN, ":No nickname given");
        return;
    }

    if (params.size() > 1)
    {
        server->dmClient(client, ERR_ERRONEUSNICKNAME, ":Erroneous nickname");
        return;
    }

    std::string nickname = params[0];

    if (nickname.length() > NICK_LIMIT)
    {
        server->dmClient(client, ERR_ERRONEUSNICKNAME, ":Erroneous nickname");
        return;
    }

    // Check if nickname is already in use
    Client *existing_client = server->getClientByNick(nickname);
    if (existing_client && existing_client != &client)
    {
        server->dmClient(client, ERR_NICKNAMEINUSE, nickname + " :Nickname is already in use");
        return;
    }

    server->removeUniqueNick(client.getNick());
    std::string old_nick = client.getNick();
    client.setNick(nickname);
    server->addUniqueNick(nickname, client);

    server->updateClientNickInAllChannels(old_nick, nickname, client);
    
    if (old_nick.empty())
        old_nick = nickname;

    std::string nick_change_msg = ":" + old_nick + "!" + client.getUsername() + "@localhost NICK " + nickname + "\r\n";

    server->broadcastToAllClients(nick_change_msg);

    if (!client.isRegistered() && !client.getUsername().empty())
    {
        client.setRegistered(true);
        server->dmClient(client, RPL_WELCOME, ":Welcome to the IRC network, " + client.getNick() + "!" + client.getUsername() + "@localhost");
    }
}
