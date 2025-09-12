# ESP32 MQTT OTA FSM

## 專案說明
本專案是一個 ESP32 MQTT OTA 範例，使用**狀態機 (FSM)** 控制 OTA 流程，主要功能包括：

1. 連上 WiFi
2. 檢查網路可用性 (Ping / HTTP / DNS)
3. 訂閱 MQTT Topic:
   - `esp32/update` (全域 OTA)
   - `esp32/update/<device_id>` (單一裝置 OTA)
4. 接收 OTA JSON，解析版本號
5. 若有新版韌體，自動下載並更新
6. 更新完成自動重啟
7. 網路不可用時，會自動 Retry

---

## 專案結構

```
esp32_mqtt_ota_fsm/
├── platformio.ini          # PlatformIO 設定檔
├── src/
│   └── main.cpp            # ESP32 狀態機 OTA 程式骨架
└── docs/
    └── fsm.md              # Mermaid 狀態圖
```

---

## 編譯與上傳

1. 開啟 VSCode + PlatformIO
2. 將專案資料夾打開
3. 編譯：`PlatformIO: Build`
4. 上傳到 ESP32：`PlatformIO: Upload`
5. 打開 Serial Monitor 查看輸出

---

## MQTT OTA 使用方式

### Trigger 單一裝置
Topic: `esp32/update/<device_id>`  
Payload: `"update"`

### Trigger 全部裝置
Topic: `esp32/update/all`  
Payload: `"update"`

### OTA JSON 範例 (Controller 發佈)
```json
{
  "version": "1.2.0",
  "url": "http://example.com/firmware.bin",
  "target": "esp32-01"   // 或 "all"
}
```

ESP32 收到 JSON 後，會判斷 target 是否符合自己或 `all`，再進行 OTA。

---

## 注意事項

- 請先修改 `platformio.ini` 與 `main.cpp` 中的 WiFi/MQTT 設定
- OTA URL 必須可直接下載 bin 檔
- 更新完成後 ESP32 會自動重啟
- 若網路失敗，FSM 會自動 retry
