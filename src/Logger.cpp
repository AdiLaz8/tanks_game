#include "Logger.h"
#include <fstream>
#include <iostream>

std::ofstream Logger::debugLogFile;
bool Logger::debugEnabled = false;

void Logger::init(const std::string& configFile) {
    std::ifstream file(configFile);
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("ENABLE_DEBUG_LOG=true") != std::string::npos) {
            debugEnabled = true;
            debugLogFile.open("debug_log.txt");
            break;
        }
    }
}

void Logger::debug(const std::string& message) {
    if (debugEnabled && debugLogFile.is_open()) {
        debugLogFile << message << std::endl;
    }
}

void Logger::shutdown() {
    if (debugLogFile.is_open()) {
        debugLogFile.close();
    }
}