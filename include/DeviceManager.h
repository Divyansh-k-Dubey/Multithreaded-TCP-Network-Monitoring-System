#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "Device.h"
#include "FaultDetector.h"
#include <map>
#include <vector>
#include <mutex>
#include <string>

class DeviceManager {
public:
    DeviceManager();

    // Thread-safe update of device state and telemetry
    void updateDeviceTelemetry(const std::string& ipAddress, int port,
                              const Telemetry& telemetry,
                              const std::vector<Alert>& newAlerts,
                              HealthState calculatedState);

    // Thread-safe marking of a device as OFFLINE
    void setDeviceOffline(const std::string& deviceId);

    // Check for device timeouts (mark inactive devices as OFFLINE)
    void checkTimeouts(int timeoutSeconds = 10);

    // Render formatted terminal monitoring dashboard
    void renderDashboard() const;

    // Get snapshot of all devices
    std::vector<Device> getAllDevices() const;

    // Get snapshot of recent alerts
    std::vector<Alert> getRecentAlerts() const;

private:
    mutable std::mutex m_mutex;
    std::map<std::string, Device> m_devices;
    std::vector<Alert> m_recentAlerts;
    static const size_t MAX_RECENT_ALERTS = 10;
};

#endif // DEVICEMANAGER_H
