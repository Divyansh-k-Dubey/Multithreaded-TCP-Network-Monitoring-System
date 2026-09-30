# Manual Testing Procedures & Verification Guide

This document outlines manual test cases used to verify the functionality, concurrency, fault detection, and robustness of the **Multithreaded TCP Network Monitoring System**.

---

## Prerequisites & Setup

Ensure the project is built cleanly before starting tests:

```bash
make clean && make
```

Confirm that the binaries `network_monitor_server` and `network_monitor_client` are generated.

---

## Test Cases

### Test Case 1: Server Startup & Port Binding
- **Objective**: Verify that the central monitoring server initializes socket binding and begins listening on the configured port.
- **Command**:
  ```bash
  ./network_monitor_server 8080
  ```
- **Expected Outcome**:
  - Server starts without socket errors.
  - Dashboard appears on terminal.
  - Log entry added to `logs/network_monitor.log`:
    `[INFO] Starting Centralized TCP Network Monitoring Server on port 8080`

---

### Test Case 2: Single Client Connection & Telemetry Ingestion
- **Objective**: Verify that a client can connect over TCP, transmit telemetry, and update server state.
- **Command** (Terminal 2):
  ```bash
  ./network_monitor_client DEVICE-01 127.0.0.1 8080 normal
  ```
- **Expected Outcome**:
  - Server accepts connection and registers `DEVICE-01`.
  - Terminal table shows `DEVICE-01` in state `NORMAL`.
  - Metrics update every 2 seconds.

---

### Test Case 3: Concurrent Multi-Client Telemetry Processing
- **Objective**: Verify that multiple clients transmit telemetry concurrently on separate worker threads without data corruption or state collision.
- **Commands** (Separate Terminals):
  ```bash
  ./network_monitor_client DEVICE-01 127.0.0.1 8080 normal
  ./network_monitor_client DEVICE-02 127.0.0.1 8080 normal
  ./network_monitor_client DEVICE-03 127.0.0.1 8080 normal
  ./network_monitor_client DEVICE-04 127.0.0.1 8080 normal
  ```
- **Expected Outcome**:
  - All 4 devices appear in the server's dashboard table concurrently.
  - Telemetry updates independently for each device.

---

### Test Case 4: High Temperature Fault Alert Detection
- **Objective**: Verify that telemetry exceeding temperature warning/critical threshold (>85°C) triggers critical alerts.
- **Command**:
  ```bash
  ./network_monitor_client DEVICE-02 127.0.0.1 8080 fault_temp
  ```
- **Expected Outcome**:
  - Server marks `DEVICE-02` status as `CRITICAL`.
  - Alert displayed on terminal:
    `[ALERT] DEVICE-02 | HIGH TEMPERATURE | 88.5 C`
  - Critical entry recorded in `logs/network_monitor.log`.

---

### Test Case 5: High Packet Loss Fault Alert Detection
- **Objective**: Verify packet loss fault detection (>15%).
- **Command**:
  ```bash
  ./network_monitor_client DEVICE-03 127.0.0.1 8080 fault_loss
  ```
- **Expected Outcome**:
  - Server marks `DEVICE-03` status as `CRITICAL`.
  - Alert displayed:
    `[ALERT] DEVICE-03 | HIGH PACKET LOSS | 18.9%`

---

### Test Case 6: Low Signal Strength Fault Alert Detection
- **Objective**: Verify low signal strength fault detection (<40).
- **Command**:
  ```bash
  ./network_monitor_client DEVICE-04 127.0.0.1 8080 fault_signal
  ```
- **Expected Outcome**:
  - Server marks `DEVICE-04` status as `CRITICAL`.
  - Alert displayed:
    `[ALERT] DEVICE-04 | LOW SIGNAL | 31.2`

---

### Test Case 7: Graceful Client Disconnection
- **Objective**: Verify server detects client socket disconnect and updates device status.
- **Action**:
  1. Start `DEVICE-01` client.
  2. Terminate client process (`Ctrl+C` or `kill`).
- **Expected Outcome**:
  - Server detects socket close (`recv()` returns 0).
  - Server logs: `[WARNING] Device connection closed/lost: DEVICE-01`.
  - Device status transitions to `OFFLINE` on dashboard.
  - Server thread exits cleanly without crash or resource leak.

---

### Test Case 8: Thread-Safe Log File Audit
- **Objective**: Confirm log output is formatted and synchronized across threads.
- **Command**:
  ```bash
  cat logs/network_monitor.log
  ```
- **Expected Outcome**:
  - Log entries are timestamped.
  - Log entries contain levels `[INFO]`, `[WARNING]`, `[CRITICAL]`, `[ERROR]`.
  - No scrambled or overlapping text lines from concurrent threads.
