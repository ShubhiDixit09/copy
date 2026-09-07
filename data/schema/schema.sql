-- =============================================================================
-- DHARTI: Evidence-Backed National Land Acquisition Control Plane
-- 4-Layer Relational Schema & Spatial Indexing for Neon Serverless PostgreSQL
-- =============================================================================

-- Enable PostGIS spatial extension
CREATE EXTENSION IF NOT EXISTS postgis;
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- =============================================================================
-- LAYER 1: SOURCES & SOURCE HEALTH (System Registry & SLA Observability)
-- =============================================================================

CREATE TABLE IF NOT EXISTS sources (
    source_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(64) UNIQUE NOT NULL, -- e.g. 'PARIVESH', 'UP_BHULEKH', 'BHOOMI_RASHI'
    name VARCHAR(255) NOT NULL,
    source_type VARCHAR(64) NOT NULL, -- 'CLEARANCE', 'LAND_RECORD', 'CADASTRAL_GIS', 'ACQUISITION', 'PAYMENT'
    authority VARCHAR(64) NOT NULL CHECK (authority IN ('GOVERNMENT', 'MOCK', 'THIRD_PARTY')),
    polling_interval_seconds INT NOT NULL DEFAULT 1800,
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS source_health (
    source_id UUID PRIMARY KEY REFERENCES sources(source_id) ON DELETE CASCADE,
    last_attempt_at TIMESTAMPTZ,
    last_success_at TIMESTAMPTZ,
    records_seen BIGINT NOT NULL DEFAULT 0,
    records_changed BIGINT NOT NULL DEFAULT 0,
    consecutive_failures INT NOT NULL DEFAULT 0,
    current_lag_seconds BIGINT DEFAULT 0,
    status VARCHAR(32) NOT NULL DEFAULT 'HEALTHY' CHECK (status IN ('HEALTHY', 'DEGRADED', 'OFFLINE')),
    last_error_message TEXT,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- =============================================================================
-- LAYER 2: SOURCE RECORDS (Stable External Identities & Idempotency Keys)
-- =============================================================================

CREATE TABLE IF NOT EXISTS source_records (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    source_id UUID NOT NULL REFERENCES sources(source_id) ON DELETE CASCADE,
    source_record_id VARCHAR(128) NOT NULL, -- e.g. 'IA/KA/NHA/10482/2026'
    record_type VARCHAR(64) NOT NULL,       -- 'ENVIRONMENT_CLEARANCE', 'ROR_RECORD', 'PFMS_MANDATE'
    external_id VARCHAR(128),
    first_seen_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    last_seen_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE (source_id, source_record_id)
);

CREATE INDEX IF NOT EXISTS idx_source_records_source ON source_records(source_id);
CREATE INDEX IF NOT EXISTS idx_source_records_external_id ON source_records(source_id, external_id);
CREATE INDEX IF NOT EXISTS idx_source_records_record_id ON source_records(source_record_id);

-- =============================================================================
-- LAYER 3: SOURCE SNAPSHOTS (Immutable Versioned Evidence Pointers)
-- =============================================================================

CREATE TABLE IF NOT EXISTS source_snapshots (
    snapshot_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    source_record_id VARCHAR(128) NOT NULL,
    source_code VARCHAR(64) NOT NULL,
    retrieved_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    source_time TIMESTAMPTZ NOT NULL,
    content_type VARCHAR(64) NOT NULL,      -- 'application/json', 'application/pdf', 'text/html'
    drive_file_id VARCHAR(128) NOT NULL,    -- Pointer to immutable object in Google Drive
    drive_web_link TEXT,
    sha256 VARCHAR(64) NOT NULL,
    http_status INT NOT NULL DEFAULT 200,
    parser_version VARCHAR(32) NOT NULL DEFAULT 'v1.0.0',
    schema_version VARCHAR(32) NOT NULL DEFAULT 'v1.0.0',
    normalized_payload JSONB NOT NULL,      -- Small structured metadata payload
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_snapshots_record ON source_snapshots(source_record_id);
CREATE INDEX IF NOT EXISTS idx_snapshots_retrieved ON source_snapshots(retrieved_at DESC);
CREATE INDEX IF NOT EXISTS idx_snapshots_sha256 ON source_snapshots(sha256);

-- =============================================================================
-- LAYER 4: EVIDENCE ARTIFACTS & CANONICAL WORKFLOW EVENTS
-- =============================================================================

CREATE TABLE IF NOT EXISTS evidence_artifacts (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    source_id UUID NOT NULL REFERENCES sources(source_id),
    source_record_id VARCHAR(128),
    artifact_type VARCHAR(64) NOT NULL,     -- 'CLEARANCE_LETTER', 'GAZETTE_NOTIF', 'MUTATION_REGISTER', 'COURT_ORDER'
    drive_file_id VARCHAR(128) NOT NULL,
    drive_url TEXT,
    sha256 VARCHAR(64) NOT NULL UNIQUE,
    mime_type VARCHAR(64) NOT NULL,
    file_size BIGINT NOT NULL,
    source_time TIMESTAMPTZ,
    retrieved_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    parser_version VARCHAR(32) DEFAULT 'v1.0.0',
    schema_version VARCHAR(32) DEFAULT 'v1.0.0',
    is_current BOOLEAN NOT NULL DEFAULT TRUE,
    supersedes_artifact_id UUID REFERENCES evidence_artifacts(id),
    acceptance_status VARCHAR(32) NOT NULL CHECK (acceptance_status IN ('PENDING', 'ACCEPTED', 'QUARANTINED', 'REJECTED')),
    rejection_reason TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_evidence_artifacts_sha256 ON evidence_artifacts(sha256);
CREATE INDEX IF NOT EXISTS idx_evidence_artifacts_status ON evidence_artifacts(acceptance_status);
CREATE INDEX IF NOT EXISTS idx_evidence_artifacts_record ON evidence_artifacts(source_record_id);

CREATE TABLE IF NOT EXISTS workflow_events (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    event_id VARCHAR(64) UNIQUE NOT NULL,
    event_type VARCHAR(128) NOT NULL,       -- 'CLEARANCE_APPROVED', 'PARCEL_RECORD_VERIFIED', 'PAYMENT_ACKNOWLEDGED'
    aggregate_type VARCHAR(64) NOT NULL,   -- 'CLEARANCE', 'PARCEL', 'PAYMENT', 'POSSESSION'
    aggregate_id VARCHAR(64) NOT NULL,     -- e.g. 'PARCEL-118' or 'CLEARANCE-IA-10482'
    source_system VARCHAR(64) NOT NULL,
    source_record_id VARCHAR(128),
    source_time TIMESTAMPTZ,
    recorded_time TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    schema_version VARCHAR(32) DEFAULT 'v1.0.0',
    policy_version VARCHAR(32) DEFAULT 'v1.0.0',
    correlation_id VARCHAR(64),
    causation_id VARCHAR(64),
    payload JSONB NOT NULL,
    evidence_refs JSONB,                   -- Array of Google Drive file IDs and SHA256 references
    checksum VARCHAR(64),
    status VARCHAR(32) NOT NULL CHECK (status IN ('PENDING', 'ACCEPTED', 'QUARANTINED', 'REJECTED'))
);

CREATE INDEX IF NOT EXISTS idx_events_aggregate ON workflow_events(aggregate_type, aggregate_id);
CREATE INDEX IF NOT EXISTS idx_events_source_record ON workflow_events(source_system, source_record_id);
CREATE INDEX IF NOT EXISTS idx_events_recorded ON workflow_events(recorded_time DESC);
CREATE INDEX IF NOT EXISTS idx_events_status ON workflow_events(status);

-- =============================================================================
-- DOMAIN ENTITIES & PROJECTIONS (Materialized Views & Active State)
-- =============================================================================

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
    created_at TIMESTAMPTZ DEFAULT NOW()
);

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
    state VARCHAR(64) NOT NULL,
    boundary_wkt TEXT NOT NULL,
    geom GEOMETRY(Polygon, 4326),
    sanctioned_amount_inr NUMERIC(14, 2) DEFAULT 0,
    disbursed_amount_inr NUMERIC(14, 2) DEFAULT 0,
    bank_utr VARCHAR(64),
    payment_status VARCHAR(64),
    court_stay BOOLEAN DEFAULT FALSE,
    clearance_status VARCHAR(64) DEFAULT 'NOT_REQUIRED',
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_parcels_geom ON parcels USING GIST(geom);
CREATE INDEX IF NOT EXISTS idx_parcels_chainage ON parcels(chainage_start_km, chainage_end_km);
CREATE INDEX IF NOT EXISTS idx_parcels_state ON parcels(state);

-- Exceptions Table (Trapped Contradictions & Quarantined Incidents)
CREATE TABLE IF NOT EXISTS exceptions (
    exception_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(64) NOT NULL, -- e.g. 'RULE_5_IMPOSSIBLE_AREA_JUMP', 'COURT_STAY_ACTIVE'
    severity VARCHAR(32) NOT NULL CHECK (severity IN ('LOW', 'MEDIUM', 'HIGH', 'CRITICAL')),
    aggregate_type VARCHAR(64) NOT NULL,
    aggregate_id VARCHAR(64) NOT NULL,
    source_record_id VARCHAR(128),
    reason TEXT NOT NULL,
    evidence_ref VARCHAR(128), -- Google Drive file ID
    is_resolved BOOLEAN NOT NULL DEFAULT FALSE,
    resolved_at TIMESTAMPTZ,
    resolution_notes TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_exceptions_resolved ON exceptions(is_resolved, severity);
CREATE INDEX IF NOT EXISTS idx_exceptions_aggregate ON exceptions(aggregate_type, aggregate_id);

-- Corridor Readiness Projections (Computed Continuously)
CREATE TABLE IF NOT EXISTS readiness_projections (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    project_id VARCHAR(64) REFERENCES projects(project_id),
    total_corridor_km NUMERIC(8, 2) NOT NULL,
    total_ready_km NUMERIC(8, 2) NOT NULL,
    longest_continuous_frontage_km NUMERIC(8, 2) NOT NULL,
    pci_percentage NUMERIC(6, 2) NOT NULL,
    ready_parcel_count INT NOT NULL,
    blocked_parcel_count INT NOT NULL,
    last_computed_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
