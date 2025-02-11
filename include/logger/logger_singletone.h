#ifndef LOGGER_SINGLETONE_H
#define LOGGER_SINGLETONE_H

#include <iostream>
#include <fstream>
#include <mutex>
#include <string>
#include <memory>
#include <chrono>
#include <iomanip>
#include <sstream>

enum class LogLevel { INFO, ERROR, FATAL, VERBOSE, DEBUG };

class Logger {
public:
    static Logger& GetInstance();  // Singleton instance

    void SetLogLevel(LogLevel level);  // Change log level at runtime
    void Log(LogLevel level, const std::string& message);
    ~Logger();

private:
    Logger();  // Private constructor for Singleton
    std::ofstream log_file;
    std::mutex log_mutex;
    LogLevel current_log_level;

    std::string GetTimestamp();
    std::string LogLevelToString(LogLevel level);
};

#endif // LOGGER_SINGLETONE_H
