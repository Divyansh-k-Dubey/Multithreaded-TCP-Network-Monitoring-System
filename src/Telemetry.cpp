#include "Telemetry.h"
#include <sstream>
#include <iomanip>
#include <iostream>

Telemetry::Telemetry()
    : m_deviceId("UNKNOWN"),
      m_temperature(0.0),
      m_signalStrength(0.0),
      m_packetLoss(0.0),
      m_cpuUtilization(0.0),
      m_timestamp(std::time(nullptr)) {}

Telemetry::Telemetry(std::string deviceId, double temp, double signal, double packetLoss, double cpu)
    : m_deviceId(std::move(deviceId)),
      m_temperature(temp),
      m_signalStrength(signal),
      m_packetLoss(packetLoss),
      m_cpuUtilization(cpu),
      m_timestamp(std::time(nullptr)) {}

std::optional<Telemetry> Telemetry::parse(const std::string& rawData) {
    if (rawData.empty()) {
        return std::nullopt;
    }

    Telemetry telemetry;
    std::stringstream ss(rawData);
    std::string token;

    bool hasId = false, hasTemp = false, hasSignal = false, hasLoss = false, hasCpu = false;

    while (std::getline(ss, token, ';')) {
        // Remove trailing \r or \n if present
        while (!token.empty() && (token.back() == '\r' || token.back() == '\n' || token.back() == ' ')) {
            token.pop_back();
        }
        size_t equalPos = token.find('=');
        if (equalPos == std::string::npos) {
            continue;
        }

        std::string key = token.substr(0, equalPos);
        std::string valueStr = token.substr(equalPos + 1);

        try {
            if (key == "DEVICE_ID") {
                telemetry.setDeviceId(valueStr);
                hasId = true;
            } else if (key == "TEMP") {
                telemetry.setTemperature(std::stod(valueStr));
                hasTemp = true;
            } else if (key == "SIGNAL") {
                telemetry.setSignalStrength(std::stod(valueStr));
                hasSignal = true;
            } else if (key == "PACKET_LOSS") {
                telemetry.setPacketLoss(std::stod(valueStr));
                hasLoss = true;
            } else if (key == "CPU") {
                telemetry.setCpuUtilization(std::stod(valueStr));
                hasCpu = true;
            }
        } catch (const std::invalid_argument&) {
            return std::nullopt;
        } catch (const std::out_of_range&) {
            return std::nullopt;
        }
    }

    if (hasId && hasTemp && hasSignal && hasLoss && hasCpu) {
        telemetry.setTimestamp(std::time(nullptr));
        return telemetry;
    }

    return std::nullopt;
}

std::string Telemetry::serialize() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "DEVICE_ID=" << m_deviceId << ";"
       << "TEMP=" << m_temperature << ";"
       << "SIGNAL=" << m_signalStrength << ";"
       << "PACKET_LOSS=" << m_packetLoss << ";"
       << "CPU=" << m_cpuUtilization;
    return ss.str();
}
