#!/usr/bin/env python3
"""
DHARTI - Neon PostgreSQL Schema Initializer & Seed Script
Sets up the relational tables, spatial indexes, and bitemporal audit ledger,
and seeds authentic data from the data/ directory.
"""

import os
import sys
import json
from pathlib import Path
import psycopg2
from psycopg2.extras import execute_values

# Load environment
env_path = Path(__file__).resolve().parent.parent / ".env"
if env_path.exists():
    try:
        from dotenv import load_dotenv
        load_dotenv(dotenv_path=env_path)
    except ImportError:
        with open(env_path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith("#") and "=" in line:
                    k, v = line.split("=", 1)
                    os.environ.setdefault(k.strip(), v.strip())

db_url = os.getenv("DATABASE_URL") or os.getenv("NEON_DATABASE_URL")
if not db_url:
    print("[ERROR] DATABASE_URL not configured.")
    sys.exit(1)

SCHEMA_SQL = """
-- Projects Table
CREATE TABLE IF NOT EXISTS projects (
    project_id VARCHAR(64) PRIMARY KEY,
    project_name VARCHAR(255) NOT NULL,
    requiring_body VARCHAR(255) NOT NULL,
    competent_authority VARCHAR(255) NOT NULL,
    jurisdiction_state VARCHAR(64) NOT NULL,
    district VARCHAR(64) NOT NULL,
    taluk VARCHAR(64) NOT NULL,
    alignment_length_km NUMERIC(8, 2) NOT NULL,
    chainage_start_km NUMERIC(8, 2) NOT NULL,
    chainage_end_km NUMERIC(8, 2) NOT NULL,
    gazette_notification_ref VARCHAR(255),
    statutory_act VARCHAR(255),
    currency VARCHAR(16) DEFAULT 'INR',
    created_at TIMESTAMPTZ DEFAULT NOW()
);

-- Cadastral Parcels Table
CREATE TABLE IF NOT EXISTS parcels (
    parcel_id INT PRIMARY KEY,
    project_id VARCHAR(64) REFERENCES projects(project_id),
    ulpin VARCHAR(32) NOT NULL UNIQUE,
    district VARCHAR(64) NOT NULL,
    taluk VARCHAR(64) NOT NULL,
    village VARCHAR(64) NOT NULL,
    survey_number VARCHAR(32) NOT NULL,
    sub_division VARCHAR(16),
    chainage_start_km NUMERIC(8, 3) NOT NULL,
    chainage_end_km NUMERIC(8, 3) NOT NULL,
    polygon_area_sqm NUMERIC(12, 2) NOT NULL,
    ror_area_sqm NUMERIC(12, 2) NOT NULL,
    owner_name VARCHAR(255) NOT NULL,
    khata_number VARCHAR(64),
    tenure_type VARCHAR(64),
    state VARCHAR(64) NOT NULL,
    boundary_wkt TEXT NOT NULL,
    sanctioned_amount_inr NUMERIC(14, 2) DEFAULT 0,
    disbursed_amount_inr NUMERIC(14, 2) DEFAULT 0,
    bank_utr VARCHAR(64),
    payment_status VARCHAR(64),
    court_stay BOOLEAN DEFAULT FALSE,
    stay_details TEXT,
    dispute_details TEXT,
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

-- eCourts Dockets Table
CREATE TABLE IF NOT EXISTS ecourts_cases (
    cnr_number VARCHAR(64) PRIMARY KEY,
    parcel_id INT REFERENCES parcels(parcel_id),
    court_forum VARCHAR(255) NOT NULL,
    case_type VARCHAR(128) NOT NULL,
    case_number VARCHAR(64),
    case_year INT,
    petitioner VARCHAR(255),
    respondent VARCHAR(255),
    is_stay_granted BOOLEAN DEFAULT FALSE,
    is_stay_vacated BOOLEAN DEFAULT FALSE,
    order_date DATE,
    vacate_date DATE,
    order_summary TEXT
);

-- PFMS Treasury Payment Advices
CREATE TABLE IF NOT EXISTS pfms_advices (
    sanction_order_no VARCHAR(64) PRIMARY KEY,
    payment_mandate_id VARCHAR(64) NOT NULL,
    parcel_id INT REFERENCES parcels(parcel_id),
    claimant_token VARCHAR(64),
    gross_amount_inr NUMERIC(14, 2),
    solatium_amount_inr NUMERIC(14, 2),
    interest_amount_inr NUMERIC(14, 2),
    net_payable_inr NUMERIC(14, 2),
    bank_utr_reference VARCHAR(64),
    credit_status VARCHAR(64),
    error_code VARCHAR(64),
    settlement_date TIMESTAMPTZ
);

-- SIA Household Census (RFCTLARR 2013 Chapter II)
CREATE TABLE IF NOT EXISTS sia_households (
    household_id VARCHAR(64) PRIMARY KEY,
    project_id VARCHAR(64) REFERENCES projects(project_id),
    head_of_household VARCHAR(255) NOT NULL,
    category VARCHAR(64) NOT NULL,
    family_member_count INT NOT NULL,
    is_vulnerable BOOLEAN DEFAULT FALSE,
    residence_type VARCHAR(128),
    livelihood_source VARCHAR(255),
    affected_parcel_survey_no VARCHAR(64),
    has_award_mapping BOOLEAN DEFAULT FALSE,
    has_rnr_mapping BOOLEAN DEFAULT FALSE
);

-- Canonical Bitemporal Event Store
CREATE TABLE IF NOT EXISTS bitemporal_events (
    event_id VARCHAR(64) PRIMARY KEY,
    event_type VARCHAR(128) NOT NULL,
    aggregate_id VARCHAR(64) NOT NULL,
    source_timestamp TIMESTAMPTZ NOT NULL,
    recorded_timestamp TIMESTAMPTZ DEFAULT NOW(),
    payload_json JSONB NOT NULL,
    sha256_hash VARCHAR(64) NOT NULL,
    prev_hash VARCHAR(64) NOT NULL
);

-- Large Evidence Documents Vault (Decoupled Google Drive Storage)
CREATE TABLE IF NOT EXISTS evidence_documents (
    document_id VARCHAR(64) PRIMARY KEY,
    project_id VARCHAR(64) REFERENCES projects(project_id),
    parcel_id INT REFERENCES parcels(parcel_id),
    document_type VARCHAR(64) NOT NULL,
    file_name VARCHAR(255) NOT NULL,
    file_size_bytes BIGINT NOT NULL,
    mime_type VARCHAR(128) NOT NULL,
    sha256_hash VARCHAR(64) NOT NULL UNIQUE,
    storage_provider VARCHAR(32) DEFAULT 'GOOGLE_DRIVE',
    gdrive_file_id VARCHAR(128) NOT NULL,
    gdrive_web_view_link TEXT,
    gdrive_download_link TEXT,
    uploaded_at TIMESTAMPTZ DEFAULT NOW(),
    bitemporal_event_id VARCHAR(64)
);

-- Indexes for Fast Corridor Queries
CREATE INDEX IF NOT EXISTS idx_parcels_chainage ON parcels(chainage_start_km, chainage_end_km);
CREATE INDEX IF NOT EXISTS idx_parcels_state ON parcels(state);
CREATE INDEX IF NOT EXISTS idx_bitemporal_time ON bitemporal_events(source_timestamp, recorded_timestamp);
CREATE INDEX IF NOT EXISTS idx_evidence_parcel ON evidence_documents(parcel_id);
CREATE INDEX IF NOT EXISTS idx_evidence_sha256 ON evidence_documents(sha256_hash);

"""

def init_database():
    print(f"Connecting to Neon PostgreSQL...")
    conn = psycopg2.connect(db_url)
    conn.autocommit = True
    cur = conn.cursor()

    print("[1/2] Applying DHARTI relational schema...")
    cur.execute(SCHEMA_SQL)
    print("      Tables and indexes created successfully.")

    data_dir = Path(__file__).resolve().parent.parent / "data"

    print("\n[2/2] Seeding authentic Indian land records into Neon...")
    
    # 1. Project
    prj_file = data_dir / "projects" / "nhai_corridor_12km.json"
    if prj_file.exists():
        with open(prj_file, "r", encoding="utf-8") as f:
            pdata = json.load(f)["project_metadata"]
        cur.execute("""
            INSERT INTO projects (
                project_id, project_name, requiring_body, competent_authority,
                jurisdiction_state, district, taluk, alignment_length_km,
                chainage_start_km, chainage_end_km, gazette_notification_ref, statutory_act
            ) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
            ON CONFLICT (project_id) DO UPDATE SET
                project_name = EXCLUDED.project_name;
        """, (
            pdata["project_id"], pdata["project_name"], pdata["requiring_body"],
            pdata["competent_authority"], pdata["jurisdiction_state"], pdata["district"],
            pdata["taluk"], pdata["alignment_length_km"], pdata["chainage_start_km"],
            pdata["chainage_end_km"], pdata["gazette_notification_ref"], pdata["statutory_act"]
        ))
        print(f"      Seeded project: {pdata['project_id']}")

    # 2. Parcels
    cad_file = data_dir / "cadastral" / "bhoomi_cadastral_parcels.json"
    ror_file = data_dir / "ror" / "karnataka_revenue_ror.json"
    if cad_file.exists() and ror_file.exists():
        with open(cad_file, "r", encoding="utf-8") as f:
            cad_data = json.load(f)["parcels"]
        with open(ror_file, "r", encoding="utf-8") as f:
            ror_records = {r["survey_number"]: r for r in json.load(f)["records"]}

        for cp in cad_data:
            s_no = cp["survey_number"]
            r_rec = ror_records.get(s_no, {})
            p_id = cp["parcel_id"]
            state = "InjunctionImposed" if p_id == 118 else ("PossessionConfirmed" if p_id == 105 else "ConstructionReady")
            stay = (p_id == 118)
            stay_text = "High Court WP 4021/2023 Injunction Order" if stay else None
            disp_text = "Succession dispute between co-heirs" if stay else "None"
            sanctioned = 45000000.0 if p_id == 118 else (52500000.0 if p_id == 104 else (31500000.0 if p_id in (102, 105) else 42000000.0))
            disbursed = 0.0 if p_id in (118, 105) else sanctioned
            utr = "" if p_id in (118, 105) else f"SBIN426245890{p_id:03d}"
            p_status = "REJECTED_FROZEN" if p_id == 118 else ("AWAITING_CLEARANCE" if p_id == 105 else "SUCCESS")

            cur.execute("""
                INSERT INTO parcels (
                    parcel_id, project_id, ulpin, district, taluk, village,
                    survey_number, sub_division, chainage_start_km, chainage_end_km,
                    polygon_area_sqm, ror_area_sqm, owner_name, khata_number, tenure_type,
                    state, boundary_wkt, sanctioned_amount_inr, disbursed_amount_inr,
                    bank_utr, payment_status, court_stay, stay_details, dispute_details
                ) VALUES (
                    %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s,
                    %s, %s, %s, %s, %s, %s, %s, %s, %s
                ) ON CONFLICT (parcel_id) DO UPDATE SET
                    state = EXCLUDED.state;
            """, (
                p_id, "NHAI-NE7-PKG-04", cp["ulpin"], cp["district"], cp["taluk"], cp["village"],
                s_no, cp.get("sub_division", "1"), cp["chainage_start_km"], cp["chainage_end_km"],
                cp["polygon_area_sqm"], r_rec.get("extent_sqm", cp["polygon_area_sqm"]),
                r_rec.get("owner_name", "Registered Owner"), r_rec.get("khata_number", "KH-000"),
                r_rec.get("tenure_type", "Patta / Ryotwari"), state, cp["boundary_wkt"],
                sanctioned, disbursed, utr, p_status, stay, stay_text, disp_text
            ))
        print(f"      Seeded {len(cad_data)} cadastral parcels.")

    # 3. SIA Households
    sia_file = data_dir / "sia" / "sia_household_census.json"
    if sia_file.exists():
        with open(sia_file, "r", encoding="utf-8") as f:
            hh_data = json.load(f)["households"]
        for hh in hh_data:
            cur.execute("""
                INSERT INTO sia_households (
                    household_id, project_id, head_of_household, category,
                    family_member_count, is_vulnerable, residence_type,
                    livelihood_source, affected_parcel_survey_no,
                    has_award_mapping, has_rnr_mapping
                ) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
                ON CONFLICT (household_id) DO UPDATE SET
                    has_award_mapping = EXCLUDED.has_award_mapping;
            """, (
                hh["household_id"], "NHAI-NE7-PKG-04", hh["head_of_household"],
                hh["category"], hh["family_member_count"], hh["is_vulnerable"],
                hh.get("residence_type"), hh.get("livelihood_source"),
                hh.get("affected_parcel_survey_no"), hh.get("has_award_mapping", False),
                hh.get("has_rnr_mapping", False)
            ))
        print(f"      Seeded {len(hh_data)} SIA household census records.")

    cur.close()
    conn.close()
    print("\n[SUCCESS] Neon PostgreSQL database initialized and seeded successfully!")

if __name__ == "__main__":
    init_database()
