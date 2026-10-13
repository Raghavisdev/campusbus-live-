"""Build database/campusbus.db from schema.sql and seed.sql.

Run from the repo root:
    python database/init_db.py

Re-running it resets the database to the committed mock data.
"""
import sqlite3
import sys
from pathlib import Path

DB_DIR = Path(__file__).resolve().parent


def main():
    db_path = Path(sys.argv[1]) if len(sys.argv) > 1 else DB_DIR / "campusbus.db"
    schema = (DB_DIR / "schema.sql").read_text(encoding="utf-8")
    seed = (DB_DIR / "seed.sql").read_text(encoding="utf-8")

    conn = sqlite3.connect(db_path)
    try:
        conn.executescript(schema)
        conn.executescript(seed)
        conn.commit()
    finally:
        conn.close()

    print(f"Database created at {db_path}")


if __name__ == "__main__":
    main()
