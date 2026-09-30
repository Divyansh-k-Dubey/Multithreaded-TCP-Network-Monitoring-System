#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <string>

class NetworkUtils {
public:
    // Create, bind, and listen on a TCP server socket
    static int createServerSocket(int port, int backlog = 10);

    // Connect to a remote TCP server
    static int connectToServer(const std::string& host, int port);

    // Send line-delimited message over TCP socket
    static bool sendMessage(int socketFd, const std::string& message);

    // Read single line-delimited message from TCP socket
    static bool readMessage(int socketFd, std::string& outMessage);

    // Set receive timeout for socket operations
    static bool setSocketTimeout(int socketFd, int seconds);

    // Close socket descriptor cleanly
    static void closeSocket(int socketFd);
};

#endif // NETWORKUTILS_H
