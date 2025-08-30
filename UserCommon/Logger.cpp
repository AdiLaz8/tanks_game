#include "Logger.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <thread>
#include <filesystem>

namespace UserCommon_318772340_206580102 {

// Static member definitions
std::unique_ptr<std::ofstream> Logger::logFile = nullptr;
std::mutex Logger::logMutex;
LogLevel Logger::currentLogLevel = LogLevel::INFO;
bool Logger::consoleOutput = true;
bool Logger::timestampEnabled = true;
bool Logger::threadIdEnabled = false;
std::string Logger::logFileName = "tanks_game.log";

void Logger::init(const std::string& configFile) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    // Load configuration from file
    loadConfiguration(configFile);
    
    // Initialize log file
    if (!logFileName.empty()) {
        logFile = std::make_unique<std::ofstream>(logFileName, std::ios::app);
        if (!logFile->is_open()) {
            std::cerr << "Warning: Could not open log file: " << logFileName << std::endl;
            logFile.reset();
        }
    }
    
    // Log initialization messages
    if (LogLevel::INFO >= currentLogLevel) {
        writeLogInternal(LogLevel::INFO, "Logger initialized", "LOGGER");
        writeLogInternal(LogLevel::INFO, "Log level: " + levelToString(currentLogLevel), "LOGGER");
        writeLogInternal(LogLevel::INFO, "Console output: " + std::string(consoleOutput ? "enabled" : "disabled"), "LOGGER");
        writeLogInternal(LogLevel::INFO, "Timestamp: " + std::string(timestampEnabled ? "enabled" : "disabled"), "LOGGER");
        writeLogInternal(LogLevel::INFO, "Thread ID: " + std::string(threadIdEnabled ? "enabled" : "disabled"), "LOGGER");
    }
}

void Logger::debug(const std::string& message, const std::string& component) {
    log(LogLevel::DEBUG, message, component);
}

void Logger::info(const std::string& message, const std::string& component) {
    log(LogLevel::INFO, message, component);
}

void Logger::warn(const std::string& message, const std::string& component) {
    log(LogLevel::WARN, message, component);
}

void Logger::error(const std::string& message, const std::string& component) {
    log(LogLevel::ERROR, message, component);
}

void Logger::log(LogLevel level, const std::string& message, const std::string& component) {
    if (level < currentLogLevel) {
        return; 
    }
    
    writeLog(level, message, component);
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    currentLogLevel = level;
    writeLogInternal(LogLevel::INFO, "Log level changed to: " + levelToString(level), "LOGGER");
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    // Close current file
    if (logFile && logFile->is_open()) {
        logFile->close();
    }
    
    logFileName = filename;
    
    // Open new file
    if (!filename.empty()) {
        logFile = std::make_unique<std::ofstream>(filename, std::ios::app);
        if (!logFile->is_open()) {
            std::cerr << "Warning: Could not open log file: " << filename << std::endl;
            logFile.reset();
        }
    }
}

void Logger::enableConsoleOutput(bool enable) {
    std::lock_guard<std::mutex> lock(logMutex);
    consoleOutput = enable;
}

void Logger::enableTimestamp(bool enable) {
    std::lock_guard<std::mutex> lock(logMutex);
    timestampEnabled = enable;
}

void Logger::enableThreadId(bool enable) {
    std::lock_guard<std::mutex> lock(logMutex);
    threadIdEnabled = enable;
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(logMutex);
    
    writeLogInternal(LogLevel::INFO, "Logger shutting down", "LOGGER");
    
    if (logFile && logFile->is_open()) {
        logFile->close();
    }
    logFile.reset();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::OFF:   return "OFF";
        default: return "UNKNOWN";
    }
}

LogLevel Logger::stringToLevel(const std::string& levelStr) {
    if (levelStr == "DEBUG") return LogLevel::DEBUG;
    if (levelStr == "INFO")  return LogLevel::INFO;
    if (levelStr == "WARN")  return LogLevel::WARN;
    if (levelStr == "ERROR") return LogLevel::ERROR;
    if (levelStr == "OFF")   return LogLevel::OFF;
    return LogLevel::INFO; // Default
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::getCurrentThreadId() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

void Logger::writeLog(LogLevel level, const std::string& message, const std::string& component) {
    std::lock_guard<std::mutex> lock(logMutex);
    writeLogInternal(level, message, component);
}

void Logger::writeLogInternal(LogLevel level, const std::string& message, const std::string& component) {
    std::stringstream logEntry;
    
    // Build log entry
    if (timestampEnabled) {
        logEntry << "[" << getCurrentTimestamp() << "] ";
    }
    
    logEntry << "[" << levelToString(level) << "] ";
    
    if (threadIdEnabled) {
        logEntry << "[Thread:" << getCurrentThreadId() << "] ";
    }
    
    if (!component.empty()) {
        logEntry << "[" << component << "] ";
    }
    
    logEntry << message;
    
    std::string logLine = logEntry.str();
    
    // Write to console if enabled
    if (consoleOutput && level >= LogLevel::INFO) {
        if (level >= LogLevel::WARN) {
            std::cerr << logLine << std::endl;
        } else {
            std::cout << logLine << std::endl;
        }
    }
    
    // Write to file if available
    if (logFile && logFile->is_open()) {
        *logFile << logLine << std::endl;
        logFile->flush(); 
    }
}

void Logger::loadConfiguration(const std::string& configFile) {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        // Create default config file if it doesn't exist
        std::ofstream defaultConfig(configFile);
        if (defaultConfig.is_open()) {
            defaultConfig << "# Tanks Game Logging Configuration\n";
            defaultConfig << "LOG_LEVEL=INFO\n";
            defaultConfig << "LOG_FILE=tanks_game.log\n";
            defaultConfig << "CONSOLE_OUTPUT=true\n";
            defaultConfig << "TIMESTAMP=true\n";
            defaultConfig << "THREAD_ID=false\n";
            defaultConfig.close();
        }
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        // Process configuration options
        if (key == "LOG_LEVEL") {
            currentLogLevel = stringToLevel(value);
        } else if (key == "LOG_FILE") {
            logFileName = value;
        } else if (key == "CONSOLE_OUTPUT") {
            consoleOutput = (value == "true" || value == "1");
        } else if (key == "TIMESTAMP") {
            timestampEnabled = (value == "true" || value == "1");
        } else if (key == "THREAD_ID") {
            threadIdEnabled = (value == "true" || value == "1");
        }
    }
}

} // namespace UserCommon_318772340_206580102