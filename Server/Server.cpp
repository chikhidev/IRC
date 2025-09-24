#include "Server.hpp"
#include "../Services/Services.hpp"
#include "../Client/Client.hpp"
#include "../Channel/Channel.hpp"

/*
* Just the constructor
*/
Server::Server(int p)
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

    makeNonBlocking(fd);

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

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        close(fd);
        throw std::runtime_error("Epoll create failed");
    }
    event.events = EPOLLIN;
    event.data.fd = fd;
    // Add the server socket to the epoll instance
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
        close(fd);
        close(epoll_fd);
        throw std::runtime_error("Epoll ctl failed");
    }

    // Since the server socket is the only one in the set initially
    event_count = 1;

    services = new Services(this);
    
}

/*
* Just the destructor
*/
Server::~Server()
{
    close(fd);

    // Clean up clients
    std::map<int, Client*>::iterator cl_it = clients.begin();
    for (; cl_it != clients.end(); ++cl_it) {
        cl_it->second->disconnect();
        cl_it->second->quitAllChannels();
        delete cl_it->second;
    }
    clients.clear();

    // Clean up channels
    std::map<std::string, Channel*>::iterator ch_it = channels.begin();
    for (; ch_it != channels.end(); ++ch_it) {
        delete ch_it->second;
    }
    channels.clear();

    close(epoll_fd);
    delete services;
}

/*
* Force add non-blocking mode to a given file descriptor
*/
void Server::makeNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("Failed to get file descriptor flags");
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error("Failed to set non-blocking mode");
    }
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
* Add a file descriptor to the epoll set
*/
void Server::addEpollFd(int new_fd)
{
    makeNonBlocking(new_fd);
    event.data.fd = new_fd;
    event.events = EPOLLIN;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &event) == -1) {
        throw std::runtime_error("Epoll ctl failed");
    }

    event_count++;
}


/*
* Remove a file descriptor from the epoll set
*/
void Server::removeEpollFd(int fd_to_remove)
{
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd_to_remove, NULL) == -1) {
        throw std::runtime_error("Epoll ctl remove failed");
    }
}

/*
* Remove a client from the server
*/
void Server::removeClient(int client_fd)
{
    log("Removing client: " + std::to_string(client_fd));

    // Check the clients map
    std::map<int, Client*>::iterator it = clients.find(client_fd);
    if (it != clients.end())
    {
        log("Found client " + std::to_string(client_fd) + ", proceeding to disconnect.");

        Client* existing_client = getClient(client_fd);
        if (existing_client) {
            log("Found client instance, disconnecting.");

            existing_client->disconnect();
            existing_client->quitAllChannels();

            if (existing_client->hasNick()) {
                removeUniqueNick(existing_client->getNick());
            }

            delete existing_client;

            log("Client " + std::to_string(client_fd) + " disconnected and removed.");
        }
        
        clients.erase(it);
        log("Client " + std::to_string(client_fd) + " erased from clients map.");
        removeEpollFd(client_fd);
        log("Client " + std::to_string(client_fd) + " removed from epoll set.");
        close(client_fd);
        log("Client " + std::to_string(client_fd) + " socket closed.");

        event_count--;

    } else {
        log("Client " + std::to_string(client_fd) + " not found in clients map.");
    }
}

void Server::removeClient(Client& client)
{
    removeClient(client.getFd());
}


/* Add a unique nickname to the map
*/
void Server::addUniqueNick(const std::string &nick, Client &client) {
    unique_nicks[nick] = &client;
}

/* Remove a unique nickname from the map
*/
void Server::removeUniqueNick(const std::string &nick) {
    unique_nicks.erase(nick);
}

/*
* Create a new client and add it to the clients map
*/
void Server::createClient()
{
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int new_socket = accept(fd, (struct sockaddr *)&client_addr, &addr_len);
    if (new_socket < 0)
    {
        log("Accept failed");
        return;
    }

    clients[new_socket] = new Client(new_socket, client_addr, addr_len, this);
    makeNonBlocking(new_socket);
    addEpollFd(new_socket);

    log("Client connected: " + std::to_string(new_socket));
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
Client* Server::getClient(int fd)
{
    std::map<int, Client*>::iterator it = clients.find(fd);
    if (it == clients.end()) {
        return NULL;
    }
    return it->second; 
}

Client* Server::getClientByNick(const std::string &nick)
{
    std::map<std::string, Client*>::iterator it = unique_nicks.find(nick);
    if (it == unique_nicks.end()) {
        return NULL;
    }
    return it->second;
}

/*
* Main server loop:
* - ePolls for events
* - Accepts new connections
* - Reads data from clients
* - Handles client disconnections
* - Delegates command processing to the Services class
* - Handles sleepy clients (new)
*/
void Server::loop()
{
    log("Server started on port " + std::to_string(port));

    while (true)
    {
        int n = epoll_wait(epoll_fd, events, MAX_CONNECTIONS, -1);

        for (int i = 0; i < n; ++i)
        {
            struct epoll_event &ev = events[i];

            if (ev.events & EPOLLIN)
            {
                if (ev.data.fd == fd)
                {
                    if (event_count >= MAX_CONNECTIONS) {
                        log("Maximum connections reached, rejecting new connection.");
                        continue;
                    }

                    log("[SERVER] New connection on server socket");
                    createClient();
                }
                else {
                    services->dealWithClient(ev.data.fd);
                }
            } else if (
                (ev.events & (EPOLLHUP | EPOLLERR)) ||
                (ev.events == 0)
            )
            {
                log("Client disconnected or error on fd " + std::to_string(ev.data.fd));
                removeClient(ev.data.fd);
            } else {
                log("No relevant events, checking for sleepy clients");
                services->dealWithSleepModeClient(ev.data.fd);
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

    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        int client_fd = it->first;
        if (send(client_fd, prefixed_message.c_str(), prefixed_message.length(), 0) < 0)
        {
            log("Failed to send message to fd " + std::to_string(client_fd));
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
        log("Failed to send message to fd " + std::to_string(client.getFd()));
    }
}
/*--------------------------------------------------------------*/



/*--------------------- Channel Management ---------------------*/
/*
* Public method for the service to access a channel by name
*/
Channel* Server::getChannel(const std::string& name)
{
    std::map<std::string, Channel*>::iterator it = channels.find(name);
    if (it == channels.end()) {
        return NULL;
    }
    return it->second;
}


/*
* Public method for the service to add a channel
*/
void Server::createChannel(const std::string& name, Client& creator)
{
    channels[name] = new Channel(name, creator, this);
}


/*
* Public method for the service to remove a channel
*/
void Server::removeChannel(const std::string& name)
{
    std::map<std::string, Channel*>::iterator it = channels.find(name);
    if (it == channels.end()) {
        throw std::runtime_error("Channel not found: " + name);
    }
    delete it->second;
    channels.erase(it);
}
/*--------------------------------------------------------------*/


/*
* Public method for the service to log messages
*/
void Server::log(const std::string &message) const {
    std::cout << "[SERVER] " << message << std::endl;
}

/*
* Get the time difference in seconds from a given timestamp to now in seconds
*/
size_t Server::getDiffTime(size_t last_time) const {
    return static_cast<size_t>(time(NULL) - last_time);
}
