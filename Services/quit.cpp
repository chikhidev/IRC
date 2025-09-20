#include "Services.hpp"
#include "../Server/Server.hpp"

void Services::handleQuit(Client &client, std::string &params) {
    
    (void)params;

    server->dmClient(client, 221, "Closing Link, Bye!");
    std::cout << "[SERVICE] Client with fd " << client.getFd() << " has quit." << std::endl;
    
    client.disconnect();
}