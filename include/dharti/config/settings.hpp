#ifndef DHARTI_CONFIG_SETTINGS_HPP
#define DHARTI_CONFIG_SETTINGS_HPP

#include <string>

namespace dharti {
namespace config {

class AppConfig {
public:
    static AppConfig& instance();

    const std::string& app_name() const { return app_name_; }
    const std::string& app_env() const { return app_env_; }
    bool debug() const { return debug_; }
    const std::string& log_level() const { return log_level_; }

    void reload();

private:
    AppConfig();

    std::string app_name_;
    std::string app_env_;
    bool debug_;
    std::string log_level_;
};

inline const AppConfig& get_config() {
    return AppConfig::instance();
}

} // namespace config
} // namespace dharti

#endif // DHARTI_CONFIG_SETTINGS_HPP
