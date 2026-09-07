#!/usr/bin/env python3
"""
DHARTI - Google Drive Integration Client
Supports Service Account authentication and OAuth2 User Consent Flow.
Provides file upload, streaming download, folder management, and SHA-256 verification.
"""

import os
import sys
import hashlib
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

from googleapiclient.discovery import build
from googleapiclient.http import MediaFileUpload, MediaIoBaseDownload
from google.oauth2 import service_account
from google.oauth2.credentials import Credentials
from google_auth_oauthlib.flow import InstalledAppFlow
from google.auth.transport.requests import Request

SCOPES = ['https://www.googleapis.com/auth/drive']

class GDriveClient:
    def __init__(self, credentials_path: Optional[str] = None):
        self.creds = None
        self.service = None
        self.credentials_path = credentials_path or os.getenv("GDRIVE_CREDENTIALS_PATH")
        self.token_path = os.getenv("GDRIVE_TOKEN_PATH", str(Path(__file__).resolve().parent.parent / "token.json"))
        self.vault_folder_id = os.getenv("GDRIVE_VAULT_FOLDER_ID")
        self._authenticate()

    def _authenticate(self):
        """Authenticates via Service Account (if provided) or OAuth2 Flow."""
        # 1. Check for Service Account
        sa_path = os.getenv("GOOGLE_APPLICATION_CREDENTIALS") or self.credentials_path
        if sa_path and Path(sa_path).exists():
            try:
                self.creds = service_account.Credentials.from_service_account_file(
                    sa_path, scopes=SCOPES
                )
                self.service = build('drive', 'v3', credentials=self.creds)
                print(f"[INFO] Authenticated to Google Drive via Service Account: {sa_path}")
                return
            except Exception as e:
                print(f"[WARN] Failed to authenticate with service account: {e}")

        # 2. Check for existing OAuth2 Token
        if Path(self.token_path).exists():
            try:
                self.creds = Credentials.from_authorized_user_file(self.token_path, SCOPES)
            except Exception as e:
                print(f"[WARN] Error reading token.json: {e}")

        # 3. Refresh or prompt for OAuth2 login
        if not self.creds or not self.creds.valid:
            if self.creds and self.creds.expired and self.creds.refresh_token:
                try:
                    self.creds.refresh(Request())
                    print("[INFO] Google Drive OAuth2 token refreshed.")
                except Exception as e:
                    print(f"[WARN] Failed to refresh token: {e}")
                    self.creds = None

            if not self.creds:
                client_secret = self.credentials_path or "credentials.json"
                if not Path(client_secret).exists():
                    raise FileNotFoundError(
                        f"No Google Drive credentials found. Please provide a 'credentials.json' (OAuth client ID) "
                        f"or 'service_account.json' (Service Account Key) in the project root or configure "
                        f"GDRIVE_CREDENTIALS_PATH in .env."
                    )
                flow = InstalledAppFlow.from_client_secrets_file(client_secret, SCOPES)
                print("[INFO] Launching local browser for Google Drive user authorization...")
                self.creds = flow.run_local_server(port=0)
                # Save token for next time
                with open(self.token_path, 'w', encoding='utf-8') as token_file:
                    token_file.write(self.creds.to_json())
                print(f"[SUCCESS] OAuth token saved to {self.token_path}")

        self.service = build('drive', 'v3', credentials=self.creds)
        print("[SUCCESS] Connected to Google Drive API (v3).")

    def get_or_create_vault_folder(self, folder_name: str = "DHARTI_EVIDENCE_VAULT") -> str:
        """Finds or creates the root vault folder in Google Drive."""
        if self.vault_folder_id:
            return self.vault_folder_id

        query = f"name = '{folder_name}' and mimeType = 'application/vnd.google-apps.folder' and trashed = false"
        results = self.service.files().list(q=query, spaces='drive', fields='files(id, name)').execute()
        items = results.get('files', [])

        if items:
            self.vault_folder_id = items[0]['id']
            print(f"[INFO] Found existing Google Drive Vault: '{folder_name}' (ID: {self.vault_folder_id})")
        else:
            file_metadata = {
                'name': folder_name,
                'mimeType': 'application/vnd.google-apps.folder',
                'description': 'DHARTI National Land Acquisition Evidence Vault'
            }
            folder = self.service.files().create(body=file_metadata, fields='id').execute()
            self.vault_folder_id = folder.get('id')
            print(f"[INFO] Created new Google Drive Vault: '{folder_name}' (ID: {self.vault_folder_id})")

        return self.vault_folder_id

    def upload_evidence_file(self, local_path: str, parcel_id: Optional[int] = None, description: str = "") -> Dict[str, Any]:
        """Uploads a large evidentiary file to Google Drive and computes its SHA-256 hash."""
        path = Path(local_path)
        if not path.exists():
            raise FileNotFoundError(f"File not found: {local_path}")

        # Compute SHA-256 digest
        sha256 = hashlib.sha256()
        file_size = path.stat().st_size
        with open(path, "rb") as f:
            for chunk in iter(lambda: f.read(65536), b""):
                sha256.update(chunk)
        digest = sha256.hexdigest()

        folder_id = self.get_or_create_vault_folder()

        file_metadata = {
            'name': path.name,
            'parents': [folder_id],
            'description': f"DHARTI Evidence | Parcel P-{parcel_id} | SHA256: {digest} | {description}".strip()
        }

        media = MediaFileUpload(str(path), resumable=True)
        uploaded = self.service.files().create(
            body=file_metadata,
            media_body=media,
            fields='id, name, webViewLink, webContentLink, size, mimeType'
        ).execute()

        return {
            'file_id': uploaded.get('id'),
            'file_name': uploaded.get('name'),
            'sha256_hash': digest,
            'file_size_bytes': file_size,
            'mime_type': uploaded.get('mimeType'),
            'web_view_link': uploaded.get('webViewLink'),
            'download_link': uploaded.get('webContentLink'),
            'parcel_id': parcel_id
        }

    def list_vault_files(self) -> List[Dict[str, Any]]:
        """Lists all evidence files inside the DHARTI vault."""
        folder_id = self.get_or_create_vault_folder()
        query = f"'{folder_id}' in parents and trashed = false"
        results = self.service.files().list(
            q=query,
            spaces='drive',
            fields='files(id, name, mimeType, size, webViewLink, createdTime, description)'
        ).execute()
        return results.get('files', [])
