#!/usr/bin/env python3
"""
DHARTI - Google Drive Connection Verification Script
Tests authentication, checks vault folder, and verifies API readiness.
"""

import os
import sys
from pathlib import Path

# Add project root to sys.path
root_dir = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(root_dir))

# Check available credential files
sa_file = root_dir / "service_account.json"
oauth_file = root_dir / "credentials.json"
token_file = root_dir / "token.json"

print("=" * 80)
print("   DHARTI: Google Drive API Connection & Vault Test")
print("=" * 80)

has_creds = False
if sa_file.exists():
    print(f"[FOUND] Google Service Account Key: {sa_file}")
    has_creds = True
elif oauth_file.exists():
    print(f"[FOUND] Google OAuth Client Secret: {oauth_file}")
    has_creds = True
elif token_file.exists():
    print(f"[FOUND] Saved Google OAuth Token: {token_file}")
    has_creds = True
elif os.getenv("GOOGLE_APPLICATION_CREDENTIALS"):
    print(f"[FOUND] GOOGLE_APPLICATION_CREDENTIALS: {os.getenv('GOOGLE_APPLICATION_CREDENTIALS')}")
    has_creds = True

if not has_creds:
    print("\n[ACTION REQUIRED] No Google Drive credentials file detected in project root.")
    print("-" * 80)
    print("To connect your Google Drive, choose one of the two standard methods:\n")
    print("METHOD 1: Personal/Work Google Account (OAuth2 Consent Flow - Recommended)")
    print("  1. Go to Google Cloud Console (https://console.cloud.google.com/)")
    print("  2. Enable 'Google Drive API' in APIs & Services -> Library")
    print("  3. Create an OAuth 2.0 Client ID (Application type: Desktop App)")
    print("  4. Download JSON and save it as 'credentials.json' in this folder:")
    print(f"     {root_dir / 'credentials.json'}")
    print("  5. Run this script again! It will open your browser once to authorize and save token.json.\n")
    print("METHOD 2: Google Cloud Service Account")
    print("  1. In Cloud Console -> IAM & Admin -> Service Accounts -> Create Service Account")
    print("  2. Create & Download Key as JSON -> save as 'service_account.json' in this folder:")
    print(f"     {root_dir / 'service_account.json'}")
    print("  3. Create a folder in your Google Drive (e.g. 'DHARTI_EVIDENCE_VAULT') and share it")
    print("     with your Service Account's email address.")
    print("=" * 80)
    sys.exit(0)

try:
    from scripts.gdrive_client import GDriveClient

    print("\n[1/3] Initializing Google Drive Client & Authenticating...")
    client = GDriveClient()

    print("\n[2/3] Checking / Creating DHARTI Evidence Vault folder...")
    folder_id = client.get_or_create_vault_folder("DHARTI_EVIDENCE_VAULT")
    print(f"  Vault Folder ID: {folder_id}")

    print("\n[3/3] Inspecting Vault files...")
    files = client.list_vault_files()
    print(f"  Current files in vault: {len(files)}")
    for f in files[:5]:
        print(f"    - {f.get('name')} (ID: {f.get('id')}, Size: {f.get('size', 0)} bytes)")

    print("\n" + "=" * 80)
    print(">>> GOOGLE DRIVE API CONNECTED & OPERATIONAL <<<")
    print("=" * 80)

except Exception as e:
    print(f"\n[FAILURE] Connection error: {e}")
    sys.exit(1)
