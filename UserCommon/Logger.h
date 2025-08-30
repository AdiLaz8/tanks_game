#ifndef USERCOMMON_318772340_206580102_LOGGER_H
#define USERCOMMON_318772340_206580102_LOGGER_H

#include <fstream>
#include <string>
#include <mutex>
#include <memory>

namespace UserCommon_318772340_206580102 {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    OFF = 4
};

class Logger {
public:
    // Initialize logger with configuration file
    static void init(const std::string& configFile = "logging.conf");
    
    // Logging methods for different levels
    static void debug(const std::string& message, const std::string& component = "");
    static void info(const std::string& message, const std::string& component = "");
    static void warn(const std::string& message, const std::string& component = "");
    static void error(const std::string& message, const std::string& component = "");
    
    // Generic log method
    static void log(LogLevel level, const std::string& message, const std::string& component = "");
    
    // Configuration methods
    static void setLogLevel(LogLevel level);
    static void setLogFile(const std::string& filename);
    static void enableConsoleOutput(bool enable);
    static void enableTimestamp(bool enable);
    static void enableThreadId(bool enable);
    
    // Cleanup
    static void shutdown();
    
    // Utility methods
    static std::string levelToString(LogLevel level);
    static LogLevel stringToLevel(const std::string& levelStr);

private:
    static std::unique_ptr<std::ofstream> logFile;
    static std::mutex logMutex;
    static LogLevel currentLogLevel;
    static bool consoleOutput;
    static bool timestampEnabled;
    static bool threadIdEnabled;
    static std::string logFileName;
    
    // Internal helper methods
    static std::string getCurrentTimestamp();
    static std::string getCurrentThreadId();
    static void writeLog(LogLevel level, const std::string& message, const std::string& component);
    static void writeLogInternal(LogLevel level, const std::string& message, const std::string& component);
    static void loadConfiguration(const std::string& configFile);
};

// Convenience macros for easier logging
#define LOG_DEBUG(msg, comp) UserCommon_318772340_206580102::Logger::debug(msg, comp)
#define LOG_INFO(msg, comp) UserCommon_318772340_206580102::Logger::info(msg, comp)
#define LOG_WARN(msg, comp) UserCommon_318772340_206580102::Logger::warn(msg, comp)
#define LOG_ERROR(msg, comp) UserCommon_318772340_206580102::Logger::error(msg, comp)

} // namespace UserCommon_318772340_206580102

#endif // USERCOMMON_318772340_206580102_LOGGER_H