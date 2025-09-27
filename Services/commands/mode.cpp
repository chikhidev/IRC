#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"

/* MODE #channel +/-mode params */
void Services::mode(Client &client, std::vector<std::string> &params)
{
    if (params.size() < 2) {
        server->dmClient(client, 461, "MODE :Not enough parameters");
        return;
    }

    Channel *existing_channel = server->getChannel(params[0]);
    
    if (!existing_channel) {
        server->dmClient(client, 403, params[0] + " :No such channel");
        return;
    }
    
    int param_index = 2;
    std::string &modes = params[1];

    if (modes.size() % 2 != 0) {
        server->dmClient(client, 472, modes + " :is not a valid mode");
        return;
    }

    const std::string valid_modes = "itkol";
    for (size_t i = 0; i < modes.size() - 1; i += 2) {
        if (modes[i] != '+' && modes[i] != '-') {
            server->dmClient(client, 472, std::string() + modes[i] + modes[i + 1] + " :is not a valid mode");
            return;
        }

        if (valid_modes.find(modes[i + 1]) == std::string::npos) {
            server->dmClient(client, 472, std::string() + modes[i] + modes[i + 1] + " :is not a valid mode");
            return;
        }
    }
    


    if (!existing_channel->isOperator(client)) {
        server->dmClient(client, 482, params[0] + " :You're not channel operator");
        return;
    }

    for (size_t i = 0; i < modes.size(); i += 2) {
        char chosen_mode = modes[i + 1];
        bool state = (modes[i] == '+');
        std::string *param = NULL;

        if (param_index < (int)params.size()) {
            param = &params[param_index];
        }

        try {
            switch (chosen_mode) {
                case 'i':
                case 't':
                    existing_channel->updateMode(chosen_mode, params[1][0] == '+');
                    existing_channel->broadcastToMembers(client, ":" + client.getNick() + " MODE " + existing_channel->getName() + " " + params[1]);
                    break;
                case 'k':
                    param_index += handlePass(*existing_channel, client, state, param);
                    break;
                case 'l':
                    param_index += handleMembersLimit(*existing_channel, client, state, param);
                    break;
                case 'o':
                    param_index += handleOperator(*existing_channel, client, state, param);
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
}


/* MODE #channel +k <key> or MODE #channel -k */
// Basically the bool return is to indicate whether the mode used a parameter or not
bool Services::handlePass(Channel &c, Client &client, bool state, std::string *param)
{
    if (state && !param) {
        server->dmClient(client, 461, "MODE :Not enough parameters");
        return false;
    }

    if (state && param->empty()) {
        server->dmClient(client, 461, "MODE :Bad key");
        return true;
    }

    c.updateMode('k', state);
    c.updatePassword(*param);
    std::string mode_change = ":" + client.getNick() + " MODE " + c.getName() + " " + (state ? "+k" : "-k");
    c.broadcastToMembers(client, mode_change);
    return state;
}

/* MODE #channel +l <limit> or MODE #channel -l */
// Basically the bool return is to indicate whether the mode used a parameter or not
bool Services::handleMembersLimit(Channel &c, Client &client, bool state, std::string *param)
{
    if (state && !param) {
        server->dmClient(client, 461, "MODE :Not enough parameters");
        return false;
    }

    if (state && param->empty()) {
        server->dmClient(client, 461, "MODE :Bad <limit>");
        return true;
    }

    c.updateMode('l', state);

    if (state) {
        int limit = std::atoi(param->c_str());
        c.updateUserLimit(limit);
        c.broadcastToMembers(client, ":" + client.getNick() + " MODE " + c.getName() + " +l " + *param);
    } else {
        c.updateUserLimit(0);
        c.broadcastToMembers(client, ":" + client.getNick() + " MODE " + c.getName() + " -l");
    }

    return state;
}


/* MODE #channel +/-o <nick>*/
// Basically the bool return is to indicate whether the mode used a parameter or not
bool Services::handleOperator(Channel &c, Client &client, bool state, std::string *param)
{
    if (!param) {
        server->dmClient(client, 461, "MODE :Not enough parameters");
        return false;
    }

    if (param->empty()) {
        server->dmClient(client, 461, "MODE :Erroneous nickname");
        return state;
    }

    Client *target_client = server->getClientByNick(*param);
    if (!target_client) {
        server->dmClient(client, 401, *param + " :No such nick/channel");
        return state;
    }

    if (!c.isMember(*target_client)) {
        server->dmClient(client, 441, *param + " :They aren't on that channel");
        return state;
    }

    if (state) {
        c.addOperator(*target_client);
        c.broadcastToMembers(client, ":" + client.getNick() + " MODE " + c.getName() + " +o " + *param);
    } else {
        c.removeOperator(*target_client);
        server->dmClient(client, 324, c.getName() + " -o " + *param);
        c.broadcastToMembers(client, ":" + client.getNick() + " MODE " + c.getName() + " -o " + *param);
    }

    return state;
}
