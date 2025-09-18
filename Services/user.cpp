#include "Services.hpp"
#include "../Server/Server.hpp"

void Services::handleUser(Client& client, std::string &params) {
    std::istringstream iss(params);
    std::string username, mode, unused, realname;

    if (!(iss >> username >> mode >> unused)) {
        server->dmClient(client, ":ircserv 461 " + client.getNick() + " USER :Not enough parameters\r\n");
        return;
    }

    // Get the realname (rest of the line)
    std::getline(iss, realname);
    if (!realname.empty() && realname[0] == ' ')
        realname.erase(0, 1);
    if (!realname.empty() && realname[0] == ':')
        realname.erase(0, 1);

    client.setUsername(username);
    client.setRealname(realname);


    client.setRegistered(true);

    // Send registration burst
    server->dmClient(client, ":ircserv 001 " + client.getNick() + " :Welcome to the IRC network, " + client.getNick() + "!" + client.getUsername() + "@localhost\r\n");
}
