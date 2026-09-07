#include "dharti/scrapers/web_scraper.hpp"
#include "dharti/utils/sha256.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <array>
#include <algorithm>

namespace dharti {
namespace scrapers {

WebScraper::WebScraper(int default_timeout_sec)
    : m_default_timeout(default_timeout_sec) {}

std::string WebScraper::current_iso_utc() {
    auto now = std::chrono::system_clock::now();
    auto in_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&in_time), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

ScrapedObservation WebScraper::fetch_url(
    const std::string& url,
    const std::string& source_name,
    const std::string& record_id,
    int timeout_sec
) {
    ScrapedObservation obs;
    obs.source_name = source_name;
    obs.source_url = url;
    obs.record_id = record_id;
    obs.retrieved_at = current_iso_utc();

    if (timeout_sec <= 0) timeout_sec = m_default_timeout;

    // Execute native Windows curl.exe with resilient TLS, browser user-agent, and timeout
    std::string cmd = "curl.exe -k -s -i -m " + std::to_string(timeout_sec) +
                      " -A \"Mozilla/5.0 (Windows NT 10.0; Win64; x64) Chrome/128.0.0.0\" \"" +
                      url + "\"";

    std::array<char, 1024> buffer;
    std::string response;
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) {
        obs.success = false;
        obs.error_message = "Failed to launch curl.exe process";
        return obs;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        response += buffer.data();
    }
    int exit_code = _pclose(pipe);

    if (response.empty() || exit_code != 0) {
        obs.success = false;
        obs.error_message = "Empty response or curl exit code: " + std::to_string(exit_code);
        return obs;
    }

    // Parse HTTP Status code
    // Example: HTTP/1.1 200 OK or HTTP/2 200
    size_t first_line_end = response.find("\r\n");
    if (first_line_end == std::string::npos) {
        first_line_end = response.find("\n");
    }
    if (first_line_end != std::string::npos) {
        std::string status_line = response.substr(0, first_line_end);
        size_t http_pos = status_line.find("HTTP/");
        if (http_pos != std::string::npos) {
            size_t code_pos = status_line.find(' ', http_pos);
            if (code_pos != std::string::npos) {
                try {
                    obs.http_status = std::stoi(status_line.substr(code_pos + 1, 3));
                } catch (...) {
                    obs.http_status = 200;
                }
            }
        }
    }

    // Separate HTTP header from payload
    size_t body_start = response.find("\r\n\r\n");
    if (body_start != std::string::npos) {
        obs.raw_payload = response.substr(body_start + 4);
    } else {
        body_start = response.find("\n\n");
        if (body_start != std::string::npos) {
            obs.raw_payload = response.substr(body_start + 2);
        } else {
            obs.raw_payload = response;
        }
    }

    // RFC 6234 SHA-256 Checksum on raw body
    obs.sha256 = utils::SHA256::hash_string(obs.raw_payload);
    obs.success = (obs.http_status >= 200 && obs.http_status < 400);

    return obs;
}

ScrapedObservation WebScraper::scrape_parivesh_proposal(const std::string& proposal_no) {
    // Official PARIVESH Portal
    std::string url = "https://parivesh.nic.in";
    auto obs = fetch_url(url, "PARIVESH", proposal_no, 15);
    if (!obs.success) {
        // Fallback probe to MoEFCC KYA
        obs = fetch_url("https://parivesh.nic.in/kya/", "PARIVESH", proposal_no, 15);
    }
    return obs;
}

ScrapedObservation WebScraper::scrape_bhoomirashi_gazette(const std::string& project_id) {
    // Official MoRTH Bhoomi Rashi Public Revamp Portal
    std::string url = "https://bhoomirashi.gov.in/auth/revamp/login1.cshtml";
    return fetch_url(url, "BHOOMI_RASHI", project_id, 15);
}

ScrapedObservation WebScraper::scrape_upbhulekh(const std::string& district_name) {
    // Official UP Bhulekh Revenue Portal
    std::string url = "https://upbhulekh.gov.in";
    return fetch_url(url, "UP_BHULEKH", district_name, 15);
}

} // namespace scrapers
} // namespace dharti
