# Multithreaded TCP Network Monitoring System

A clean, modular C++17 client-server prototype that monitors simulated network devices concurrently over TCP/IP sockets. The central monitoring server receives real-time telemetry from multiple devices, parses metrics, maintains device state, checks threshold limits to trigger fault alerts, and logs events in a thread-safe manner.

> **Note**: This is an educational network health monitoring simulation prototype designed to demonstrate core C++, multithreading, socket programming, and software engineering principles for technical interviews (e.g., Nokia Associate Engineer).

---

## Architecture Overview

```
                      +-------------------+
                      |   DEVICE-01       | (TCP Client)
                      +---------+---------+
                                |
                                | TCP/IP Socket (Telemetry Stream)
                                v
+-------------------------------------------------------------------+
|                  CENTRAL MONITORING SERVER                        |
|                                                                   |
|   +-----------------------+     +-----------------------------+   |
|   |  Main Server Thread   |     | Worker Thread per Client    |   |
|   | (accept() loop)       |---->| (recv, parse telemetry)     |   |
|   +-----------------------+     +--------------+--------------+   |
|                                                |                  |
|                                                v                  |
|   +-----------------------------------------------------------+   |
|   |                      FaultDetector                        |   |
|   | (evaluates temp, signal, packet loss, CPU thresholds)     |   |
|   +----------------------------+------------------------------+   |
|                                |                                  |
|                                v                                  |
|   +-----------------------------------------------------------+   |
|   |                      DeviceManager                        |   |
|   |    (Thread-safe std::mutex state & alert management)      |   |
|   +----------------------------+------------------------------+   |
|                                |                                  |
|            +-------------------+-------------------+              |
|            |                                       |              |
|            v                                       v              |
|  +-------------------+                   +-------------------+    |
|  |   Terminal UI     |                   | Thread-Safe Logger|    |
|  |  (Live Dashboard) |                   |  (File & Console) |    |
|  +-------------------+                   +-------------------+    |
+-------------------------------------------------------------------+
```

---

## Features

- **Concurrent Multithreaded Processing**: Dedicated worker thread spawned per connected client to process telemetry concurrently without blocking the main server loop.
- **Real-Time Telemetry Parsing**: Custom string parser for semi-colon delimited telemetry metrics (`DEVICE_ID`, `TEMP`, `SIGNAL`, `PACKET_LOSS`, `CPU`).
- **Health State Management**: Maintains real-time health state (`NORMAL`, `WARNING`, `CRITICAL`, `OFFLINE`) for every connected device.
- **Automated Fault Detection**: Automatically evaluates telemetry metrics against health threshold boundaries and generates critical/warning alerts.
- **Thread-Safe Logging**: Synchronized thread-safe logger writing timestamped events to `logs/network_monitor.log`.
- **Live Terminal Dashboard**: Dynamic terminal dashboard displaying a health status table and recent alert history.
- **Disconnect & Timeout Handling**: Detects socket closures and heartbeat timeouts, marking inactive devices as `OFFLINE`.

---

## Telemetry Format

Simulated network devices send line-delimited key-value telemetry payloads:

```text
DEVICE_ID=DEVICE-01;TEMP=65.2;SIGNAL=82.5;PACKET_LOSS=1.2;CPU=45.0
```

### Metrics Tracked
| Metric | Description | Unit / Scale |
| :--- | :--- | :--- |
| `DEVICE_ID` | Unique device identifier | String (e.g., `DEVICE-01`) |
| `TEMP` | Operating Temperature | °C |
| `SIGNAL` | Cellular / RF Signal Strength | 0 - 100 Scale |
| `PACKET_LOSS` | Network Transmission Loss | % |
| `CPU` | CPU Core Utilization | % |

---

## Health Thresholds

The `FaultDetector` module checks telemetry values against configured thresholds:

| Metric | Normal Range | Warning Threshold | Critical Threshold |
| :--- | :--- | :--- | :--- |
| **Temperature** | $\le 75.0^\circ\text{C}$ | $> 75.0^\circ\text{C}$ | $> 85.0^\circ\text{C}$ |
| **Signal Strength** | $\ge 60.0$ | $< 60.0$ | $< 40.0$ |
| **Packet Loss** | $\le 5.0\%$ | $> 5.0\%$ | $> 15.0\%$ |
| **CPU Utilization**| $\le 80.0\%$ | $> 80.0\%$ | $> 95.0\%$ |

---

## Project Structure

```text
Multithreaded-TCP-Network-Monitoring-System/
├── CMakeLists.txt         # CMake build configuration
├── Makefile               # Standard Makefile fallback
├── README.md              # Project overview & documentation
├── INTERVIEW_GUIDE.md     # In-depth technical interview guide
├── TESTING.md             # Manual test suite verification
├── .gitignore             # Git ignore patterns
├── include/
│   ├── Device.h           # Device state & health representation
│   ├── DeviceManager.h    # Thread-safe device map & UI renderer
│   ├── FaultDetector.h    # Rule-based threshold evaluator & alerts
│   ├── Logger.h           # Thread-safe file & console logging
│   ├── NetworkUtils.h     # POSIX socket wrapper utility functions
│   └── Telemetry.h        # Telemetry data model & parsing logic
├── src/
│   ├── Device.cpp
│   ├── DeviceManager.cpp
│   ├── FaultDetector.cpp
│   ├── Logger.cpp
│   ├── NetworkUtils.cpp
│   ├── Telemetry.cpp
│   ├── server.cpp         # Central monitoring server executable main
│   └── client.cpp         # Simulated network device executable main
└── logs/
    └── .gitkeep           # Preserves log directory
```

---

## Build Instructions

### Prerequisites
- C++17 compatible compiler (`g++` or `clang++`)
- POSIX threads (`pthread`)
- `make` or `cmake`

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

This compiles two executable binaries:
- `network_monitor_server`
- `network_monitor_client`

---

## Run Instructions

### Step 1: Start Central Server
In Terminal 1:
```bash
./network_monitor_server 8080
```

### Step 2: Start Simulated Device Clients
In separate terminal windows, start device clients:

**Normal Device:**
```bash
./network_monitor_client DEVICE-01 127.0.0.1 8080 normal
```

**Fault Simulation Devices:**
```bash
./network_monitor_client DEVICE-02 127.0.0.1 8080 fault_temp
./network_monitor_client DEVICE-03 127.0.0.1 8080 fault_loss
./network_monitor_client DEVICE-04 127.0.0.1 8080 fault_signal
```

---

## Example Terminal Monitoring Output

```text
=================================================================================
                   MULTITHREADED TCP NETWORK DEVICE MONITOR                      
=================================================================================
Device ID   Status     Temp (°C) Signal    Loss (%)   CPU (%)   Last Update 
---------------------------------------------------------------------------------
DEVICE-01   NORMAL     62.1       85.3      1.2        42.4      19:43:46    
DEVICE-02   CRITICAL   88.7       84.1      1.3        42.0      19:43:42    
DEVICE-03   CRITICAL   61.4       84.4      18.4       41.7      19:43:46    
DEVICE-04   CRITICAL   62.1       32.5      1.3        42.4      19:43:46    
---------------------------------------------------------------------------------
Recent Alerts & Fault Notifications:
 [ALERT] DEVICE-02 | HIGH TEMPERATURE | 88.7 C
 [ALERT] DEVICE-03 | HIGH PACKET LOSS | 18.4%
 [ALERT] DEVICE-04 | LOW SIGNAL | 32.5
=================================================================================
 Press Ctrl+C on Server to cleanly shutdown.
```

---

## How Multithreading Works

1. **Main Server Thread**: Listens on TCP port 8080 using `accept()`. When a client connects, it spawns a new `std::thread(handleClientConnection, ...)`.
2. **Worker Threads**: Each client connection runs in its own worker thread. It reads incoming telemetry lines over the socket using POSIX `recv()`, parses metrics, evaluates health thresholds, and updates shared device state.
3. **UI & Timeout Thread**: A background thread refreshes the terminal dashboard every 2 seconds and checks for device timeouts.
4. **Synchronization**: All access to the central `DeviceManager` state map and file logger is synchronized using `std::mutex` and `std::lock_guard` to prevent race conditions.

---

## Limitations & Future Improvements

- **Connection Architecture**: Currently uses one thread per client socket connection. For thousands of concurrent connections, an event-driven non-blocking I/O multiplexing model (`epoll` / `kqueue`) would be more scalable.
- **Protocol**: Uses simple text telemetry. Production systems typically utilize binary protocols like Protocol Buffers or binary TLV frames.
- **Persistence**: State is stored in-memory. Persistence to disk/database could be added for historical trends.

---

## Resume Description Verification

This project directly demonstrates the following resume description:

> *"Developed a multithreaded C++ client-server system using TCP/IP sockets to concurrently collect and process telemetry from simulated network devices, monitoring their health and connection status in real time. Designed modular OOP components for telemetry parsing, device management, fault detection, and thread-safe logging to automatically identify network issues such as high packet loss, low signal strength, and excessive temperature."*
