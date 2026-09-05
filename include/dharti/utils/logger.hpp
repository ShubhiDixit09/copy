#ifndef DHARTI_UTILS_LOGGER_HPP
#define DHARTI_UTILS_LOGGER_HPP

#include <string>
#include <iostream>
#include <mutex>

namespace dharti {
namespace utils {

enum class LogLevel {
    DEBUG_LEVEL = 0,
    INFO_LEVEL = 1,
    WARN_LEVEL = 2,
    ERROR_LEVEL = 3
};

class Logger {
public:
    explicit Logger(std::string name);

    void debug(const std::string& message) const;
    void info(const std::string& message) const;
    void warn(const std::string& message) const;
    void error(const std::string& message) const;

    const std::string& name() const { return name_; }

private:
    void log(LogLevel level, const std::string& message) const;

    std::string name_;
    static std::mutex log_mutex_;
};

inline Logger get_logger(const std::string& name) {
    return Logger(name);
}

} // namespace utils
} // namespace dharti

#endif // DHARTI_UTILS_LOGGER_HPP
