import paho.mqtt.client as mqtt
import json
import logging
import sqlite3
# import pymysql
# pymysql.install_as_MySQLdb()
from datetime import datetime
from ota_db import init_db, log_update

# ====== 設定 ======
MQTT_BROKER = "127.0.0.1"
MQTT_PORT   = 1883
TOPIC_TRIGGER_BASE = "esp32/update"
TOPIC_PAYLOAD      = "esp32/update"
TOPIC_COMMAND_BASE = "esp32/command"

NEW_VERSION = "1.2.0"
FW_URL      = "http://example.com/"
FW_IMAGE    = "firmware_v1.2.0.bin"

# ====== 資料庫模式: "sqlite" 或 "mysql" ======
DB_MODE = "sqlite"   # 改成 "mysql" 就會用 MySQL

# ====== SQLite 設定 ======
SQLITE_FILE = "ota_history.db"

# ====== MySQL 設定 ======
MYSQL_CONFIG = {
    "host": "localhost",
    "user": "ota_user",
    "password": "ota_password",
    "database": "ota_db"
}

# ====== Logging 設定 ======
LOG_FILE = "ota_controller.log"
logging.basicConfig(
    filename=LOG_FILE,
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s"
)

'''
# ====== 資料庫初始化 ======
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
    if DB_MODE == "sqlite":
        conn = sqlite3.connect(SQLITE_FILE)
        c = conn.cursor()
        c.execute("""
            INSERT INTO ota_log (timestamp, device_id, version, url, image, status)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (datetime.now().isoformat(timespec='seconds'), device_id, version, url, image, status))
        conn.commit()
        conn.close()
    elif DB_MODE == "mysql":
        conn = mysql.connector.connect(**MYSQL_CONFIG)
        cursor = conn.cursor()
        cursor.execute("""
            INSERT INTO ota_log (timestamp, device_id, version, url, image, status)
            VALUES (%s, %s, %s, %s, %s, %s)
        """, (datetime.now(), device_id, version, url, image, status))
        conn.commit()
        cursor.close()
        conn.close()
    logging.info(f"OTA log: {device_id} | {version} | {status}")
'''

# ====== 發送 OTA Payload ======
def publish_ota_payload(client, target="all"):
    payload = json.dumps({
        "version": NEW_VERSION,
        "url": FW_URL,
        "image": FW_IMAGE,
        "target": target
    })
    client.publish(TOPIC_PAYLOAD, payload, qos=1)
    logging.info(f"OTA payload sent to {target}: {payload}")
    log_update(target, NEW_VERSION, FW_URL, FW_IMAGE, "SENT")

# ====== 發送指令 ======
def send_command(client, device_id, command):
    topic = f"{TOPIC_COMMAND_BASE}/{device_id}"
    client.publish(topic, command, qos=1)
    logging.info(f"Command '{command}' sent to {device_id}")

# ====== MQTT Callback ======
def on_connect(client, userdata, flags, reason_code, properties):
    logging.info(f"Connected to MQTT broker, reason_code={reason_code}")
    print(f"Connected to MQTT broker, reason_code={reason_code}")
    # client.subscribe(f"{TOPIC_TRIGGER_BASE}/+")
    client.subscribe(f"{TOPIC_TRIGGER_BASE}/#")
    client.subscribe(f"{TOPIC_COMMAND_BASE}/#")

def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode()
    logging.info(f"Received on {topic}: {payload}")
    print(f"Received on {topic}: {payload}")
    try:
        data = json.loads(payload)
        product = data.get("product", "unknown")
        device_id = data.get("device", "unknown")
    except json.JSONDecodeError as e:
        logging.error(f"Invalid JSON payload: {e}")
        print(f"Invalid JSON payload: {e}")
        return
# OTA 觸發邏輯
    if topic.startswith(TOPIC_TRIGGER_BASE):
        if product != "unknown" and device_id != "unknown":
            logging.info(f"Trigger received for {device_id} with product {product}")
            print(f"Trigger received for {device_id} with product {product}")
            publish_ota_payload(client, target=device_id)

            # 查詢 SQLite 資料庫中的產品資訊
            try:
                conn = sqlite3.connect("ota_history.db")
                cursor = conn.cursor()
                cursor.execute("SELECT * FROM products WHERE product_name = ?", (product,))
                result = cursor.fetchall()
                conn.close()

                if result:
                    for row in result:
                        logging.info(f"Product Info for {product}: {row}")
                        print(f"Product Info for {product}: {row}")
                else:
                    logging.warning(f"No product info found for: {product}")
                    print(f"No product info found for: {product}")
            except sqlite3.Error as e:
                logging.error(f"SQLite error: {e}")
                print(f"SQLite error: {e}")
        else:
            logging.warning("Missing 'product' or 'device' in payload")
            print("Missing 'product' or 'device' in payload")

# ESP32 指令回覆
    elif topic.startswith(TOPIC_COMMAND_BASE):
        if device_id:
            logging.info(f"Command response from {device_id}: {payload}")
            print(f"Command response from {device_id}: {payload}")
        print("TOPIC_COMMAND_BASE END")

'''
    # OTA 觸發
    if topic.startswith(TOPIC_TRIGGER_BASE):
        device_id = topic.split("/")[-1]
        if payload.lower() == "update":
            logging.info(f"Trigger received for {device_id}")
            publish_ota_payload(client, target=device_id)
        elif payload.lower() in ["success", "fail"]:
            log_update(device_id, NEW_VERSION, FW_URL, FW_IMAGE, payload.upper())
            logging.info(f"OTA result from {device_id}: {payload.upper()}")

    # ESP32 回覆指令結果
    elif topic.startswith(TOPIC_COMMAND_BASE):
        device_id = topic.split("/")[-1]
        logging.info(f"Command response from {device_id}: {payload}")
'''

# ====== 主程式 ======
if __name__ == "__main__":
    init_db()

    client = mqtt.Client(
        client_id="OTA_Controller",
        protocol=mqtt.MQTTv5,
        callback_api_version=mqtt.CallbackAPIVersion.VERSION2
    )
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    client.loop_forever()
'''
    # 範例：啟動後 5 秒查詢 esp32-01 韌體版本與狀態
    import threading
    def send_test_commands():
        send_command(client, "esp32-01", "get_version")
        send_command(client, "esp32-01", "get_status")
    threading.Timer(5, send_test_commands).start()
'''
