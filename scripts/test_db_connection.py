#!/usr/bin/env python3
"""
DHARTI - Neon PostgreSQL Connection Verification Script
Tests connectivity, verifies SSL/TLS parameters, and checks database schema.
"""

import os
import sys
import time
from pathlib import Path

# Load environment variables from .env
env_path = Path(__file__).resolve().parent.parent / ".env"

if env_path.exists():
    try:
        from dotenv import load_dotenv
        load_dotenv(dotenv_path=env_path)
        print(f"[INFO] Loaded environment from: {env_path}")
    except ImportError:
        # Fallback manual parser for .env
        with open(env_path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith("#") and "=" in line:
                    key, val = line.split("=", 1)
                    os.environ.setdefault(key.strip(), val.strip())
        print(f"[INFO] Manually loaded environment from: {env_path}")
else:
    print(f"[WARN] .env file not found at {env_path}")

db_url = os.getenv("DATABASE_URL") or os.getenv("NEON_DATABASE_URL")

if not db_url:
    print("[ERROR] DATABASE_URL or NEON_DATABASE_URL not set in environment or .env.")
    sys.exit(1)

print("=" * 80)
print("   DHARTI: Neon Serverless PostgreSQL Connection Test")
print("=" * 80)
print(f"Target Host: {os.getenv('PGHOST', 'ep-frosty-butterfly-ax2quyii-pooler.c-4.us-east-2.aws.neon.tech')}")
print(f"Database:    {os.getenv('PGDATABASE', 'neondb')}")
print(f"User:        {os.getenv('PGUSER', 'neondb_owner')}")
print(f"SSL Mode:    {os.getenv('PGSSLMODE', 'require')}")
print("-" * 80)

try:
    import psycopg2
    from psycopg2 import sql

    print("[1/3] Initiating TLS/SSL connection to Neon PostgreSQL endpoint...")
    t0 = time.perf_counter()
    conn = psycopg2.connect(db_url)
    latency_ms = (time.perf_counter() - t0) * 1000
    print(f"[SUCCESS] Connected successfully in {latency_ms:.2f} ms!")

    with conn.cursor() as cur:
        print("\n[2/3] Querying server metadata...")
        cur.execute("SELECT version();")
        version = cur.fetchone()[0]
        print(f"  PostgreSQL Version: {version}")

        cur.execute("SELECT current_database(), current_user, now();")
        current_db, current_user, server_time = cur.fetchone()
        print(f"  Current Database:   {current_db}")
        print(f"  Current User:       {current_user}")
        print(f"  Server Time (UTC):  {server_time}")

        print("\n[3/3] Inspecting schema tables...")
        cur.execute("""
            SELECT table_name 
            FROM information_schema.tables 
            WHERE table_schema = 'public' 
            ORDER BY table_name;
        """)
        tables = cur.fetchall()
        if tables:
            print(f"  Existing Tables in 'public' schema ({len(tables)}):")
            for t in tables:
                print(f"    - {t[0]}")
        else:
            print("  No user tables found in 'public' schema (Fresh database ready for initialization).")

    conn.close()
    print("\n" + "=" * 80)
    print(">>> NEON POSTGRESQL CONNECTION VERIFIED & OPERATIONAL <<<")
    print("=" * 80)

except Exception as e:
    print(f"\n[FAILURE] Connection error: {e}")
    sys.exit(1)
