# Multithreaded TCP Network Monitoring System

## Overview

The Multithreaded TCP Network Monitoring System is a C++ prototype that monitors health telemetry from multiple network devices concurrently. The system consists of a central monitoring server and simulated client devices that transmit operational metrics over TCP/IP sockets.

The central server accepts incoming client connections, parses telemetry strings, tracks device states, evaluates metrics against health thresholds, logs events, and displays monitoring information in the terminal. The project demonstrates core C++17 capabilities, POSIX socket networking, standard thread concurrency, and object-oriented design.

## Features

- **Concurrent TCP Client Handling**: Spawns a dedicated worker thread per connected client socket.
- **Telemetry Parsing**: Parses semicolon-delimited key-value telemetry payloads.
- **Device State Management**: Tracks device status (`NORMAL`, `WARNING`, `CRITICAL`, `OFFLINE`), last update times, and network endpoints.
- **Rule-Based Fault Detection**: Compares metrics against thresholds to identify warning and critical fault conditions.
- **Thread-Safe Logging**: Appends timestamped events and alerts to `logs/network_monitor.log` using standard mutex synchronization.
- **Terminal Monitoring Dashboard**: Renders a formatted health table and recent alert list in the terminal.
- **Timeout and Disconnection Detection**: Identifies closed client sockets and marks inactive devices as offline after a timeout period.
- **Fault Simulation Modes**: Includes simulated client modes to test temperature, signal, packet loss, and CPU fault scenarios.

## Architecture

```
Simulated Network Devices
        |
        | TCP/IP telemetry
        v
Central Monitoring Server
        |
        +-- Client connection handling
        +-- Telemetry parsing
        +-- Fault detection
        +-- Device state management
        +-- Thread-safe logging
        |
        +-- Terminal monitoring output
```

## Telemetry

Client devices send line-delimited key-value strings over TCP sockets using the following format:

```text
DEVICE_ID=<string>;TEMP=<float>;SIGNAL=<float>;PACKET_LOSS=<float>;CPU=<float>
```

### Example Telemetry Payload
```text
DEVICE_ID=DEVICE-01;TEMP=65.2;SIGNAL=82.5;PACKET_LOSS=1.2;CPU=45.0
```

## Fault Detection

The `FaultDetector` module evaluates telemetry metrics against the following rule-based thresholds:

| Metric | Normal | Warning | Critical |
|---|---:|---:|---:|
| Temperature | ≤ 75°C | > 75°C | > 85°C |
| Signal Strength | ≥ 60 | < 60 | < 40 |
| Packet Loss | ≤ 5% | > 5% | > 15% |
| CPU Utilization | ≤ 80% | > 80% | > 95% |

When a metric violates a threshold, an alert is generated and the device's overall state is updated to `WARNING` or `CRITICAL`.

## Project Structure

```text
.
├── CMakeLists.txt         # CMake build configuration
├── Makefile               # Standard Makefile
├── README.md              # Project documentation
├── INTERVIEW_GUIDE.md     # In-depth technical explanation document
├── TESTING.md             # Manual testing procedures
├── include/
│   ├── Device.h           # Device data model and health state enum
│   ├── DeviceManager.h    # Thread-safe device map and UI rendering
│   ├── FaultDetector.h    # Rule-based threshold evaluator and alerts
│   ├── Logger.h           # Thread-safe log file writer
│   ├── NetworkUtils.h     # POSIX TCP socket utilities
│   └── Telemetry.h        # Telemetry parsing and serialization
├── src/
│   ├── Device.cpp
│   ├── DeviceManager.cpp
│   ├── FaultDetector.cpp
│   ├── Logger.cpp
│   ├── NetworkUtils.cpp
│   ├── Telemetry.cpp
│   ├── server.cpp         # Server entry point
│   └── client.cpp         # Simulated client entry point
└── logs/
    └── .gitkeep           # Preserves log directory
```

## Requirements

- C++17 compatible compiler (`g++` or `clang++`)
- POSIX threads library (`pthread`)
- CMake 3.10+ or `make`

## Build

### Option 1: Using CMake
```bash
mkdir -p build
cd build
cmake ..
make
```

### Option 2: Using Makefile
```bash
make
```

This compiles two executables: `network_monitor_server` and `network_monitor_client`.

## Run

### 1. Start Server
```bash
./network_monitor_server [port]
```
If no port is specified, the server defaults to port `8080`.

### 2. Start Simulated Client
```bash
./network_monitor_client [DEVICE_ID] [MODE]
```
Or with full arguments:
```bash
./network_monitor_client [DEVICE_ID] [SERVER_HOST] [PORT] [MODE]
```

### Available Simulation Modes
- `normal`: Generates metrics within healthy operating ranges.
- `fault_temp`: Simulates high operating temperature (> 85°C).
- `fault_signal`: Simulates low signal strength (< 40).
- `fault_loss`: Simulates high packet loss (> 15%).
- `fault_cpu`: Simulates high CPU utilization (> 95%).
- `random`: Generates varying metrics with periodic fault spikes.

### Examples
```bash
# Terminal 1: Start Server
./network_monitor_server 8080

# Terminal 2: Normal Client
./network_monitor_client DEVICE-01 normal

# Terminal 3: Temperature Fault Client
./network_monitor_client DEVICE-02 fault_temp

# Terminal 4: Packet Loss Fault Client
./network_monitor_client DEVICE-03 fault_loss
```

## Example Output

Below is an example of the terminal monitoring dashboard rendered by `DeviceManager`:

```text
=================================================================================
                   MULTITHREADED TCP NETWORK DEVICE MONITOR                      
=================================================================================
Device ID   Status     Temp (°C) Signal    Loss (%)   CPU (%)   Last Update 
---------------------------------------------------------------------------------
DEVICE-01   NORMAL     62.1       85.3      1.2        42.4      20:26:05    
DEVICE-02   CRITICAL   88.7       84.1      1.3        42.0      20:26:05    
DEVICE-03   CRITICAL   61.4       84.4      18.4       41.7      20:26:06    
---------------------------------------------------------------------------------
Recent Alerts & Fault Notifications:
 [ALERT] DEVICE-02 | HIGH TEMPERATURE | 88.7 C
 [ALERT] DEVICE-03 | HIGH PACKET LOSS | 18.4%
=================================================================================
 Press Ctrl+C on Server to cleanly shutdown.
```

## Multithreading

The server uses standard C++ concurrency (`std::thread`, `std::mutex`, `std::lock_guard`):

1. **Main Thread**: Executes a blocking `accept()` loop on the listening TCP socket to accept client connections.
2. **Worker Threads**: Each accepted client connection is handled by a dedicated worker thread running `handleClientConnection`. The thread receives telemetry lines over the socket, parses data, evaluates metrics, and updates device state.
3. **Dashboard Thread**: A background thread runs `dashboardThreadFunc`, refreshing the terminal dashboard and checking for device timeouts every 2 seconds.
4. **Synchronization**: Access to shared device records in `DeviceManager` and shared file output in `Logger` is synchronized using `std::mutex` and `std::lock_guard`.

## Limitations

- **Simulated Environment**: Uses software client processes generating simulated metrics rather than physical network hardware.
- **In-Memory State**: Device states and recent alerts are stored in memory and reset when the server stops.
- **Thread-per-Client Concurrency**: Uses one thread per client socket, which is simple and readable but not intended for large-scale production workloads.
- **Terminal Interface**: Output is presented via ANSI terminal rendering without a web interface or graphical frontend.

## Technologies

- **Language**: C++17
- **Networking**: POSIX TCP Sockets (`sys/socket.h`, `netinet/in.h`)
- **Concurrency**: C++ Standard Threads (`std::thread`, `std::mutex`, `std::lock_guard`)
- **Design**: Object-Oriented Programming (OOP)
- **Build System**: CMake / Make

## Disclaimer

This repository is an educational prototype built to demonstrate C++ systems programming, socket networking, multithreading, and network device monitoring concepts.
