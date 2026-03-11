#ifndef CORE_LOGGER_HPP
#define CORE_LOGGER_HPP

#include <fstream>
#include <string>

namespace core {

class Logger {
public:
    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    static void log(const std::string& level,
                    const char* file,
                    int line,
                    const std::string& message);

private:
    static std::ofstream logFile;

    static void init();
};

} // namespace core

#define LOG_INFO(message) core::Logger::log("I", __FILE__, __LINE__, message)
#define LOG_WARNING(message) core::Logger::log("W", __FILE__, __LINE__, message)
#define LOG_ERROR(message) core::Logger::log("E", __FILE__, __LINE__, message)

#endif // CORE_LOGGER_HPP