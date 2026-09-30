#include "Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

std::ofstream Logger::s_logFile;
std::mutex Logger::s_mutex;
bool Logger::s_initialized = false;

void Logger::init(const std::string& logFilePath) {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_initialized) {
        s_logFile.open(logFilePath, std::ios::out | std::ios::app);
        if (!s_logFile.is_open()) {
            std::cerr << "[Logger Error] Failed to open log file: " << logFilePath << std::endl;
        }
        s_initialized = true;
    }
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    struct tm tm_buf;
    localtime_r(&in_time_t, &tm_buf);
    ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARNING";
        case LogLevel::CRITICAL: return "CRITICAL";
        case LogLevel::LOG_ERROR:return "ERROR";
        default:                 return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = levelToString(level);
    std::string formattedMessage = "[" + timestamp + "] [" + levelStr + "] " + message;

    // Log to file if open
    if (s_logFile.is_open()) {
        s_logFile << formattedMessage << std::endl;
    }
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::LOG_ERROR, message);
}
