#ifndef SOCKETS_HPP
#define SOCKETS_HPP

#include <string>
#include <vector>
#include <netinet/in.h>

class Sockets {
    int sockfd;
    struct sockaddr_in addr;

public:
    Sockets();

    bool connectToServer(const std::string &ip, int port);
    bool bindAndListen(int port, int backlog = 1);
    int acceptClient(std::string* out_ip = nullptr, int* out_port = nullptr);

    bool sendVector(const std::vector<int> &vec, int fd = -1);
    bool receiveVector(std::vector<int> &vec, int fd = -1);

    bool sendStringAndInt(const std::string &str, int num, int fd = -1);
    bool receiveStringAndInt(std::string &str, int &num, int fd = -1);

    void closeSocket();
    int getSocket() const;

    ~Sockets();
};

#endif // SOCKETS_HPP
