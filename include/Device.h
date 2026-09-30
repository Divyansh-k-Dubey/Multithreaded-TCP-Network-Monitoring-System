#ifndef DEVICE_H
#define DEVICE_H

#include "Telemetry.h"
#include <string>
#include <ctime>

enum class HealthState {
    NORMAL,
    WARNING,
    CRITICAL,
    OFFLINE
};

class Device {
public:
    Device();
    explicit Device(std::string deviceId, std::string ipAddress = "", int port = 0);

    // Getters
    const std::string& getDeviceId() const { return m_deviceId; }
    const std::string& getIpAddress() const { return m_ipAddress; }
    int getPort() const { return m_port; }
    HealthState getHealthState() const { return m_healthState; }
    const Telemetry& getLastTelemetry() const { return m_lastTelemetry; }
    std::time_t getLastUpdateTime() const { return m_lastUpdateTime; }
    bool isConnected() const { return m_isConnected; }

    // Setters / Actions
    void setIpAddress(const std::string& ip) { m_ipAddress = ip; }
    void setPort(int port) { m_port = port; }
    void setHealthState(HealthState state) { m_healthState = state; }
    void updateTelemetry(const Telemetry& telemetry, HealthState calculatedState);
    void setConnected(bool connected);
    
    // Helper to format health state as string
    static std::string healthStateToString(HealthState state);

private:
    std::string m_deviceId;
    std::string m_ipAddress;
    int m_port;
    HealthState m_healthState;
    Telemetry m_lastTelemetry;
    std::time_t m_lastUpdateTime;
    bool m_isConnected;
};

#endif // DEVICE_H
