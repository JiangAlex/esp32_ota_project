import sqlite3
# import mysql.connector
import pymysql
pymysql.install_as_MySQLdb()
from datetime import datetime

# ====== 設定 ======
DB_MODE = "sqlite"  # 或 "mysql"
SQLITE_FILE = "ota_history.db"

MYSQL_CONFIG = {
    "host": "localhost",
    "user": "ota_user",
    "password": "ota_password",
    "database": "ota_db"
}

def init_db():
    if DB_MODE == "sqlite":
        conn = sqlite3.connect(SQLITE_FILE)
        c = conn.cursor()
        c.execute("""
            CREATE TABLE IF NOT EXISTS ota_log (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT,
                device_id TEXT,
                version TEXT,
                url TEXT,
                image TEXT,
                status TEXT
            )
        """)
        conn.commit()
        conn.close()
    elif DB_MODE == "mysql":
        conn = mysql.connector.connect(**MYSQL_CONFIG)
        cursor = conn.cursor()
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS ota_log (
                id INT AUTO_INCREMENT PRIMARY KEY,
                timestamp DATETIME,
                device_id VARCHAR(64),
                version VARCHAR(32),
                url TEXT,
                image VARCHAR(128),
                status VARCHAR(32)
            )
        """)
        conn.commit()
        cursor.close()
        conn.close()

def log_update(device_id, version, url, image, status):
    now = datetime.now()
    if DB_MODE == "sqlite":
        conn = sqlite3.connect(SQLITE_FILE)
        c = conn.cursor()
        c.execute("""
            INSERT INTO ota_log (timestamp, device_id, version, url, image, status)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (now.isoformat(timespec='seconds'), device_id, version, url, image, status))
        conn.commit()
        conn.close()
    elif DB_MODE == "mysql":
        conn = mysql.connector.connect(**MYSQL_CONFIG)
        cursor = conn.cursor()
        cursor.execute("""
            INSERT INTO ota_log (timestamp, device_id, version, url, image, status)
            VALUES (%s, %s, %s, %s, %s, %s)
        """, (now, device_id, version, url, image, status))
        conn.commit()
        cursor.close()
        conn.close()
