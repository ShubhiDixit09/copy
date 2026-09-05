#ifndef DHARTI_CORE_BASE_HPP
#define DHARTI_CORE_BASE_HPP

#include <string>
#include <chrono>

namespace dharti {
namespace core {

enum class HealthStatus {
    HEALTHY,
    DEGRADED,
    UNHEALTHY
};

struct ServiceHealth {
    std::string service_name;
    HealthStatus status;
    std::string details;
    int64_t timestamp_ms;
};

/**
 * @brief Abstract base class for all DHARTI core domain services in C++.
 */
class BaseService {
public:
    virtual ~BaseService() = default;

    /**
     * @brief Returns the operational health status of the service.
     */
    virtual ServiceHealth health_check() const = 0;

    /**
     * @brief Returns the unique identifier/name of the service.
     */
    virtual std::string service_name() const = 0;
};

} // namespace core
} // namespace dharti

#endif // DHARTI_CORE_BASE_HPP
