#include "Server.hpp"

Server::Server(int p) : poll_fds(NULL), poll_count(0)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        throw std::runtime_error("Socket creation failed");
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
}

Server::~Server()
{
    close(fd);
    delete[] poll_fds;
}

int Server::getFd() const
{
    return fd;
}

sockaddr_in Server::getAddr() const
{
    return addr;
}

void Server::setPassword(std::string &pass)
{
    password = pass;
}

void Server::registerClient(int fd, Client client)
{
    
}

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
    if (index == -1) return; // Not found

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
    
    Client new_client(new_socket, client_addr, addr_len);

    clients[new_socket] = new_client;
    addPollFd(new_socket);
    
    std::cout << "Client connected: " << new_socket << std::endl;
    send(new_socket, "Welcome to the IRC server!\n", 27, 0);
}

void Server::start()
{
    std::cout << "Server started on port " << port << std::endl;

    while (true)
    {
        int ready = poll(poll_fds, poll_count, -1);
        if (ready < 0)
        {
            throw std::runtime_error("Poll failed");
        }

        for (int i = 0; i < poll_count; ++i)
        {
            if ((poll_fds[i].revents & POLLIN)) {
                if (poll_fds[i].fd == fd) {

                    std::cout << "New connection on server socket" << std::endl;
                    createClient(fd);

                } else {
                    char buffer[1024] = {0};
                    int readed_bytes = read(poll_fds[i].fd, buffer, 1024);

                    if (readed_bytes < 0) {
                        std::cerr << "Read error on fd: " << poll_fds[i].fd << std::endl;

                        int saved_fd = poll_fds[i].fd;
                        clients.erase(saved_fd);
                        removePollFd(saved_fd);
                        close(saved_fd);
                        i--;

                        std::cout << "Client disconnected: " << saved_fd << std::endl;

                    } else {

                        std::cout << "Received data from fd: " << poll_fds[i].fd << std::endl;
                        std::string msg(buffer, readed_bytes = read(poll_fds[i].fd, buffer, 1024));
                        commandHandler(poll_fds[i].fd, msg);
                    }
                }

            }
        }
    }
}


void Server::commandHandler(int fd, std::string buffer)
{
    // Parse and handle commands from clients
    std::istringstream iss(buffer);
    std::string command;
    iss >> command;

   std::cout << "Received command: " << command << " from fd: " << fd << std::endl;
}

