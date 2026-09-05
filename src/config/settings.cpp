#include "dharti/config/settings.hpp"
#include <cstdlib>
#include <algorithm>

namespace dharti {
namespace config {

namespace {
std::string get_env_or_default(const char* name, const std::string& default_val) {
    const char* val = std::getenv(name);
    return (val && *val) ? std::string(val) : default_val;
}

bool get_env_bool_or_default(const char* name, bool default_val) {
    const char* val = std::getenv(name);
    if (!val || !*val) return default_val;
    std::string s(val);
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return (s == "true" || s == "1" || s == "yes");
}
} // anonymous namespace

AppConfig::AppConfig() {
    reload();
}

AppConfig& AppConfig::instance() {
    static AppConfig cfg;
    return cfg;
}

void AppConfig::reload() {
    app_name_ = get_env_or_default("DHARTI_APP_NAME", "DHARTI");
    app_env_ = get_env_or_default("DHARTI_ENV", "development");
    debug_ = get_env_bool_or_default("DHARTI_DEBUG", true);
    log_level_ = get_env_or_default("DHARTI_LOG_LEVEL", "INFO");
}

} // namespace config
} // namespace dharti
