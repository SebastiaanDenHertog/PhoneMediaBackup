CREATE TABLE IF NOT EXISTS files (
    file_name       TEXT PRIMARY KEY,
    location        TEXT NOT NULL,
    original_size   INTEGER NOT NULL,
    compressed_size INTEGER NOT NULL,
    disk_path       TEXT,
    last_access_ts  INTEGER
);
