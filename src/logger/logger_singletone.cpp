
#include "logger/logger_singletone.h"

Logger::Logger() : current_log_level(LogLevel::INFO) {
    log_file.open("system_monitor.log", std::ios::app);
    if (!log_file) {
        std::cerr << "Failed to open log file!" << std::endl;
    }
}

Logger::~Logger() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

void Logger::SetLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(log_mutex);
    current_log_level = level;
}

void Logger::Log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (level >= current_log_level) {
        std::string log_entry = GetTimestamp() + " [" + LogLevelToString(level) + "] " + message;
        
        std::cout << log_entry << std::endl; // Print to console
        if (log_file.is_open()) {
            log_file << log_entry << std::endl; // Write to file
        }
    }
}

std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        case LogLevel::VERBOSE: return "VERBOSE";
        case LogLevel::DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}