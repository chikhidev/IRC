#include "Services.hpp"
#include "../Server/Server.hpp"

void Services::handleQuit(Client &client, std::string &params) {
    
    (void)params; // Unused parameter

    server->dmClient(client, 221, "Closing Link, Bye!\r\n");
    std::cout << "[SERVICE] Client with fd " << client.getFd() << " has quit." << std::endl;
    server->removeClient(client.getFd());

}