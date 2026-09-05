#include "dharti/utils/logger.hpp"
#include <chrono>
#include <iomanip>
#include <ctime>

namespace dharti {
namespace utils {

std::mutex Logger::log_mutex_;

Logger::Logger(std::string name) : name_(std::move(name)) {}

void Logger::debug(const std::string& message) const {
    log(LogLevel::DEBUG_LEVEL, message);
}

void Logger::info(const std::string& message) const {
    log(LogLevel::INFO_LEVEL, message);
}

void Logger::warn(const std::string& message) const {
    log(LogLevel::WARN_LEVEL, message);
}

void Logger::error(const std::string& message) const {
    log(LogLevel::ERROR_LEVEL, message);
}

void Logger::log(LogLevel level, const std::string& message) const {
    std::lock_guard<std::mutex> lock(log_mutex_);

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_buf, &now_c);
#else
    localtime_r(&now_c, &tm_buf);
#endif

    const char* level_str = "INFO";
    switch (level) {
        case LogLevel::DEBUG_LEVEL: level_str = "DEBUG"; break;
        case LogLevel::INFO_LEVEL:  level_str = "INFO";  break;
        case LogLevel::WARN_LEVEL:  level_str = "WARN";  break;
        case LogLevel::ERROR_LEVEL: level_str = "ERROR"; break;
    }

    std::cout << "[" << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "] ["
              << level_str << "] [" << name_ << "]: "
              << message << std::endl;
}

} // namespace utils
} // namespace dharti
