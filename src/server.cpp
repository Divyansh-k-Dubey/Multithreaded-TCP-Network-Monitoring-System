#include "Logger.h"
#include "Telemetry.h"
#include "Device.h"
#include "FaultDetector.h"
#include "DeviceManager.h"
#include "NetworkUtils.h"

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

static std::atomic<bool> g_running(true);

void signalHandler(int signum) {
    (void)signum;
    g_running = false;
}

void handleClientConnection(int clientFd, std::string clientIp, int clientPort,
                            DeviceManager& devManager,
                            const FaultDetector& faultDetector) {
    Logger::info("Accepted new TCP connection from " + clientIp + ":" + std::to_string(clientPort));

    // Set 15s socket receive timeout so worker thread isn't blocked forever if client freezes
    NetworkUtils::setSocketTimeout(clientFd, 15);

    std::string deviceId = "UNKNOWN";
    std::string messageBuffer;

    while (g_running) {
        bool success = NetworkUtils::readMessage(clientFd, messageBuffer);
        if (!success) {
            // Client disconnected or socket error/timeout occurred
            break;
        }

        if (messageBuffer.empty()) {
            continue;
        }

        auto telemetryOpt = Telemetry::parse(messageBuffer);
        if (telemetryOpt.has_value()) {
            const Telemetry& telemetry = telemetryOpt.value();
            deviceId = telemetry.getDeviceId();

            HealthState overallState = HealthState::NORMAL;
            std::vector<Alert> alerts = faultDetector.evaluate(telemetry, overallState);

            devManager.updateDeviceTelemetry(clientIp, clientPort, telemetry, alerts, overallState);
        } else {
            Logger::error("Malformed telemetry string received from " + clientIp + ":" + 
                          std::to_string(clientPort) + ": '" + messageBuffer + "'");
        }
    }

    if (deviceId != "UNKNOWN") {
        devManager.setDeviceOffline(deviceId);
    }

    NetworkUtils::closeSocket(clientFd);
    Logger::info("Closed connection for client " + deviceId + " (" + clientIp + ":" + std::to_string(clientPort) + ")");
}

void dashboardThreadFunc(DeviceManager& devManager) {
    while (g_running) {
        devManager.checkTimeouts(10);
        devManager.renderDashboard();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            std::cerr << "Usage: " << argv[0] << " [port]" << std::endl;
            return 1;
        }
    }

    // Register signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Initialize Logger
    Logger::init("logs/network_monitor.log");
    Logger::info("Starting Centralized TCP Network Monitoring Server on port " + std::to_string(port));

    int serverFd = NetworkUtils::createServerSocket(port);
    if (serverFd < 0) {
        Logger::error("Failed to start monitoring server socket on port " + std::to_string(port));
        return 1;
    }

    // Set 1-second timeout on server socket so accept() unblocks periodically to check g_running
    NetworkUtils::setSocketTimeout(serverFd, 1);

    DeviceManager devManager;
    FaultDetector faultDetector;

    // Start background dashboard refresh thread
    std::thread uiThread(dashboardThreadFunc, std::ref(devManager));

    std::vector<std::thread> clientThreads;

    while (g_running) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        int clientFd = accept(serverFd, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientFd >= 0) {
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clientAddr.sin_addr), ipStr, INET_ADDRSTRLEN);
            int clientPort = ntohs(clientAddr.sin_port);

            // Spawn worker thread for client
            clientThreads.emplace_back(handleClientConnection, clientFd, std::string(ipStr),
                                       clientPort, std::ref(devManager), std::cref(faultDetector));
        }
    }

    Logger::info("Shutting down Central Monitoring Server...");

    // Close listening socket
    NetworkUtils::closeSocket(serverFd);

    // Wait for dashboard thread
    if (uiThread.joinable()) {
        uiThread.join();
    }

    // Join all client worker threads
    for (auto& t : clientThreads) {
        if (t.joinable()) {
            t.join();
        }
    }

    Logger::info("Server shutdown complete.");
    std::cout << "\nServer stopped gracefully.\n";

    return 0;
}
