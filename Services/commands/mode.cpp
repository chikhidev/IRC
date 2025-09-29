#include "../Services.hpp"
#include "../../Server/Server.hpp"
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"

/* MODE #channel +/-mode params */
void Services::mode(Client &client, std::vector<std::string> &params)
{
    if (params.size() < 2) {
        server->dmClient(client, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters");
        return;
    }

    Channel *existing_channel = server->getChannel(params[0]);
    
    if (!existing_channel) {
        server->dmClient(client, ERR_NOSUCHCHANNEL, params[0] + " :No such channel");
        return;
    }
    
    int param_index = 2;
    std::string &modes = params[1];

    if (!existing_channel->isOperator(client)) {
        server->dmClient(client, ERR_CHANOPRIVSNEEDED, params[0] + " :You're not channel operator");
        return;
    }

    bool found_state = false;
    bool state = true;
    
    for (size_t i = 0; i < modes.size(); i++) {
        if (modes[i] == '+' || modes[i] == '-') {
            found_state = true;
            state = (modes[i] == '+');
            continue;
        }

        if (!found_state) {
            server->log("MODE: Missing + or - before modes");
            server->dmClient(client, ERR_UNKNOWNMODE, std::string(1, modes[i]) + " :is not a valid mode");
            return;
        }

        char chosen_mode = modes[i];
        std::string *param = NULL;

        if (param_index < (int)params.size()) {
            param = &params[param_index];
        }

        try {
            server->log("Processing mode: " + std::string(1, chosen_mode) + " with state: " + (state ? "+" : "-"));

            switch (chosen_mode) {
                case 'i':
                case 't':
                    existing_channel->updateMode(chosen_mode, state);
                    existing_channel->broadcastToMembers(client, ":" + client.getNick() + " MODE " + existing_channel->getName() + " " + (state ? "+" : "-") + chosen_mode);
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
                    server->dmClient(client, ERR_UNKNOWNMODE, "'" + std::string(1, chosen_mode) + "' :is not a valid mode");
                    return;
            }
        } catch (const std::runtime_error &e) {
            server->dmClient(client, ERR_UNKNOWNMODE, "MODE: " + std::string(e.what()));
            return;
        }
    }
}


/* MODE #channel +k <key> or MODE #channel -k */
// Basically the bool return is to indicate whether the mode used a parameter or not
bool Services::handlePass(Channel &c, Client &client, bool state, std::string *param)
{
    if (state && !param) {
        server->dmClient(client, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters");
        return false;
    }

    if (state && param->empty()) {
        server->dmClient(client, ERR_NEEDMOREPARAMS, "MODE :Bad key");
        return true;
    }

    c.updateMode('k', state);

    if (!state) c.updatePassword("");
    else c.updatePassword(*param);

    std::string mode_change = ":" + client.getNick() + " MODE " + c.getName() + " " + (state ? "+k" : "-k");
    c.broadcastToMembers(client, mode_change);
    
    return state;
}

/* MODE #channel +l <limit> or MODE #channel -l */
// Basically the bool return is to indicate whether the mode used a parameter or not
bool Services::handleMembersLimit(Channel &c, Client &client, bool state, std::string *param)
{
    if (state && !param) {
        server->dmClient(client, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters");
        return false;
    }

    if (state && param->empty()) {
        server->dmClient(client, ERR_NEEDMOREPARAMS, "MODE :Bad <limit>");
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
        server->dmClient(client, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters");
        return false;
    }

    if (param->empty()) {
        server->dmClient(client, ERR_NEEDMOREPARAMS, "MODE :Erroneous nickname");
        return state;
    }

    Client *target_client = server->getClientByNick(*param);
    if (!target_client) {
        server->dmClient(client, ERR_NOSUCHNICK, *param + " :No such nick/channel");
        return state;
    }

    if (c.isOperator(*target_client) && state) {
        server->dmClient(client, ERR_USERONCHANNEL, *param + " :is already an operator");
        return state;
    }

    if (!c.isMember(*target_client)) {
        server->dmClient(client, ERR_USERONCHANNEL, *param + " :They aren't on that channel");
        return state;
    }

    if (state) {
        c.addOperator(*target_client);
        c.broadcastToMembers(client, ":" + client.getNick() + " MODE " + c.getName() + " +o " + *param);
    } else {
        c.removeOperator(*target_client);
        server->dmClient(client, ERR_CHANOPRIVSNEEDED, c.getName() + " -o " + *param);
        c.broadcastToMembers(client, ":" + client.getNick() + " MODE " + c.getName() + " -o " + *param);
    }

    return state;
}
