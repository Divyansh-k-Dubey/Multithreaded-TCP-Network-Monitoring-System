#include "DeviceManager.h"
#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>

DeviceManager::DeviceManager() {}

void DeviceManager::updateDeviceTelemetry(const std::string& ipAddress, int port,
                                         const Telemetry& telemetry,
                                         const std::vector<Alert>& newAlerts,
                                         HealthState calculatedState) {
    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string& id = telemetry.getDeviceId();
    auto it = m_devices.find(id);
    if (it == m_devices.end()) {
        Device dev(id, ipAddress, port);
        dev.updateTelemetry(telemetry, calculatedState);
        m_devices[id] = dev;
        Logger::info("Registered new device: " + id + " (" + ipAddress + ":" + std::to_string(port) + ")");
    } else {
        it->second.setIpAddress(ipAddress);
        it->second.setPort(port);
        it->second.updateTelemetry(telemetry, calculatedState);
    }

    // Add new alerts to recent alerts list
    for (const auto& alert : newAlerts) {
        m_recentAlerts.push_back(alert);
        // Log alert to file
        if (alert.severity == LogLevel::CRITICAL) {
            Logger::critical(alert.message);
        } else {
            Logger::warning(alert.message);
        }
    }

    // Trim recent alerts list to MAX_RECENT_ALERTS
    while (m_recentAlerts.size() > MAX_RECENT_ALERTS) {
        m_recentAlerts.erase(m_recentAlerts.begin());
    }
}

void DeviceManager::setDeviceOffline(const std::string& deviceId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_devices.find(deviceId);
    if (it != m_devices.end()) {
        it->second.setConnected(false);
        Logger::warning("Device connection closed/lost: " + deviceId);
        
        std::time_t now = std::time(nullptr);
        Alert offlineAlert{deviceId, LogLevel::WARNING, "DISCONNECT", 0.0,
                           "[ALERT] " + deviceId + " | DEVICE OFFLINE / DISCONNECTED", now};
        m_recentAlerts.push_back(offlineAlert);
        if (m_recentAlerts.size() > MAX_RECENT_ALERTS) {
            m_recentAlerts.erase(m_recentAlerts.begin());
        }
    }
}

void DeviceManager::checkTimeouts(int timeoutSeconds) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::time_t now = std::time(nullptr);

    for (auto& pair : m_devices) {
        Device& dev = pair.second;
        if (dev.isConnected() && (now - dev.getLastUpdateTime()) > timeoutSeconds) {
            dev.setConnected(false);
            Logger::warning("Device timeout detected (no telemetry for " +
                           std::to_string(timeoutSeconds) + "s): " + dev.getDeviceId());
            
            Alert timeoutAlert{dev.getDeviceId(), LogLevel::CRITICAL, "TIMEOUT", 0.0,
                               "[ALERT] " + dev.getDeviceId() + " | DEVICE TIMEOUT (NO HEARTBEAT)", now};
            m_recentAlerts.push_back(timeoutAlert);
            if (m_recentAlerts.size() > MAX_RECENT_ALERTS) {
                m_recentAlerts.erase(m_recentAlerts.begin());
            }
        }
    }
}

std::vector<Device> DeviceManager::getAllDevices() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Device> result;
    result.reserve(m_devices.size());
    for (const auto& pair : m_devices) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<Alert> DeviceManager::getRecentAlerts() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_recentAlerts;
}

void DeviceManager::renderDashboard() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    // ANSI escape codes for clear screen and home cursor
    std::cout << "\033[2J\033[1;1H";

    std::cout << "=================================================================================\n";
    std::cout << "                   MULTITHREADED TCP NETWORK DEVICE MONITOR                      \n";
    std::cout << "=================================================================================\n";
    std::cout << std::left 
              << std::setw(12) << "Device ID"
              << std::setw(11) << "Status"
              << std::setw(11) << "Temp (°C)"
              << std::setw(10) << "Signal"
              << std::setw(11) << "Loss (%)"
              << std::setw(10) << "CPU (%)"
              << std::setw(12) << "Last Update"
              << "\n";
    std::cout << "---------------------------------------------------------------------------------\n";

    if (m_devices.empty()) {
        std::cout << " [No active or registered devices connected. Waiting for telemetry...]\n";
    } else {
        for (const auto& pair : m_devices) {
            const Device& dev = pair.second;
            const Telemetry& t = dev.getLastTelemetry();

            std::string timeStr = "N/A";
            if (dev.getLastUpdateTime() > 0) {
                std::time_t ut = dev.getLastUpdateTime();
                struct tm tm_buf;
                localtime_r(&ut, &tm_buf);
                char buf[16];
                std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm_buf);
                timeStr = buf;
            }

            std::string statusStr = Device::healthStateToString(dev.getHealthState());

            std::cout << std::left 
                      << std::setw(12) << dev.getDeviceId()
                      << std::setw(11) << statusStr;
            
            if (dev.getHealthState() == HealthState::OFFLINE && t.getDeviceId() == "UNKNOWN") {
                std::cout << std::setw(11) << "-"
                          << std::setw(10) << "-"
                          << std::setw(11) << "-"
                          << std::setw(10) << "-"
                          << std::setw(12) << timeStr << "\n";
            } else {
                std::cout << std::fixed << std::setprecision(1)
                          << std::setw(11) << t.getTemperature()
                          << std::setw(10) << t.getSignalStrength()
                          << std::setw(11) << t.getPacketLoss()
                          << std::setw(10) << t.getCpuUtilization()
                          << std::setw(12) << timeStr << "\n";
            }
        }
    }

    std::cout << "---------------------------------------------------------------------------------\n";
    std::cout << "Recent Alerts & Fault Notifications:\n";
    if (m_recentAlerts.empty()) {
        std::cout << " [System Healthy - No active alert events]\n";
    } else {
        for (auto it = m_recentAlerts.rbegin(); it != m_recentAlerts.rend(); ++it) {
            std::cout << " " << it->message << "\n";
        }
    }
    std::cout << "=================================================================================\n";
    std::cout << " Press Ctrl+C on Server to cleanly shutdown.\n" << std::flush;
}
