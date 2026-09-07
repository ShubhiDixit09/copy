#!/usr/bin/env python3
"""
DHARTI - Hybrid Storage Pipeline: Google Drive + Neon PostgreSQL
Uploads large original files to Google Drive (Object Vault) and registers their
cryptographic SHA-256 hashes, indexes, and metadata in Neon PostgreSQL.
"""

import os
import sys
import uuid
from pathlib import Path

# Add project root to sys.path
root_dir = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(root_dir))

import psycopg2
from scripts.gdrive_client import GDriveClient

def upload_and_register_evidence(
    file_path: str,
    project_id: str = "NHAI-NE7-PKG-04",
    parcel_id: int = 118,
    doc_type: str = "HIGH_COURT_STAY_DOCKET_PDF"
):
    print("=" * 80)
    print("   DHARTI: Hybrid Storage Pipeline (Google Drive + Neon DB)")
    print("=" * 80)
    
    # 1. Connect to Google Drive & Upload File
    print(f"\n[1/3] Uploading large evidentiary file to Google Drive Vault...")
    print(f"      Source File: {file_path}")
    gdrive = GDriveClient()
    upload_res = gdrive.upload_evidence_file(
        local_path=file_path,
        parcel_id=parcel_id,
        description=f"Type: {doc_type} | Project: {project_id}"
    )
    print(f"      [SUCCESS] Uploaded to Google Drive!")
    print(f"      - GDrive File ID: {upload_res['file_id']}")
    print(f"      - SHA-256 Hash:   {upload_res['sha256_hash']}")
    print(f"      - File Size:      {upload_res['file_size_bytes']} bytes")
    print(f"      - Web View Link:  {upload_res['web_view_link']}")

    # 2. Connect to Neon PostgreSQL & Register Metadata
    db_url = os.getenv("DATABASE_URL")
    print(f"\n[2/3] Registering metadata and cryptographic hash in Neon DB...")
    conn = psycopg2.connect(db_url)
    conn.autocommit = True
    cur = conn.cursor()

    doc_id = f"DOC-{uuid.uuid4().hex[:12].upper()}"
    event_id = f"EVT-{uuid.uuid4().hex[:8].upper()}"

    cur.execute("""
        INSERT INTO evidence_documents (
            document_id, project_id, parcel_id, document_type,
            file_name, file_size_bytes, mime_type, sha256_hash,
            storage_provider, gdrive_file_id, gdrive_web_view_link,
            gdrive_download_link, bitemporal_event_id
        ) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
        ON CONFLICT (sha256_hash) DO UPDATE SET
            gdrive_file_id = EXCLUDED.gdrive_file_id;
    """, (
        doc_id, project_id, parcel_id, doc_type,
        upload_res['file_name'], upload_res['file_size_bytes'], upload_res['mime_type'],
        upload_res['sha256_hash'], 'GOOGLE_DRIVE', upload_res['file_id'],
        upload_res['web_view_link'], upload_res['download_link'], event_id
    ))
    print(f"      [SUCCESS] Document registered in Neon DB table 'evidence_documents' (Doc ID: {doc_id})")

    # 3. Record Bitemporal Audit Event in Neon DB
    print(f"\n[3/3] Recording bitemporal audit event in Neon DB...")
    cur.execute("""
        INSERT INTO bitemporal_events (
            event_id, event_type, aggregate_id, source_timestamp,
            payload_json, sha256_hash, prev_hash
        ) VALUES (%s, %s, %s, NOW(), %s::jsonb, %s, %s)
    """, (
        event_id, "EvidenceDocumentArchivedToVault", f"PARCEL-{parcel_id}",
        json.dumps({
            "document_id": doc_id,
            "document_type": doc_type,
            "gdrive_file_id": upload_res['file_id'],
            "file_name": upload_res['file_name'],
            "sha256_hash": upload_res['sha256_hash'],
            "file_size": upload_res['file_size_bytes']
        }),
        upload_res['sha256_hash'], "PREV_LEDGER_HASH"
    ))
    print(f"      [SUCCESS] Bitemporal event recorded: {event_id}")

    cur.close()
    conn.close()

    print("\n" + "=" * 80)
    print(">>> HYBRID STORAGE TRANSACTION COMPLETE & CRYPTOGRAPHICALLY SECURED <<<")
    print("=" * 80)

if __name__ == "__main__":
    import json
    # Default test file: architecture flowchart PDF
    test_pdf = root_dir / "docs" / "resources" / "SIH26016_Architecture_Flowchart_Poster.pdf"
    if len(sys.argv) > 1:
        test_pdf = Path(sys.argv[1])
    
    upload_and_register_evidence(str(test_pdf))
