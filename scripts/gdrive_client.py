#!/usr/bin/env python3
"""
DHARTI - Google Drive Integration Client
Supports:
  1. Google Apps Script Webhook (Zero Google Cloud Console / API Setup)
  2. Google Drive v3 API via Service Account or OAuth2 Flow

Provides file upload, streaming download, and SHA-256 cryptographic verification.
"""

import os
import sys
import json
import base64
import hashlib
import mimetypes
from pathlib import Path
from typing import Optional, Dict, Any, List

# Load environment
env_path = Path(__file__).resolve().parent.parent / ".env"
if env_path.exists():
    try:
        from dotenv import load_dotenv
        load_dotenv(dotenv_path=env_path)
    except ImportError:
        pass

import requests

class GDriveClient:
    def __init__(self, credentials_path: Optional[str] = None):
        self.webhook_url = os.getenv("GDRIVE_WEBHOOK_URL")
        self.credentials_path = credentials_path or os.getenv("GDRIVE_CREDENTIALS_PATH")
        self.token_path = os.getenv("GDRIVE_TOKEN_PATH", str(Path(__file__).resolve().parent.parent / "token.json"))
        self.vault_folder_id = os.getenv("GDRIVE_VAULT_FOLDER_ID")
        self.mode = "WEBHOOK" if self.webhook_url else "API"
        self.service = None

        if self.mode == "WEBHOOK":
            print(f"[INFO] Using Direct Google Drive Webhook endpoint (No GCP API required).")
        else:
            self._authenticate_api()

    def _authenticate_api(self):
        """Authenticates via Google Cloud API (OAuth2 or Service Account)."""
        from googleapiclient.discovery import build
        from google.oauth2 import service_account
        from google.oauth2.credentials import Credentials
        from google_auth_oauthlib.flow import InstalledAppFlow
        from google.auth.transport.requests import Request

        SCOPES = ['https://www.googleapis.com/auth/drive']

        # Check Service Account
        sa_path = os.getenv("GOOGLE_APPLICATION_CREDENTIALS") or self.credentials_path
        if sa_path and Path(sa_path).exists():
            try:
                creds = service_account.Credentials.from_service_account_file(sa_path, scopes=SCOPES)
                self.service = build('drive', 'v3', credentials=creds)
                print(f"[INFO] Authenticated via Service Account: {sa_path}")
                return
            except Exception as e:
                print(f"[WARN] Service account error: {e}")

        # Check OAuth token
        creds = None
        if Path(self.token_path).exists():
            try:
                creds = Credentials.from_authorized_user_file(self.token_path, SCOPES)
            except Exception as e:
                print(f"[WARN] Token read error: {e}")

        if not creds or not creds.valid:
            if creds and creds.expired and creds.refresh_token:
                creds.refresh(Request())
            else:
                client_secret = self.credentials_path or "credentials.json"
                if not Path(client_secret).exists():
                    raise FileNotFoundError("Neither GDRIVE_WEBHOOK_URL nor credentials.json was found.")
                flow = InstalledAppFlow.from_client_secrets_file(client_secret, SCOPES)
                creds = flow.run_local_server(port=0)
                with open(self.token_path, 'w', encoding='utf-8') as f:
                    f.write(creds.to_json())

        self.service = build('drive', 'v3', credentials=creds)
        print("[SUCCESS] Connected to Google Drive v3 API.")

    def upload_evidence_file(
        self,
        local_path: str,
        parcel_id: Optional[int] = None,
        description: str = ""
    ) -> Dict[str, Any]:
        """Uploads an evidentiary file to Google Drive and computes its SHA-256 hash."""
        path = Path(local_path)
        if not path.exists():
            raise FileNotFoundError(f"File not found: {local_path}")

        # 1. Compute SHA-256 digest & file metadata
        sha256 = hashlib.sha256()
        file_size = path.stat().st_size
        with open(path, "rb") as f:
            for chunk in iter(lambda: f.read(65536), b""):
                sha256.update(chunk)
        digest = sha256.hexdigest()

        mime_type, _ = mimetypes.guess_type(str(path))
        if not mime_type:
            mime_type = "application/octet-stream"

        # 2. Upload via Webhook
        if self.mode == "WEBHOOK":
            print(f"[INFO] Uploading '{path.name}' via Google Apps Script Webhook...")
            with open(path, "rb") as f:
                b64_content = base64.b64encode(f.read()).decode("utf-8")

            payload = {
                "file_name": path.name,
                "mime_type": mime_type,
                "file_base64": b64_content,
                "description": f"Parcel P-{parcel_id} | SHA256: {digest} | {description}".strip()
            }

            resp = requests.post(self.webhook_url, json=payload, timeout=60)
            if resp.status_code != 200:
                raise RuntimeError(f"Webhook upload failed with HTTP {resp.status_code}: {resp.text}")

            res_json = resp.json()
            if res_json.get("status") != "SUCCESS":
                raise RuntimeError(f"Webhook returned error: {res_json}")

            return {
                "file_id": res_json.get("file_id"),
                "file_name": res_json.get("file_name", path.name),
                "sha256_hash": digest,
                "file_size_bytes": file_size,
                "mime_type": mime_type,
                "web_view_link": res_json.get("web_link"),
                "download_link": f"https://drive.google.com/uc?export=download&id={res_json.get('file_id')}",
                "parcel_id": parcel_id,
                "provider": "GOOGLE_DRIVE_WEBHOOK"
            }

        # 3. Fallback: Google Cloud Drive API v3
        from googleapiclient.http import MediaFileUpload
        folder_id = self.get_or_create_vault_folder()
        metadata = {
            'name': path.name,
            'parents': [folder_id],
            'description': f"Parcel P-{parcel_id} | SHA256: {digest} | {description}".strip()
        }
        media = MediaFileUpload(str(path), resumable=True)
        uploaded = self.service.files().create(
            body=metadata,
            media_body=media,
            fields='id, name, webViewLink, webContentLink, size, mimeType'
        ).execute()

        return {
            "file_id": uploaded.get('id'),
            "file_name": uploaded.get('name'),
            "sha256_hash": digest,
            "file_size_bytes": file_size,
            "mime_type": uploaded.get('mimeType'),
            "web_view_link": uploaded.get('webViewLink'),
            "download_link": uploaded.get('webContentLink'),
            "parcel_id": parcel_id,
            "provider": "GOOGLE_DRIVE_API"
        }

    def get_or_create_vault_folder(self, folder_name: str = "DHARTI_EVIDENCE_VAULT") -> str:
        if self.mode == "WEBHOOK":
            return "DHARTI_EVIDENCE_VAULT"
        
        query = f"name = '{folder_name}' and mimeType = 'application/vnd.google-apps.folder' and trashed = false"
        results = self.service.files().list(q=query, spaces='drive', fields='files(id, name)').execute()
        items = results.get('files', [])
        if items:
            return items[0]['id']
        meta = {'name': folder_name, 'mimeType': 'application/vnd.google-apps.folder'}
        folder = self.service.files().create(body=meta, fields='id').execute()
        return folder.get('id')
