#include "Device.h"

Device::Device()
    : m_deviceId("UNKNOWN"),
      m_ipAddress(""),
      m_port(0),
      m_healthState(HealthState::OFFLINE),
      m_lastUpdateTime(0),
      m_isConnected(false) {}

Device::Device(std::string deviceId, std::string ipAddress, int port)
    : m_deviceId(std::move(deviceId)),
      m_ipAddress(std::move(ipAddress)),
      m_port(port),
      m_healthState(HealthState::NORMAL),
      m_lastUpdateTime(std::time(nullptr)),
      m_isConnected(true) {}

void Device::updateTelemetry(const Telemetry& telemetry, HealthState calculatedState) {
    m_lastTelemetry = telemetry;
    m_healthState = calculatedState;
    m_lastUpdateTime = std::time(nullptr);
    m_isConnected = true;
}

void Device::setConnected(bool connected) {
    m_isConnected = connected;
    if (!connected) {
        m_healthState = HealthState::OFFLINE;
    }
}

std::string Device::healthStateToString(HealthState state) {
    switch (state) {
        case HealthState::NORMAL:   return "NORMAL";
        case HealthState::WARNING:  return "WARNING";
        case HealthState::CRITICAL: return "CRITICAL";
        case HealthState::OFFLINE:  return "OFFLINE";
        default:                    return "UNKNOWN";
    }
}
