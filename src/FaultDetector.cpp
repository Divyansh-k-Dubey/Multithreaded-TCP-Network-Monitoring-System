#include "FaultDetector.h"
#include <sstream>
#include <iomanip>

FaultDetector::FaultDetector(Thresholds thresholds)
    : m_thresholds(thresholds) {}

std::vector<Alert> FaultDetector::evaluate(const Telemetry& telemetry, HealthState& outOverallState) const {
    std::vector<Alert> alerts;
    HealthState maxState = HealthState::NORMAL;

    auto updateMaxState = [&maxState](HealthState state) {
        if (state == HealthState::CRITICAL) {
            maxState = HealthState::CRITICAL;
        } else if (state == HealthState::WARNING && maxState != HealthState::CRITICAL) {
            maxState = HealthState::WARNING;
        }
    };

    const std::string& devId = telemetry.getDeviceId();
    std::time_t now = std::time(nullptr);

    // 1. Temperature Check
    double temp = telemetry.getTemperature();
    if (temp > m_thresholds.tempCriticalThreshold) {
        updateMaxState(HealthState::CRITICAL);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << temp << " C";
        alerts.push_back({devId, LogLevel::CRITICAL, "HIGH TEMPERATURE", temp,
                          "[ALERT] " + devId + " | HIGH TEMPERATURE | " + ss.str(), now});
    } else if (temp > m_thresholds.tempWarningThreshold) {
        updateMaxState(HealthState::WARNING);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << temp << " C";
        alerts.push_back({devId, LogLevel::WARNING, "HIGH TEMPERATURE", temp,
                          "[ALERT] " + devId + " | HIGH TEMPERATURE | " + ss.str(), now});
    }

    // 2. Signal Strength Check (lower is worse)
    double signal = telemetry.getSignalStrength();
    if (signal < m_thresholds.signalCriticalThreshold) {
        updateMaxState(HealthState::CRITICAL);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << signal;
        alerts.push_back({devId, LogLevel::CRITICAL, "LOW SIGNAL", signal,
                          "[ALERT] " + devId + " | LOW SIGNAL | " + ss.str(), now});
    } else if (signal < m_thresholds.signalWarningThreshold) {
        updateMaxState(HealthState::WARNING);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << signal;
        alerts.push_back({devId, LogLevel::WARNING, "LOW SIGNAL", signal,
                          "[ALERT] " + devId + " | LOW SIGNAL | " + ss.str(), now});
    }

    // 3. Packet Loss Check
    double loss = telemetry.getPacketLoss();
    if (loss > m_thresholds.lossCriticalThreshold) {
        updateMaxState(HealthState::CRITICAL);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << loss << "%";
        alerts.push_back({devId, LogLevel::CRITICAL, "HIGH PACKET LOSS", loss,
                          "[ALERT] " + devId + " | HIGH PACKET LOSS | " + ss.str(), now});
    } else if (loss > m_thresholds.lossWarningThreshold) {
        updateMaxState(HealthState::WARNING);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << loss << "%";
        alerts.push_back({devId, LogLevel::WARNING, "HIGH PACKET LOSS", loss,
                          "[ALERT] " + devId + " | HIGH PACKET LOSS | " + ss.str(), now});
    }

    // 4. CPU Utilization Check
    double cpu = telemetry.getCpuUtilization();
    if (cpu > m_thresholds.cpuCriticalThreshold) {
        updateMaxState(HealthState::CRITICAL);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << cpu << "%";
        alerts.push_back({devId, LogLevel::CRITICAL, "HIGH CPU UTILIZATION", cpu,
                          "[ALERT] " + devId + " | HIGH CPU UTILIZATION | " + ss.str(), now});
    } else if (cpu > m_thresholds.cpuWarningThreshold) {
        updateMaxState(HealthState::WARNING);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << cpu << "%";
        alerts.push_back({devId, LogLevel::WARNING, "HIGH CPU UTILIZATION", cpu,
                          "[ALERT] " + devId + " | HIGH CPU UTILIZATION | " + ss.str(), now});
    }

    outOverallState = maxState;
    return alerts;
}
