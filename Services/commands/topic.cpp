#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"

void Services::topic(Client &client, std::vector<std::string> &params) {

    if (!server) {
        throw std::runtime_error("Server reference is null");
    }

    if (params.size() < 1) {
        server->dmClient(client, 461, "TOPIC :bad number of parameters");
        return;
    }

    std::string channel_name = params[0];

    if (channel_name[0] != '#') {
        server->dmClient(client, 403, channel_name + " :No such channel");
        return;
    }

    std::string new_topic;

    if (params.size() > 1) {
        new_topic = params[1];

        for (size_t i = 2; i < params.size(); ++i) {
            new_topic += " " + params[i];
        }
    }

    server->log("[SERVICES] TOPIC command by " + client.getNick() + " on channel " + channel_name + " with topic: " + new_topic);

    if (!new_topic.empty() && new_topic[0] != ':') {
        server->dmClient(client, 461, "TOPIC :badly formed topic");
        return ;
    }
    
    Channel *channel = server->getChannel(channel_name);
    if (!channel) {
        server->dmClient(client, 403, channel_name + " :No such channel");
        return;
    }

    if (!channel->isMember(client)) {
        server->dmClient(client, 442, channel_name + " :You're not a member of that channel");
        return;
    }

    if (new_topic.empty()) {
        if (channel->getTopic().empty()) {
            server->dmClient(client, 331, channel->getName() + " :No topic is set");
        } else {
            server->dmClient(client, 332, channel->getName() + " :" + channel->getTopic());
        }

        return ;
    }

    bool is_operator = channel->isOperator(client);
    bool only_operators = channel->mode('t');

    if (only_operators && !is_operator) {
        server->dmClient(client, 482, channel_name + " :Only channel operators may set the topic");
        return;
    }

    if (new_topic[0] == ':') {
        new_topic.erase(0, 1); // Remove leading ':' if present
    }
    
    channel->setTopic(new_topic);
    server->dmClient(client, 332, channel_name + " :" + new_topic);
    channel->broadcastToMembers(client, ":" + client.getNick() + " TOPIC " + channel_name + " :" + new_topic);
    
}
