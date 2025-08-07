#ifndef USERCOMMON_318772340_206580102_LOGGER_H
#define USERCOMMON_318772340_206580102_LOGGER_H

#include <fstream>
#include <string>

namespace UserCommon_318772340_206580102 {

class Logger {
public:
    static void init(const std::string& configFile);
    static void debug(const std::string& message);
    static void shutdown();

private:
    static std::ofstream debugLogFile;
    static bool debugEnabled;
};

} // namespace UserCommon_318772340_206580102

#endif // USERCOMMON_318772340_206580102_LOGGER_H