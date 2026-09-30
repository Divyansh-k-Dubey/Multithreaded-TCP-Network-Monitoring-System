#ifndef FAULTDETECTOR_H
#define FAULTDETECTOR_H

#include "Telemetry.h"
#include "Device.h"
#include "Logger.h"
#include <vector>
#include <string>

struct Thresholds {
    // Temperature thresholds (°C)
    double tempWarningThreshold = 75.0;
    double tempCriticalThreshold = 85.0;

    // Signal Strength thresholds (0-100)
    double signalWarningThreshold = 60.0;
    double signalCriticalThreshold = 40.0;

    // Packet Loss thresholds (%)
    double lossWarningThreshold = 5.0;
    double lossCriticalThreshold = 15.0;

    // CPU Utilization thresholds (%)
    double cpuWarningThreshold = 80.0;
    double cpuCriticalThreshold = 95.0;
};

struct Alert {
    std::string deviceId;
    LogLevel severity;
    std::string metricName;
    double currentValue;
    std::string message;
    std::time_t timestamp;
};

class FaultDetector {
public:
    explicit FaultDetector(Thresholds thresholds = Thresholds());

    // Evaluates telemetry, generates alerts, and calculates overall health state
    std::vector<Alert> evaluate(const Telemetry& telemetry, HealthState& outOverallState) const;

    // Access thresholds
    const Thresholds& getThresholds() const { return m_thresholds; }
    void setThresholds(const Thresholds& thresholds) { m_thresholds = thresholds; }

private:
    Thresholds m_thresholds;
};

#endif // FAULTDETECTOR_H
