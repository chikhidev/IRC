#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"


void Services::mode(Client &client, std::vector<std::string> &params)
{
    if (params.size() < 2) {
        server->dmClient(client, 461, "MODE :Not enough parameters");
        return;
    }

    if (!server->channelExists(params[0])) {
        server->dmClient(client, 403, params[0] + " :No such channel");
        return;
    }

    if (params[1].size() < 2 || (params[1][0] != '+' && params[1][0] != '-')) {
        server->dmClient(client, 472, params[1] + " :is not a valid mode");
        return;
    }

    Channel existing_channel = server->getChannel(params[0]);

    if (!existing_channel.isOperator(client)) {
        server->dmClient(client, 482, params[0] + " :You're not channel operator");
        return;
    }

    char chosen_mode = params[1][1];

    try {
        switch (chosen_mode) {
            case 'i':
            case 't':
                existing_channel.updateMode(chosen_mode, params[1][0] == '+');
                break;
            case 'k':
                handlePass(existing_channel, client, params);
                break;
            case 'l':
                handleMembersLimit(existing_channel, client, params);
                break;
            case 'o':
                handleOperator(existing_channel, client, params);
                break;
            default:
                server->dmClient(client, 472, std::string(1, chosen_mode) + " :is not a valid mode");
                return;
        }
    } catch (const std::runtime_error &e) {
        server->dmClient(client, 472, "MODE: " + std::string(e.what()));
        return;
    }
}

void Services::handlePass(Channel &c, Client &client, std::vector<std::string> &params)
{
    if (params.size() < 3) {
        server->dmClient(client, 461, "MODE :Not enough parameters");
        return;
    }

    bool set_pass = params[1][0] == '+';

    c.updateMode('k', set_pass);
    c.updatePassword(params[2]);
    server->dmClient(client, 324, c.getName() + " " + (set_pass ? "+k" : "-k"));
}

void Services::handleMembersLimit(Channel &c, Client &client, std::vector<std::string> &params)
{
    bool set_limit = params[1][0] == '+';

    if (set_limit && params.size() < 3) {
        server->dmClient(client, 461, "MODE :Not enough parameters");
        return;
    }

    c.updateMode('l', set_limit);

    if (set_limit) {
        int limit = std::atoi(params[2].c_str());
        c.updateUserLimit(limit);
        server->dmClient(client, 324, c.getName() + " +l " + params[2]);
    } else {
        c.updateUserLimit(0);
        server->dmClient(client, 324, c.getName() + " -l");
    }
}

void Services::handleOperator(Channel &c, Client &client, std::vector<std::string> &params)
{
    if (params.size() < 2) {
        server->dmClient(client, 461, "MODE :Not enough parameters");
        return;
    }

    std::string &target_nick = params[2];

    if (!c.isMember(*server->existingNick(target_nick))) {
        server->dmClient(client, 441, target_nick + " :They aren't on that channel");
        return;
    }

    bool add_operator = params[1][0] == '+';

    if (add_operator) {
        c.addOperator(client);
        server->dmClient(client, 324, c.getName() + " +o " + target_nick);
    } else {
        c.removeOperator(client);
        server->dmClient(client, 324, c.getName() + " -o " + target_nick);  
    }
}
