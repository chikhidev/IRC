#include "Server.hpp"
#include "../Client/Client.hpp"
#include "../Services/Services.hpp"
#include "../Channel/Channel.hpp"

Server::Server(int p) : poll_fds(NULL), poll_count(0)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        throw std::runtime_error("Socket creation failed");
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(fd);
        throw std::runtime_error("Setsockopt failed");
    }

    port = p;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(fd);
        throw std::runtime_error("Bind failed");
    }

    if (listen(fd, 3) < 0)
    {
        close(fd);
        throw std::runtime_error("Listen failed");
    }

    // Set up polling with initial server socket
    poll_fds = new struct pollfd[1];
    poll_fds[0].fd = fd;
    poll_fds[0].events = POLLIN;
    poll_count = 1;

    services = new Services(this);
}

Server::~Server()
{
    close(fd);
    delete[] poll_fds;
    delete services;
}


/*
* Get the server file descriptor
*/
int Server::getFd() const
{
    return fd;
}

/*
* Get the server address
*/
sockaddr_in Server::getAddr() const
{
    return addr;
}

/*
* Set the server password
*/
void Server::setPassword(std::string &pass)
{
    password = pass;
}

void Server::registerClient(int fd, Client client)
{
}

/*
* Add a file descriptor to the poll set
*/
void Server::addPollFd(int new_fd)
{
    struct pollfd *new_poll_fds = new struct pollfd[poll_count + 1];
    for (int i = 0; i < poll_count; ++i)
    {
        new_poll_fds[i] = poll_fds[i];
    }
    new_poll_fds[poll_count].fd = new_fd;
    new_poll_fds[poll_count].events = POLLIN;

    delete[] poll_fds;
    poll_fds = new_poll_fds;
    poll_count++;
}

/*
* Remove a file descriptor from the poll set
*/
void Server::removePollFd(int fd_to_remove)
{
    int index = -1;
    for (int i = 0; i < poll_count; ++i)
    {
        if (poll_fds[i].fd == fd_to_remove)
        {
            index = i;
            break;
        }
    }
    if (index == -1)
        return; // Not found

    struct pollfd *new_poll_fds = new struct pollfd[poll_count - 1];
    for (int i = 0, j = 0; i < poll_count; ++i)
    {
        if (i != index)
        {
            new_poll_fds[j++] = poll_fds[i];
        }
    }

    delete[] poll_fds;
    poll_fds = new_poll_fds;
    poll_count--;
}

/*
* Remove a client from the server
*/
void Server::removeClient(int client_fd)
{
    std::cout << "[SERVER] Removing client: " << client_fd << std::endl;

    // Remove from clients map
    std::map<int, Client>::iterator it = clients.find(client_fd);
    if (it != clients.end())
    {
        it->second.disconnect();
        clients.erase(it);
    }
    
    // Remove from poll_fds
    removePollFd(client_fd);
    
    // Close the socket
    close(client_fd);
}

void Server::removeClient(Client& client)
{
    int client_fd = client.getFd();

    std::cout << "[SERVER] Removing client: " << client_fd << std::endl;

    // Remove from clients map
    std::map<int, Client>::iterator it = clients.find(client_fd);
    if (it != clients.end())
    {
        it->second.disconnect();
        clients.erase(it);
    }
    
    // Remove from poll_fds
    removePollFd(client_fd);
    
    // Close the socket
    close(client_fd);
}

/*
* Create a new client and add it to the clients map
*/
void Server::createClient(int fd)
{
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int new_socket = accept(fd, (struct sockaddr *)&client_addr, &addr_len);
    if (new_socket < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        return;
    }

    Client new_client(new_socket, client_addr, addr_len, this);

    clients[new_socket] = new_client;
    addPollFd(new_socket);

    std::cout << "[SERVER] Client connected: " << new_socket << std::endl;
}


/*
* Check if the provided password matches the server's password
*/
bool Server::isPasswordMatching(const std::string &pass) const
{
    return pass == password;
}


/*
* Public method for the service to access a client by fd
*/
Client& Server::getClient(int fd)
{
    return clients[fd];
}

/*
* Main server loop:
* - Polls for events
* - Accepts new connections
* - Reads data from clients
* - Handles client disconnections
* - Delegates command processing to the Services class
*/
void Server::loop()
{
    std::cout << "[SERVER] Server started on port " << port << std::endl;

    while (true)
    {
        int ready = poll(poll_fds, poll_count, -1);
        if (ready < 0)
        {
            throw std::runtime_error("Poll failed");
        }

        for (int i = poll_count - 1; i >= 0; --i)
        {
            if ((poll_fds[i].revents & POLLIN))
            {
                if (poll_fds[i].fd == fd)
                {
                    std::cout << "[SERVER] New connection on server socket" << std::endl;
                    createClient(fd);
                }
                else
                {
                    char buffer[1024] = {0};
                    int readed_bytes = read(poll_fds[i].fd, buffer, 1024);

                    if (readed_bytes <= 0)
                    {
                        if (readed_bytes == 0)
                        {
                            std::cout << "[SERVER] Client disconnected gracefully: " << poll_fds[i].fd << std::endl;
                        }
                        else
                        {
                            std::cerr << "Read error on fd: " << poll_fds[i].fd << " - errno: " << errno << std::endl;
                        }
                        
                        // Remove the client properly
                        removeClient(poll_fds[i].fd);
                    }
                    else
                    {
                        std::string msg(buffer, readed_bytes);
                        services->handleCommand(poll_fds[i].fd, msg);

                    }
                }
            }
        }
    }
}


/*-------------------------Messaging-------------------------*/
/*
* Send a message to all connected clients at once
*/
void Server::sendToAllClients(const std::string &message)
{
    std::string prefixed_message = ":ircserv " + message;

    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        int client_fd = it->first;
        if (send(client_fd, prefixed_message.c_str(), prefixed_message.length(), 0) < 0)
        {
            std::cerr << "[SERVER] Failed to send message to fd " << client_fd << std::endl;
        }
    }
}

/*
* Send a direct message to a specific client, perspective of ircserv
*/

void Server::dmClient(Client& client, int code, const std::string &message) {
    std::string response = ":ircserv ";

    response += (code < 10 ? "00" : (code < 100 ? "0" : "")) +
                           std::to_string(code) + " ";

    response += (client.hasNick() ? client.getNick() : "*");

    response += " :" + message + client.getCommandTerminators();

    this->sendMessage(client, response);
}

/*
* send a message to an FD
*/
void Server::sendMessage(Client& client, const std::string& message) {
    if (send(client.getFd(), message.c_str(), message.size(), 0) < 0) {
        std::cerr << "[SERVER] Failed to send message to fd "
                  << fd << std::endl;
    }
}
/*--------------------------------------------------------------*/



/*--------------------- Channel Management ---------------------*/
/*
* Public method for the service to access a channel by name
*/
Channel& Server::getChannel(const std::string& name)
{
    std::map<std::string, Channel>::iterator it = channels.find(name);
    if (it == channels.end()) {
        throw std::runtime_error("Channel not found: " + name);
    }
    return it->second;
}


/*
* Public method for the service to add a channel
*/
void Server::createChannel(const std::string& name, Client& creator)
{
    channels[name] = Channel(name, creator, this);
}


/*
* Public method for the service to remove a channel
*/
void Server::removeChannel(const std::string& name)
{
    channels.erase(name);
}

/*
* Public method for the service to check if a channel exists
*/
bool Server::channelExists(const std::string& name) const
{
    return channels.find(name) != channels.end();
}

/*
* Public method for the service to add a client to a channel
*/
void Server::addClientToChannel(const std::string& channel_name, Client& client)
{
    Channel& channel = getChannel(channel_name);
    channel.addMember(client);
}

/*
* Public method for the service to remove a client from a channel
*/
void Server::removeClientFromChannel(const std::string& channel_name, Client& client)
{
    Channel& channel = getChannel(channel_name);
    channel.removeMember(client);
}
/*--------------------------------------------------------------*/