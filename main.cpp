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

    ~Client() {};
};

int main() {

    sockaddr_in infos;
    int port = PORT;

    infos.sin_family = AF_INET;
    infos.sin_port = htons(port);
    infos.sin_addr.s_addr = INADDR_ANY;

    int socketFd;
    uint64_t infosSize = sizeof(infos);
    int tries = 0;

    while (true) {
      socketFd = socket(infos.sin_family, SOCK_STREAM, 0);

      if (socketFd < 0) {
          cout << "Error creating the socket!" << endl;
          return 1;
      }

      if (bind(socketFd, reinterpret_cast<const struct sockaddr*>(&infos), infosSize) < 0) {
          cerr << "Error binding, changing the port to " << ++port << endl;
          close(socketFd);
          infos.sin_port = htons(port);

          if (tries >= MAX_BIND_TRIES) {
            cerr << "Done retrying" << endl;
            return 1; 

          } else {
            sleep(1);
          }
          
          tries++;
      } else {
        cout << "Connection estabilated at port: " << port << endl;
        break ;
      }
    }

    if (listen(socketFd, SOMAXCONN) < 0) {
        cerr << "Error listening to clients!" << endl;
        return 1;
    }

    cout << "Server is running successfuly on port 8080!" << endl;

    try {

      if (fork() == 0) {
        // create a new client over here
        Client user(8082);
        user.connectToServer(infos, infosSize);

        //now lets try to send something and get it from the server

        return 0;

      } else {

        sockaddr_in client_add;
        size_t client_add_size;

        // now phase to accept the clinet from the server:
        int clientFd = accept(socketFd, reinterpret_cast<struct sockaddr *>(&client_add),
            reinterpret_cast<socklen_t *>(&client_add_size));

        if (clientFd < 0) {
          throw std::runtime_error("Failed to accept the connection of the user!");
        }

        cout << "Client accepted successfully from the server!" << endl;

      }
                      



    } catch (std::exception &e) {
      cout << e.what() << endl;
    }


    return 0;
}
