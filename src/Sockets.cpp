#include "Sockets.hpp"
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>

Sockets::Sockets() : sockfd(-1) {
    std::memset(&addr, 0, sizeof(addr));
}

bool Sockets::connectToServer(const std::string &ip, int port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        perror("inet_pton");
        return false;
    }

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return false;
    }

    return true;
}

bool Sockets::bindAndListen(int port, int backlog) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return false;
    }

    if (listen(sockfd, backlog) < 0) {
        perror("listen");
        return false;
    }

    return true;
}

int Sockets::acceptClient() {
    socklen_t len = sizeof(addr);
    int client_fd = accept(sockfd, (struct sockaddr *)&addr, &len);
    if (client_fd < 0) {
        perror("accept");
    }
    return client_fd;
}

bool Sockets::sendVector(const std::vector<int> &vec, int fd) {
    int use_fd = (fd == -1) ? sockfd : fd;
    int size = vec.size();
    if (send(use_fd, &size, sizeof(size), 0) != sizeof(size)) return false;
    if (size == 0) return true;
    return send(use_fd, vec.data(), size * sizeof(int), 0) == size * sizeof(int);
}

bool Sockets::receiveVector(std::vector<int> &vec, int fd) {
    int use_fd = (fd == -1) ? sockfd : fd;
    int size = 0;
    if (recv(use_fd, &size, sizeof(size), MSG_WAITALL) != sizeof(size)) return false;
    if (size < 0 || size > 1000000) return false; // Límite arbitrario
    vec.resize(size);
    if (size == 0) return true;
    return recv(use_fd, vec.data(), size * sizeof(int), MSG_WAITALL) == size * sizeof(int);
}

bool Sockets::sendStringAndInt(const std::string &str, int num, int fd) {
    int use_fd = (fd == -1) ? sockfd : fd;
    int len = str.length();
    if (send(use_fd, &len, sizeof(len), 0) != sizeof(len)) return false;
    if (!str.empty() && send(use_fd, str.c_str(), len, 0) != len) return false;
    return send(use_fd, &num, sizeof(num), 0) == sizeof(num);
}

bool Sockets::receiveStringAndInt(std::string &str, int &num, int fd) {
    int use_fd = (fd == -1) ? sockfd : fd;
    int len = 0;
    if (recv(use_fd, &len, sizeof(len), MSG_WAITALL) != sizeof(len)) return false;
    if (len < 0 || len > 1000000) return false;

    std::vector<char> buffer(len);
    if (len > 0 && recv(use_fd, buffer.data(), len, MSG_WAITALL) != len) return false;
    str.assign(buffer.begin(), buffer.end());

    return recv(use_fd, &num, sizeof(num), MSG_WAITALL) == sizeof(num);
}

void Sockets::closeSocket() {
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
}

int Sockets::getSocket() const {
    return sockfd;
}

Sockets::~Sockets() {
    closeSocket();
}
