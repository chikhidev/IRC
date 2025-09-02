#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>

#define MAX_BIND_TRIES 10
#define PORT 3000

using namespace std;

class Client {
    string name;
    int fd;
    sockaddr_in addressInfos;
    uint64_t addressInfosSize;

public:
    Client(uint64_t port) {
      addressInfos.sin_family = AF_INET; 
      addressInfos.sin_port = htons(port);
      addressInfos.sin_addr.s_addr = INADDR_LOOPBACK;

      fd = socket(addressInfos.sin_family, SOCK_STREAM, 0);
      if (fd < 0) {
        throw std::runtime_error("Failed to creat a socket for a client!");
      }

      cout << "Client created successfuly!" << endl;
    };


    void connectToServer(sockaddr_in &infos, uint64_t addrlen) {
      
      int state = connect(fd, reinterpret_cast<const struct sockaddr *>(&infos),
            static_cast<socklen_t>(addrlen));
      if (state < 0) {
        throw std::runtime_error("Failed to connect client with server!");
      }

      cout << "Client conneceted successfuly!" << endl;
    }

    sockaddr_in *getAddress() {
      return &addressInfos; 
    }

    size_t* getAddressLen() {
      return &addressInfosSize;
    }

    int getFd() const {
      return fd;
    }

    void closeConnection() {
      close(fd);
      cout << "Client disconnected successfuly!" << endl;
    }

    ~Client() {};
};

int main(int ac, char **av) {

    if (ac != 3) {
      cerr << "Usage: ./server <port> <password>" << endl;
      return 1;
    }

    sockaddr_in infos;
    int port = atoi(av[1]);
    string password = av[2];

    infos.sin_family = AF_INET;
    infos.sin_port = htons(port);
    infos.sin_addr.s_addr = INADDR_ANY;

    int socketFd;
    uint64_t infosSize = sizeof(infos);

    socketFd = socket(infos.sin_family, SOCK_STREAM, 0);

    if (socketFd < 0) {
        cout << "Error creating the socket!" << endl;
        return 1;
    }

    if (bind(socketFd, reinterpret_cast<const struct sockaddr*>(&infos), infosSize) < 0) {
        cerr << "Error binding, changing the port to " << ++port << endl;
        close(socketFd);
        return 1;
    }

    if (listen(socketFd, SOMAXCONN) < 0) {
        cerr << "Error listening to clients!" << endl;
        close(socketFd);
        return 1;
    }

    cout << "Server is running successfuly on port: " << port << endl;

    try {

      // if (fork() == 0) {
      //   // Client's scope
      //   Client user(8082);
      //   user.connectToServer(infos, infosSize);

      //   //now lets try to send something and get it from the server
      //   string message = "Hello from the client!";

      //   size_t sent_bytes = write(user.getFd(), message.c_str(), message.size());

      //   if (sent_bytes < 0) {
      //     throw std::runtime_error("Failed to send the message to the server!");
      //   }

        

      //   user.closeConnection();
      //   return 0;

      // } else {

      sockaddr_in client_add;
      size_t client_add_size = sizeof(client_add);

      // now phase to accept the clinet from the server:
      int clientFd = accept(socketFd, reinterpret_cast<struct sockaddr *>(&client_add),
          reinterpret_cast<socklen_t *>(&client_add_size));

      if (clientFd < 0) {
        throw std::runtime_error("Failed to accept the connection of the user!");
      }

      cout << "Client accepted successfully from the server!" << endl;

      char buff[1024] = {0};

      ssize_t read_bytes = read(clientFd, buff, sizeof(buff) - 1);

      if (read_bytes < 0) {
        throw std::runtime_error("Failed to read the message from the client!");
      }

      cout << "Message from client: " << buff << endl;
      // }
                      



    } catch (std::exception &e) {
      close(socketFd);
      cout << e.what() << endl;
    }


    return 0;
}
