#include "NetworkUtils.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>

int NetworkUtils::createServerSocket(int port, int backlog) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        std::cerr << "[Network Error] Failed to create socket: " << strerror(errno) << std::endl;
        return -1;
    }

    int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "[Network Error] setsockopt SO_REUSEADDR failed: " << strerror(errno) << std::endl;
        close(serverFd);
        return -1;
    }

    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[Network Error] Bind failed on port " << port << ": " << strerror(errno) << std::endl;
        close(serverFd);
        return -1;
    }

    if (listen(serverFd, backlog) < 0) {
        std::cerr << "[Network Error] Listen failed: " << strerror(errno) << std::endl;
        close(serverFd);
        return -1;
    }

    return serverFd;
}

int NetworkUtils::connectToServer(const std::string& host, int port) {
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd < 0) {
        std::cerr << "[Network Error] Socket creation failed: " << strerror(errno) << std::endl;
        return -1;
    }

    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "[Network Error] Invalid address / Address not supported: " << host << std::endl;
        close(clientFd);
        return -1;
    }

    if (connect(clientFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[Network Error] Connection to " << host << ":" << port << " failed: " << strerror(errno) << std::endl;
        close(clientFd);
        return -1;
    }

    return clientFd;
}

bool NetworkUtils::sendMessage(int socketFd, const std::string& message) {
    std::string formattedMsg = message;
    if (formattedMsg.empty() || formattedMsg.back() != '\n') {
        formattedMsg += "\n";
    }

    size_t totalSent = 0;
    size_t length = formattedMsg.length();
    const char* buffer = formattedMsg.c_str();

    while (totalSent < length) {
        ssize_t sent = send(socketFd, buffer + totalSent, length - totalSent, MSG_NOSIGNAL);
        if (sent <= 0) {
            return false;
        }
        totalSent += sent;
    }
    return true;
}

bool NetworkUtils::readMessage(int socketFd, std::string& outMessage) {
    outMessage.clear();
    char ch;

    while (true) {
        ssize_t bytesRead = recv(socketFd, &ch, 1, 0);
        if (bytesRead <= 0) {
            // Connection closed or socket error
            return false;
        }

        if (ch == '\n') {
            break;
        }
        if (ch != '\r') {
            outMessage += ch;
        }
    }
    return true;
}

bool NetworkUtils::setSocketTimeout(int socketFd, int seconds) {
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    if (setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
        return false;
    }
    return true;
}

void NetworkUtils::closeSocket(int socketFd) {
    if (socketFd >= 0) {
        shutdown(socketFd, SHUT_RDWR);
        close(socketFd);
    }
}
