#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <string>
#include <ctime>
#include <optional>

class Telemetry {
public:
    Telemetry();
    Telemetry(std::string deviceId, double temp, double signal, double packetLoss, double cpu);

    // Parse semicolon-delimited key-value string format:
    // DEVICE_ID=DEVICE-01;TEMP=65.2;SIGNAL=82.5;PACKET_LOSS=1.2;CPU=45.0
    static std::optional<Telemetry> parse(const std::string& rawData);

    // Serialize object into formatted telemetry string
    std::string serialize() const;

    // Getters
    const std::string& getDeviceId() const { return m_deviceId; }
    double getTemperature() const { return m_temperature; }
    double getSignalStrength() const { return m_signalStrength; }
    double getPacketLoss() const { return m_packetLoss; }
    double getCpuUtilization() const { return m_cpuUtilization; }
    std::time_t getTimestamp() const { return m_timestamp; }

    // Setters
    void setDeviceId(const std::string& id) { m_deviceId = id; }
    void setTemperature(double temp) { m_temperature = temp; }
    void setSignalStrength(double signal) { m_signalStrength = signal; }
    void setPacketLoss(double loss) { m_packetLoss = loss; }
    void setCpuUtilization(double cpu) { m_cpuUtilization = cpu; }
    void setTimestamp(std::time_t ts) { m_timestamp = ts; }

private:
    std::string m_deviceId;
    double m_temperature;
    double m_signalStrength;
    double m_packetLoss;
    double m_cpuUtilization;
    std::time_t m_timestamp;
};

#endif // TELEMETRY_H
