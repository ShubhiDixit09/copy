#ifndef DHARTI_STORAGE_GDRIVE_CLIENT_HPP
#define DHARTI_STORAGE_GDRIVE_CLIENT_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace dharti {
namespace storage {

/**
 * @brief Result of an evidence upload operation to Google Drive.
 */
struct GDriveUploadResult {
    bool success{false};
    std::string file_id;
    std::string web_link;
    std::string sha256;
    uint64_t file_size{0};
    std::string folder_path;
    std::string filename;
    std::string error_message;
};

/**
 * @brief Google Drive Evidence Vault Client in Pure C++.
 * Communicates with the Google Apps Script Webhook to archive immutable evidence objects.
 */
class GDriveClient {
public:
    explicit GDriveClient(std::string webhook_url = "");

    /**
     * @brief Uploads raw string content (JSON, XML, HTML) to the partitioned Drive vault.
     * @param source_code e.g. "parivesh", "bhoomi_rashi"
     * @param filename e.g. "snapshot_001.json"
     * @param content Raw string data
     * @param mime_type e.g. "application/json"
     * @param explicit_folder Optional explicit folder path (defaults to dharti/raw/{source}/{YYYY}/{MM}/{DD})
     */
    GDriveUploadResult upload_raw_snapshot(
        const std::string& source_code,
        const std::string& filename,
        const std::string& content,
        const std::string& mime_type = "application/json",
        const std::string& explicit_folder = ""
    );

    /**
     * @brief Computes a standard partitioned vault folder path for the current date.
     * e.g. "dharti/raw/parivesh/2026/09/08"
     */
    static std::string build_partition_folder(const std::string& source_code);

    /**
     * @brief Computes a standard evidence vault folder path.
     * e.g. "dharti/evidence/clearance"
     */
    static std::string build_evidence_folder(const std::string& category);

    /**
     * @brief Standard C++ Base64 encoder for binary/text payloads.
     */
    static std::string base64_encode(const std::string& input);

private:
    std::string m_webhook_url;
};

} // namespace storage
} // namespace dharti

#endif // DHARTI_STORAGE_GDRIVE_CLIENT_HPP
