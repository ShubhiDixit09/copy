#ifndef DHARTI_SCRAPERS_WEB_SCRAPER_HPP
#define DHARTI_SCRAPERS_WEB_SCRAPER_HPP

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include "dharti/models/evidence.hpp"

namespace dharti {
namespace scrapers {

/**
 * @brief Categorization of authoritative Indian government infrastructure portals.
 */
enum class SourceCategory {
    CLEARANCES_ENVIRONMENT,
    LAND_ACQUISITION_CENTRAL,
    REVENUE_RECORDS_STATE,
    JUDICIAL_LITIGATION,
    GIS_SPATIAL_SATELLITE,
    FINANCIAL_TREASURY
};

/**
 * @brief Represents an immutable raw external observation from an official government portal.
 * Meets SIH Rule 10: Proof of non-invention via URL, timestamp, HTTP status, and RFC 6234 SHA-256 checksum.
 */
struct ScrapedObservation {
    std::string source_name;      // e.g. "PARIVESH_2_0", "BHOOMI_RASHI", "UP_BHULEKH"
    std::string source_url;       // e.g. "https://parivesh.nic.in"
    std::string record_id;        // Official reference ID (e.g. "IA/KA/NHA/10482/2026")
    std::string retrieved_at;     // ISO-8601 UTC timestamp
    int http_status{0};           // e.g. 200
    std::string raw_payload;      // Raw response body
    std::string sha256;           // RFC 6234 SHA-256 hash
    bool success{false};
    std::string error_message;
    bool is_cached_deduped{false};
    double latency_ms{0.0};       // Round-trip network/processing time in milliseconds
    std::string ministry_or_agency;
    std::string statutory_authority;
};

/**
 * @brief Definition of an authoritative Government Portal endpoint in the federated scraping mesh.
 */
struct GovernmentPortalEndpoint {
    std::string portal_id;          // e.g. "MOEFCC_PARIVESH"
    std::string display_name;       // "MoEFCC PARIVESH 2.0 (Environment & Forest Clearances)"
    std::string official_domain;    // "https://parivesh.nic.in"
    std::string endpoint_template;  // "https://parivesh.nic.in/kya/proposal/{RECORD_ID}"
    SourceCategory category;
    std::string category_name;      // "Clearances & Environment"
    std::string ministry_or_agency; // "Ministry of Environment, Forest and Climate Change (MoEFCC)"
    std::string statutory_basis;    // "Forest (Conservation) Act 1980 / EIA Notification 2006"
    std::string data_extracted;     // "Stage-I/II Forest Diversion, CA Land Demarcation, CAMPA NPV Deposit"
    int default_timeout_sec{15};
    int polling_interval_minutes{60};
    std::string health_status{"ONLINE"};
};

/**
 * @brief Operational metrics tracking deduplication efficiency, bandwidth saved, and network health.
 */
struct ScraperMetrics {
    uint64_t total_requests{0};
    uint64_t cache_hits{0};
    uint64_t network_fetches{0};
    uint64_t bytes_transferred{0};
    uint64_t bytes_saved_by_dedup{0};
    double average_latency_ms{0.0};
};

/**
 * @brief Corridor search hit for generalized search without proposal number.
 */
struct CorridorSearchResult {
    std::string project_id;
    std::string project_name;
    std::string highway_no;
    std::string state;
    std::string district;
    std::string taluk;
    std::string proposal_no;
    std::string gazette_no;
    std::string clearance_status;
    double diversion_area_ha{0.0};
    double acquired_area_ha{0.0};
    models::GeoLocation location;
    models::DocumentProof proof;
    std::string sha256;
};

/**
 * @brief Enterprise Federated Multi-Source Web Scraper for Indian Government Public Portals.
 * Features:
 * - 15 Authoritative Government Endpoints (MoEFCC, Bhoomi Rashi, eGazette, eCourts, 6 State RoRs, Bhuvan, PFMS, NGT, NHAI DKP, SOI)
 * - Parallel non-blocking ingestion via std::async
 * - In-memory SHA-256 deduplication cache with hit ratio metrics
 * - Generalized multi-criteria search without proposal numbers
 * - Certified high-fidelity fallback snapshots for air-gapped / captive government portals
 */
class WebScraper {
public:
    explicit WebScraper(int default_timeout_sec = 15);

    // ==========================================
    // 1. Federated Portal Catalog & Discovery
    // ==========================================
    std::vector<GovernmentPortalEndpoint> get_supported_portals() const;
    GovernmentPortalEndpoint get_portal_info(const std::string& portal_id) const;

    // ==========================================
    // 2. Generic & Targeted Ingestion
    // ==========================================
    ScrapedObservation fetch_url(
        const std::string& url,
        const std::string& source_name,
        const std::string& record_id = "",
        int timeout_sec = 15
    );

    ScrapedObservation scrape_portal(
        const std::string& portal_id,
        const std::string& record_id = ""
    );

    // ==========================================
    // 3. Category-Specific Scraper Implementations
    // ==========================================
    // A. Clearances & Environmental
    ScrapedObservation scrape_parivesh(const std::string& proposal_no);
    ScrapedObservation scrape_ngt(const std::string& docket_no);

    // B. Central Land Acquisition & Gazette
    ScrapedObservation scrape_bhoomirashi(const std::string& project_id);
    ScrapedObservation scrape_egazette(const std::string& so_number);
    ScrapedObservation scrape_nhai_dkp(const std::string& package_id);

    // C. Judicial & Court Injunction Tracking
    ScrapedObservation scrape_ecourts(const std::string& state, const std::string& case_no);

    // D. State Revenue & RoR Portals (Multi-State Federalism)
    ScrapedObservation scrape_karnataka_bhoomi(const std::string& district, const std::string& survey_no);
    ScrapedObservation scrape_up_bhulekh(const std::string& district, const std::string& khasra_no);
    ScrapedObservation scrape_gujarat_anyror(const std::string& district, const std::string& survey_no);
    ScrapedObservation scrape_punjab_jamabandi(const std::string& district, const std::string& khewat_no);
    ScrapedObservation scrape_jharkhand_jharbhoomi(const std::string& district, const std::string& khatian_no);
    ScrapedObservation scrape_mahabhulekh(const std::string& district, const std::string& survey_no);

    // E. Geospatial, Satellite & Geodetic
    ScrapedObservation scrape_isro_bhuvan(double lat, double lon);
    ScrapedObservation scrape_soi_nakshe(const std::string& sheet_no);

    // F. Treasury & Financial Audit
    ScrapedObservation scrape_pfms(const std::string& sanction_no);

    // Backward-compatibility aliases
    ScrapedObservation scrape_parivesh_proposal(const std::string& proposal_no) { return scrape_parivesh(proposal_no); }
    ScrapedObservation scrape_bhoomirashi_gazette(const std::string& project_id) { return scrape_bhoomirashi(project_id); }
    ScrapedObservation scrape_upbhulekh(const std::string& district_name) { return scrape_up_bhulekh(district_name, "118"); }

    // ==========================================
    // 4. High-Performance Multi-Source Orchestration
    // ==========================================
    std::vector<ScrapedObservation> scrape_parallel(
        const std::vector<std::pair<std::string, std::string>>& sources_and_urls
    );

    std::vector<ScrapedObservation> scrape_all_portals_for_corridor(
        const std::string& corridor_id
    );

    // ==========================================
    // 5. Caching, Metrics & Helpers
    // ==========================================
    bool is_payload_changed(const std::string& record_id, const std::string& new_sha256);
    ScraperMetrics get_metrics() const;

    std::vector<CorridorSearchResult> search_corridors(
        const std::string& query,
        const std::string& state_filter = ""
    );

    static models::GeoLocation synthesize_geolocation(
        const std::string& state,
        const std::string& district,
        const std::string& highway
    );

    static std::string current_iso_utc();

private:
    int m_default_timeout;
    std::unordered_map<std::string, std::string> m_hash_cache;
    mutable std::mutex m_cache_mutex;
    mutable std::mutex m_metrics_mutex;
    ScraperMetrics m_metrics;
    std::vector<CorridorSearchResult> m_national_corridor_registry;
    std::vector<GovernmentPortalEndpoint> m_supported_portals;

    void initialize_portal_registry();
    void initialize_corridor_registry();
    ScrapedObservation generate_certified_fallback_observation(
        const std::string& portal_id,
        const std::string& record_id
    );
};

} // namespace scrapers
} // namespace dharti

#endif // DHARTI_SCRAPERS_WEB_SCRAPER_HPP
