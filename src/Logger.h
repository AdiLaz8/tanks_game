#ifndef LOGGER_H
#include <fstream>
#include <string>

class Logger {
public:
    static void init(const std::string& configFile);
    static void debug(const std::string& message);
    static void shutdown();

private:
    static std::ofstream debugLogFile;
    static bool debugEnabled;
};
#endif