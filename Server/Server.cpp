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

Server::~Server() throw()
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

void Server::createClient(int fd, sockaddr_in addr, socklen_t addr_len)
{
    Client new_client;

    int new_socket = accept(fd, (struct sockaddr *)new_client.getAddr(), (socklen_t *)new_client.getAddrLen());
    if (new_socket < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        return;
    }
    std::cout << "New connection, socket fd is " << new_socket << std::endl;

    new_client.setFd(new_socket);

    std::cout << "Client connected: " << new_socket << std::endl;
}

void Server::start()
{
    std::cout << "Server started on port " << port << std::endl;

    // logic to accept and handle client connections would go here

    while (true)
    {
        int ready = poll(poll_fds, poll_count, -1);
        if (ready < 0)
        {
            throw std::runtime_error("Poll failed");
        }

        for (int i = 0; i < poll_count; ++i)
        {
            if (poll_fds[i].revents & POLLIN)
            {
                if (poll_fds[i].fd == fd)
                {
                    createClient(fd, addr, sizeof(addr));
                }
            }
            else
            {
                // Handle data from existing client
                
            }
        }
    }
}
