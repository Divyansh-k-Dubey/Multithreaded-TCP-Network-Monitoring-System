#include "Telemetry.h"
#include "NetworkUtils.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <csignal>
#include <atomic>
#include <algorithm>

static std::atomic<bool> g_running(true);

void signalHandler(int signum) {
    (void)signum;
    g_running = false;
}

bool isKnownMode(const std::string& arg) {
    return (arg == "normal" || arg == "fault_temp" || arg == "fault_signal" ||
            arg == "fault_loss" || arg == "fault_cpu" || arg == "random");
}

int main(int argc, char* argv[]) {
    std::string deviceId = "DEVICE-01";
    std::string serverHost = "127.0.0.1";
    int serverPort = 8080;
    std::string mode = "normal";

    // Flexible CLI parsing:
    // Formats supported:
    // 1. ./network_monitor_client DEVICE-01
    // 2. ./network_monitor_client DEVICE-01 fault_temp
    // 3. ./network_monitor_client DEVICE-01 127.0.0.1 8080 fault_temp
    if (argc > 1) {
        deviceId = argv[1];
    }

    if (argc == 2) {
        // Only device ID provided
    } else if (argc == 3) {
        std::string arg2 = argv[2];
        if (isKnownMode(arg2)) {
            mode = arg2;
        } else {
            serverHost = arg2;
        }
    } else if (argc >= 4) {
        std::string arg2 = argv[2];
        if (isKnownMode(arg2)) {
            mode = arg2;
        } else {
            serverHost = arg2;
            try { serverPort = std::stoi(argv[3]); } catch (...) {}
            if (argc > 4) {
                mode = argv[4];
            }
        }
    }

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "========================================================\n";
    std::cout << "       SIMULATED NETWORK DEVICE (TCP CLIENT)            \n";
    std::cout << "========================================================\n";
    std::cout << " Device ID  : " << deviceId << "\n";
    std::cout << " Server     : " << serverHost << ":" << serverPort << "\n";
    std::cout << " Mode       : " << mode << "\n";
    std::cout << " Status     : Connecting to server...\n";
    std::cout << "========================================================\n";

    int socketFd = NetworkUtils::connectToServer(serverHost, serverPort);
    if (socketFd < 0) {
        std::cerr << "\n[Client Error] Could not connect to monitoring server at " 
                  << serverHost << ":" << serverPort << std::endl;
        std::cerr << "[Client Error] Make sure './network_monitor_server " << serverPort 
                  << "' is running first in Terminal 1!\n";
        return 1;
    }

    std::cout << "[Client Info] Connected to monitoring server successfully.\n";
    std::cout << "[Client Info] Streaming simulated telemetry every 2 seconds. Press Ctrl+C to stop.\n\n";

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> distNoise(-1.5, 1.5);
    std::uniform_real_distribution<double> distLossNoise(-0.3, 0.3);

    int count = 0;

    while (g_running) {
        double temp = 62.0 + distNoise(rng);
        double signal = 85.0 + distNoise(rng);
        double loss = 1.2 + distLossNoise(rng);
        double cpu = 42.0 + distNoise(rng);

        if (loss < 0.0) loss = 0.0;

        // Apply fault behavior based on mode or loop count
        if (mode == "fault_temp") {
            temp = 88.5 + distNoise(rng); // Critical temp threshold > 85
        } else if (mode == "fault_signal") {
            signal = 32.5 + distNoise(rng); // Critical signal threshold < 40
        } else if (mode == "fault_loss") {
            loss = 18.7 + distLossNoise(rng); // Critical packet loss > 15
        } else if (mode == "fault_cpu") {
            cpu = 96.8 + distNoise(rng); // Critical CPU threshold > 95
        } else if (mode == "random") {
            count++;
            if (count % 5 == 0) {
                temp = 89.2;
                loss = 16.4;
            }
        }

        Telemetry telemetry(deviceId, temp, signal, loss, cpu);
        std::string payload = telemetry.serialize();

        std::cout << "[Tx Telemetry] " << payload << std::endl;

        if (!NetworkUtils::sendMessage(socketFd, payload)) {
            std::cerr << "[Client Error] Server closed connection or send failed.\n";
            break;
        }

        // Sleep for 2 seconds before next telemetry transmission
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    std::cout << "\n[Client Info] Disconnecting client " << deviceId << "...\n";
    NetworkUtils::closeSocket(socketFd);
    std::cout << "[Client Info] Disconnected cleanly.\n";

    return 0;
}
