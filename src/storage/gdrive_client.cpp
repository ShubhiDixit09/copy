#include "dharti/storage/gdrive_client.hpp"
#include "dharti/utils/sha256.hpp"
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <iostream>
#include <array>
#include <algorithm>

namespace dharti {
namespace storage {

static const char B64_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string GDriveClient::base64_encode(const std::string& in) {
    std::string out;
    int val = 0, valb = -6;
    for (uint8_t c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(B64_CHARS[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(B64_CHARS[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

GDriveClient::GDriveClient(std::string webhook_url)
    : m_webhook_url(std::move(webhook_url)) {
    if (m_webhook_url.empty()) {
        const char* env_url = std::getenv("GDRIVE_WEBHOOK_URL");
        if (env_url) {
            m_webhook_url = env_url;
        } else {
            // Read from .env
            std::ifstream env_file(".env");
            std::string line;
            while (std::getline(env_file, line)) {
                if (line.rfind("GDRIVE_WEBHOOK_URL=", 0) == 0) {
                    m_webhook_url = line.substr(19);
                    if (!m_webhook_url.empty() && (m_webhook_url.front() == '"' || m_webhook_url.front() == '\'')) {
                        m_webhook_url = m_webhook_url.substr(1, m_webhook_url.length() - 2);
                    }
                    break;
                }
            }
            if (m_webhook_url.empty()) {
                m_webhook_url = "https://script.google.com/macros/s/AKfycbwdFuqoOa06F0Z1wIFJ6Awlkr_EM5ex-PJBjKPLV7U-Xa5Uyqwbw4nphvUmFU2Wi-P6mA/exec";
            }
        }
    }
}

std::string GDriveClient::build_partition_folder(const std::string& source_code) {
    auto now = std::chrono::system_clock::now();
    auto in_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "dharti/raw/" << source_code << "/"
       << std::put_time(std::gmtime(&in_time), "%Y/%m/%d");
    return ss.str();
}

std::string GDriveClient::build_evidence_folder(const std::string& category) {
    return "dharti/evidence/" + category;
}

GDriveUploadResult GDriveClient::upload_raw_snapshot(
    const std::string& source_code,
    const std::string& filename,
    const std::string& content,
    const std::string& mime_type,
    const std::string& explicit_folder
) {
    GDriveUploadResult res;
    res.filename = filename;
    res.file_size = content.size();
    res.sha256 = utils::SHA256::hash_string(content);
    res.folder_path = explicit_folder.empty() ? build_partition_folder(source_code) : explicit_folder;

    std::string b64_content = base64_encode(content);
    int rand_id = std::rand();
    std::string temp_payload_file = "temp_gdrive_payload_" + std::to_string(rand_id) + ".json";

    {
        std::ofstream req_out(temp_payload_file);
        req_out << "{\n"
                << "  \"root_folder\": \"dharti\",\n"
                << "  \"folder_path\": \"" << res.folder_path << "\",\n"
                << "  \"folder_name\": \"" << res.folder_path << "\",\n"
                << "  \"file_name\": \"" << filename << "\",\n"
                << "  \"mime_type\": \"" << mime_type << "\",\n"
                << "  \"file_base64\": \"" << b64_content << "\",\n"
                << "  \"description\": \"Vault: " << res.folder_path << " | SHA256: " << res.sha256 << "\"\n"
                << "}";
    }

    // Pure C++ invocation of native Windows curl.exe (zero Python dependency)
    std::string cmd = "curl.exe -s -L -H \"Content-Type: application/json\" --data-binary \"@" +
                      temp_payload_file + "\" \"" + m_webhook_url + "\"";

    std::array<char, 512> buffer;
    std::string response_str;
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) {
        res.success = false;
        res.error_message = "Failed to execute curl.exe uploader process";
        remove(temp_payload_file.c_str());
        return res;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        response_str += buffer.data();
    }
    _pclose(pipe);
    remove(temp_payload_file.c_str());

    // Parse JSON response for file_id, web_link, status
    auto find_json_val = [](const std::string& json, const std::string& key) -> std::string {
        std::string search = "\"" + key + "\":";
        size_t pos = json.find(search);
        if (pos == std::string::npos) {
            search = "\"" + key + "\" :";
            pos = json.find(search);
        }
        if (pos == std::string::npos) return "";

        size_t quote_start = json.find("\"", pos + search.length());
        if (quote_start == std::string::npos) return "";
        size_t quote_end = json.find("\"", quote_start + 1);
        if (quote_end == std::string::npos) return "";
        return json.substr(quote_start + 1, quote_end - quote_start - 1);
    };

    std::string status = find_json_val(response_str, "status");
    std::transform(status.begin(), status.end(), status.begin(), ::toupper);
    std::string file_id = find_json_val(response_str, "file_id");
    std::string web_link = find_json_val(response_str, "web_link");

    if (status == "SUCCESS" && !file_id.empty()) {
        res.success = true;
        res.file_id = file_id;
        res.web_link = web_link;
    } else {
        res.success = false;
        res.error_message = response_str.empty() ? "Empty response from Drive webhook" : response_str;
    }

    return res;
}

} // namespace storage
} // namespace dharti
