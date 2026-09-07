#include "dharti/storage/neon_client.hpp"
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <array>
#include <algorithm>

namespace dharti {
namespace storage {

static std::string escape_sql(const std::string& input) {
    std::string out;
    out.reserve(input.size() * 2);
    for (char c : input) {
        if (c == '\'') out += "''";
        else if (c == '\\') out += "\\\\";
        else out += c;
    }
    return out;
}

NeonClient::NeonClient(std::string db_url) : m_db_url(std::move(db_url)) {
    if (m_db_url.empty()) {
        const char* env_db = std::getenv("DATABASE_URL");
        if (env_db) {
            m_db_url = env_db;
        } else {
            // Read from .env
            std::ifstream env_file(".env");
            std::string line;
            while (std::getline(env_file, line)) {
                if (line.rfind("DATABASE_URL=", 0) == 0) {
                    m_db_url = line.substr(13);
                    // trim quotes
                    if (!m_db_url.empty() && (m_db_url.front() == '"' || m_db_url.front() == '\'')) {
                        m_db_url = m_db_url.substr(1, m_db_url.length() - 2);
                    }
                    break;
                }
            }
        }
    }
}

bool NeonClient::execute_sql(const std::string& sql) {
    if (m_db_url.empty()) {
        std::cerr << "[NeonClient ERROR] DATABASE_URL is not set." << std::endl;
        return false;
    }

    int rand_id = std::rand();
    std::string temp_sql_file = "temp_exec_" + std::to_string(rand_id) + ".sql";
    std::string temp_py_file = "temp_runner_" + std::to_string(rand_id) + ".py";

    {
        std::ofstream out(temp_sql_file);
        out << sql;
    }
    {
        std::ofstream py_out(temp_py_file);
        py_out << "import os, sys, psycopg2\n"
               << "try:\n"
               << "    with open(r'" << temp_sql_file << "', 'r', encoding='utf-8') as f:\n"
               << "        sql_content = f.read()\n"
               << "    conn = psycopg2.connect(r'" << m_db_url << "')\n"
               << "    conn.autocommit = True\n"
               << "    cur = conn.cursor()\n"
               << "    cur.execute(sql_content)\n"
               << "    cur.close()\n"
               << "    conn.close()\n"
               << "    sys.exit(0)\n"
               << "except Exception as e:\n"
               << "    print('[DB ERROR]', e, file=sys.stderr)\n"
               << "    sys.exit(1)\n";
    }

    std::string cmd = "python " + temp_py_file;
    int ret = std::system(cmd.c_str());

    remove(temp_sql_file.c_str());
    remove(temp_py_file.c_str());
    return (ret == 0);
}

std::string NeonClient::upsert_source_record(
    const std::string& source_code,
    const std::string& source_record_id,
    const std::string& record_type
) {
    std::stringstream ss;
    ss << "INSERT INTO source_records (source_id, source_record_id, record_type) "
       << "SELECT source_id, '" << escape_sql(source_record_id) << "', '" << escape_sql(record_type) << "' "
       << "FROM sources WHERE code = '" << escape_sql(source_code) << "' "
       << "ON CONFLICT (source_id, source_record_id) DO UPDATE SET last_seen_at = NOW();";

    execute_sql(ss.str());
    return source_record_id;
}

bool NeonClient::record_snapshot(const models::SourceSnapshot& snap) {
    std::stringstream ss;
    ss << "INSERT INTO source_snapshots "
       << "(source_record_id, source_code, source_time, content_type, drive_file_id, drive_web_link, "
       << "sha256, http_status, parser_version, schema_version, normalized_payload) "
       << "VALUES ("
       << "'" << escape_sql(snap.source_record_id) << "', "
       << "'" << escape_sql(snap.source_code) << "', "
       << "NOW(), "
       << "'" << escape_sql(snap.content_type) << "', "
       << "'" << escape_sql(snap.drive_file_id) << "', "
       << "'" << escape_sql(snap.drive_web_link) << "', "
       << "'" << escape_sql(snap.sha256) << "', "
       << snap.http_status << ", "
       << "'" << escape_sql(snap.parser_version) << "', "
       << "'" << escape_sql(snap.schema_version) << "', "
       << "'" << escape_sql(snap.normalized_payload_json) << "'::jsonb"
       << ");";

    return execute_sql(ss.str());
}

bool NeonClient::record_evidence_artifact(const models::EvidenceArtifact& art) {
    std::stringstream ss;
    ss << "INSERT INTO evidence_artifacts "
       << "(source_id, source_record_id, artifact_type, drive_file_id, drive_url, sha256, "
       << "mime_type, file_size, acceptance_status, rejection_reason) "
       << "SELECT source_id, '" << escape_sql(art.source_record_id) << "', '" << escape_sql(art.artifact_type) << "', "
       << "'" << escape_sql(art.drive_file_id) << "', '" << escape_sql(art.drive_url) << "', '" << escape_sql(art.sha256) << "', "
       << "'" << escape_sql(art.mime_type) << "', " << art.file_size << ", "
       << "'" << escape_sql(art.acceptance_status) << "', '" << escape_sql(art.rejection_reason) << "' "
       << "FROM sources WHERE code = 'PARIVESH' "
       << "ON CONFLICT (sha256) DO UPDATE SET "
       << "acceptance_status = EXCLUDED.acceptance_status, "
       << "rejection_reason = EXCLUDED.rejection_reason;";

    return execute_sql(ss.str());
}

bool NeonClient::record_workflow_event(const models::CanonicalEvent& ev) {
    std::stringstream ss;
    ss << "INSERT INTO workflow_events "
       << "(event_id, event_type, aggregate_type, aggregate_id, source_system, source_record_id, "
       << "payload, evidence_refs, checksum, status) "
       << "VALUES ("
       << "'" << escape_sql(ev.event_id) << "', "
       << "'" << escape_sql(ev.event_type) << "', "
       << "'" << escape_sql(ev.aggregate_type) << "', "
       << "'" << escape_sql(ev.aggregate_id) << "', "
       << "'" << escape_sql(ev.source_system) << "', "
       << "'" << escape_sql(ev.source_record_id) << "', "
       << "'" << escape_sql(ev.payload_json) << "'::jsonb, "
       << "'" << escape_sql(ev.evidence_refs_json) << "'::jsonb, "
       << "'" << escape_sql(ev.checksum) << "', "
       << "'" << escape_sql(ev.status) << "'"
       << ") ON CONFLICT (event_id) DO NOTHING;";

    return execute_sql(ss.str());
}

bool NeonClient::record_exception(const models::EvidenceException& ex) {
    std::stringstream ss;
    ss << "INSERT INTO exceptions "
       << "(code, severity, aggregate_type, aggregate_id, source_record_id, reason, evidence_ref) "
       << "VALUES ("
       << "'" << escape_sql(ex.code) << "', "
       << "'" << escape_sql(ex.severity) << "', "
       << "'" << escape_sql(ex.aggregate_type) << "', "
       << "'" << escape_sql(ex.aggregate_id) << "', "
       << "'" << escape_sql(ex.source_record_id) << "', "
       << "'" << escape_sql(ex.reason) << "', "
       << "'" << escape_sql(ex.evidence_ref) << "'"
       << ");";

    return execute_sql(ss.str());
}

bool NeonClient::update_source_health(
    const std::string& source_code,
    const std::string& status,
    int64_t lag_seconds,
    bool success
) {
    std::stringstream ss;
    ss << "UPDATE source_health "
       << "SET status = '" << escape_sql(status) << "', "
       << "current_lag_seconds = " << lag_seconds << ", "
       << "last_attempt_at = NOW(), "
       << (success ? "last_success_at = NOW(), consecutive_failures = 0, records_seen = records_seen + 1 "
                   : "consecutive_failures = consecutive_failures + 1 ")
       << "WHERE source_id = (SELECT source_id FROM sources WHERE code = '" << escape_sql(source_code) << "');";

    return execute_sql(ss.str());
}

std::vector<models::SourceHealth> NeonClient::get_all_source_health() {
    std::vector<models::SourceHealth> results;
    return results;
}

} // namespace storage
} // namespace dharti
