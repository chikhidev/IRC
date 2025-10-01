#include "../Server/Server.hpp"
#include "../Channel/Channel.hpp"
#include "Client.hpp"

/*
 * Parameterized constructor
 */
Client::Client(int socket_fd, sockaddr_in address, socklen_t length, Server *srv)
{
    fd = socket_fd;
    memcpy(&addr, &address, sizeof(address));
    addr_len = length;
    _isRegistered = false;
    _isAuthenticated = false;
    _connected = true;
    _sent_first_command = false;
    server = srv;
    last_active_time = time(NULL);
    is_pinged = false;
}

/*
 * Destructor
 */
Client::~Client()
{
    server->log("Client " + glob::to_string(fd) + " destroyed.");
}

/*
 * Set the client address
 */
void Client::setAddr(sockaddr_in address)
{
    memcpy(&addr, &address, sizeof(address));
}

/*
 * Set the client address length
 */
void Client::setAddrLen(socklen_t length)
{
    addr_len = length;
}

/*
 * Get the client file descriptor
 */
int Client::getFd() const
{
    return fd;
}

/*
 * Get the client address
 */
sockaddr_in Client::getAddr()
{
    return addr;
}

/*
 * Get the client address length
 */
socklen_t Client::getAddrLen()
{
    return addr_len;
}

/*
 * Set the client file descriptor
 */
void Client::setFd(int socket_fd)
{
    fd = socket_fd;
}

/*
 * Check if the client is registered
 * Registered means he set-upped USER command successfully
 */
bool Client::isRegistered() const
{
    return _isRegistered;
}

/*
 * Set the client registration status
 * Registered means he set-upped USER command successfully
 */
void Client::setRegistered(bool status)
{
    _isRegistered = status;
}

/*
 * Set the client authentication status
 * Authenticated means he provided the correct PASS command
 */
void Client::setAuthenticated(bool status)
{
    _isAuthenticated = status;
}

/*
 * Check if the client has a nickname
 */
bool Client::hasNick() const
{
    return !nickname.empty();
}

/*
 * Set the client nickname
 */
void Client::setNick(const std::string &nick)
{
    nickname = nick;
}

/*
 * Get the client nickname
 */
std::string Client::getNick() const
{
    return nickname;
}

/*
 * Set the client username
 */
void Client::setUsername(const std::string &user)
{
    username = user;
}

/*
 * Set the client realname
 */
void Client::setRealname(const std::string &real)
{
    realname = real;
}

/*
 * Check if the client is authenticated
 * Authenticated means he provided the correct PASS command
 */
bool Client::isAuthenticated() const
{
    return _isAuthenticated;
}

/*
 * Get the client username
 */
std::string Client::getUsername() const
{
    return username;
}

/*
 * Get the client realname
 */
std::string Client::getRealname() const
{
    return realname;
}

/*
 * Disconnect the client (just to not process his commands anymore!, not what you think)
 */
void Client::disconnect()
{
    _connected = false;
}

/*
 * Check if the client is connected (read the Client::disconnect() method comment to understand)
 */
bool Client::isConnected() const
{
    return _connected;
}

/*
 * Set the client first command status
 */
void Client::setSentFirstCommand()
{
    _sent_first_command = true;
}

/*
 * send a message to another client, perspective of this client
 */
void Client::sendMessage(Client &receiver, const std::string &message)
{
    if (!server)
    {
        throw std::runtime_error("Server reference is null");
    }

    std::string formatted_client = ":" + nickname + "!" + username + "@localhost";

    std::string formatted_message = formatted_client + " " + message;

    formatted_message += "\r\n";

    server->sendMessage(receiver, formatted_message);
}

/*
 * Add a channel to the client's joined channels list, this is used when quitting all channels
 * to avoid iterating over all channels in the server, and for fast&easy access relationship
 */
void Client::addToJoinedChannels(const std::string &channel_name)
{
    joined_channels.push_back(channel_name);
}

/*
 * Remove a channel from the client's joined channels list, this is used when quitting all channels
 * to avoid iterating over all channels in the server, and for fast&easy access relationship
 */
void Client::removeFromJoinedChannels(const std::string &channel_name)
{
    for (std::vector<std::string>::iterator it = joined_channels.begin(); it != joined_channels.end(); ++it)
    {
        if (*it == channel_name)
        {
            joined_channels.erase(it);
            return;
        }
    }
}

/*
 * Quit all channels the client has joined, used when the client disconnects (real disconnect)
 */
void Client::quitAllChannels()
{
    if (!server)
    {
        throw std::runtime_error("Server reference is null");
    }

    size_t channel_count = joined_channels.size();

    server->log("Client " + glob::to_string(fd) + " is quitting all channels (" + glob::to_string(channel_count) + ").");

    if (channel_count == 0)
    {
        return;
    }

    for (size_t i = channel_count - 1;; i--)
    {
        server->log("[CLIENT] Client " + glob::to_string(fd) + " quitting channel " + joined_channels[i]);

        Channel *channel = server->getChannel(joined_channels[i]);
        if (!channel)
        {
            if (i == 0)
                break;
            continue;
        }

        if (channel->isMember(*this))
        {
            std::string quit_message = "PART " + channel->getName();
            channel->broadcastToMembers(*this, quit_message);
            channel->removeMember(*this);
        }

        if (i == 0)
            break;
    }

    server->log("Client " + glob::to_string(fd) + " has quit all channels.");
    joined_channels.clear();
}

/*
 * Here we get the command stream buffer, since we are using non-blocking sockets
 * we need to accumulate the data until we get a full command (terminated by \r\n)
 * each payload received from the client is appended to this stream
 */
std::stringstream &Client::getCommandStream()
{
    return command_buffer;
}

/*
 * Here we clear the command stream buffer, after processing a full command
 */
void Client::clearCommandStream()
{
    command_buffer.str("");
    command_buffer.clear();
}

/*
 * Get the last active time of the client (last time he sent a command)
 */
size_t Client::getLastActiveTime() const
{
    return last_active_time;
}

/*
 * Set the last active time of the client (last time he sent a command)
 */
void Client::setLastActiveTime(size_t time)
{
    last_active_time = time;
}

bool Client::isPinged() const
{
    return is_pinged;
}

void Client::setIsPinged(bool state)
{
    is_pinged = state;
}
