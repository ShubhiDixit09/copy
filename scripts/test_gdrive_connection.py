#!/usr/bin/env python3
"""
DHARTI - Google Drive Connection Verification Script
Tests Webhook or API authentication, verifies vault status, and reports connectivity.
"""

import os
import sys
from pathlib import Path

# Add project root to sys.path
root_dir = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(root_dir))

# Load .env
env_path = root_dir / ".env"
if env_path.exists():
    try:
        from dotenv import load_dotenv
        load_dotenv(dotenv_path=env_path)
    except ImportError:
        pass

# Check available credential files & webhook
webhook_url = os.getenv("GDRIVE_WEBHOOK_URL")
sa_file = root_dir / "service_account.json"
oauth_file = root_dir / "credentials.json"
token_file = root_dir / "token.json"

print("=" * 80)
print("   DHARTI: Google Drive Connection & Vault Test")
print("=" * 80)

has_creds = False
if webhook_url:
    print(f"[FOUND] Google Apps Script Direct Webhook: {webhook_url[:50]}...")
    has_creds = True
elif sa_file.exists():
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
    print("\n[ACTION REQUIRED] No Google Drive webhook or credentials detected.")
    print("-" * 80)
    print("Option 1 (Recommended): Add your Google Apps Script Webhook URL to .env:")
    print("  GDRIVE_WEBHOOK_URL=https://script.google.com/macros/s/.../exec\n")
    print("Option 2: Place 'credentials.json' (Desktop OAuth) or 'service_account.json' in:")
    print(f"  {root_dir}")
    print("=" * 80)
    sys.exit(0)

try:
    from scripts.gdrive_client import GDriveClient

    print("\n[1/2] Initializing Google Drive Client...")
    client = GDriveClient()

    if client.mode == "WEBHOOK":
        print("\n[2/2] Testing direct webhook ping to Google Drive...")
        import base64, requests
        test_payload = {
            "file_name": "dharti_ping_check.txt",
            "mime_type": "text/plain",
            "file_base64": base64.b64encode(b"DHARTI Ping Verified").decode("utf-8"),
            "description": "System connectivity ping"
        }
        resp = requests.post(client.webhook_url, json=test_payload, timeout=30)
        if resp.status_code == 200:
            res_data = resp.json()
            print(f"  [SUCCESS] Webhook Response Status: {res_data.get('status')}")
            print(f"  Vault Target: 'DHARTI_EVIDENCE_VAULT'")
            print(f"  Uploaded Ping File ID: {res_data.get('file_id')}")
            print(f"  Drive Web Link:        {res_data.get('web_link')}")
        else:
            print(f"  [ERROR] Webhook returned HTTP {resp.status_code}: {resp.text}")
            sys.exit(1)
    else:
        print("\n[2/2] Checking Vault folder via Google Drive v3 API...")
        folder_id = client.get_or_create_vault_folder("DHARTI_EVIDENCE_VAULT")
        print(f"  Vault Folder ID: {folder_id}")

    print("\n" + "=" * 80)
    print(">>> GOOGLE DRIVE CONNECTED & OPERATIONAL (ZERO GCP SETUP) <<<")
    print("=" * 80)

except Exception as e:
    print(f"\n[FAILURE] Connection error: {e}")
    sys.exit(1)
