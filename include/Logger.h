#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <iostream>

enum class LogLevel {
    INFO,
    WARNING,
    CRITICAL,
    LOG_ERROR
};

class Logger {
public:
    // Initialize logger with log file path
    static void init(const std::string& logFilePath = "logs/network_monitor.log");
    
    // Log a message with specified log level
    static void log(LogLevel level, const std::string& message);
    
    // Helper convenience functions
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void critical(const std::string& message);
    static void error(const std::string& message);

    // Convert LogLevel enum to readable string
    static std::string levelToString(LogLevel level);

private:
    static std::ofstream s_logFile;
    static std::mutex s_mutex;
    static bool s_initialized;

    // Helper to format current timestamp
    static std::string getCurrentTimestamp();
};

#endif // LOGGER_H
