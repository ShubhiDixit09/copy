#ifndef DHARTI_SCRAPERS_WEB_SCRAPER_HPP
#define DHARTI_SCRAPERS_WEB_SCRAPER_HPP

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace dharti {
namespace scrapers {

/**
 * @brief Represents an immutable raw external observation from an official government portal.
 * Meets SIH Rule 10: Proof of non-invention via URL, timestamp, HTTP status, and SHA-256 checksum.
 */
struct ScrapedObservation {
    std::string source_name;      // e.g. "PARIVESH", "BHOOMI_RASHI", "UP_BHULEKH"
    std::string source_url;       // e.g. "https://parivesh.nic.in"
    std::string record_id;        // Official reference ID (e.g. "IA/KA/NHA/10482/2026")
    std::string retrieved_at;     // ISO-8601 UTC timestamp
    int http_status{0};           // e.g. 200
    std::string raw_payload;      // Raw response body
    std::string sha256;           // RFC 6234 SHA-256 hash
    bool success{false};
    std::string error_message;
};

/**
 * @brief High-performance Pure C++ Web Scraper for Indian Government Public Portals.
 * Operates natively using curl.exe without Python runtime dependencies.
 */
class WebScraper {
public:
    explicit WebScraper(int default_timeout_sec = 15);

    /**
     * @brief Performs an HTTP GET request to an official portal and returns an observation.
     */
    ScrapedObservation fetch_url(
        const std::string& url,
        const std::string& source_name,
        const std::string& record_id = "",
        int timeout_sec = 15
    );

    /**
     * @brief Scrapes live clearance information from MoEFCC PARIVESH.
     */
    ScrapedObservation scrape_parivesh_proposal(const std::string& proposal_no);

    /**
     * @brief Scrapes live Gazette Land Acquisition notifications from MoRTH Bhoomi Rashi.
     */
    ScrapedObservation scrape_bhoomirashi_gazette(const std::string& project_id);

    /**
     * @brief Scrapes live revenue portal status from UP Bhulekh.
     */
    ScrapedObservation scrape_upbhulekh(const std::string& district_name);

    /**
     * @brief Formats the current system UTC time in ISO-8601 format.
     */
    static std::string current_iso_utc();

private:
    int m_default_timeout;
};

} // namespace scrapers
} // namespace dharti

#endif // DHARTI_SCRAPERS_WEB_SCRAPER_HPP
