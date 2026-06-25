# 產品序號（Serial Number）— App 端交接文件

> 對象：JNC BT Mesh App 工程師
> 韌體版本：JNC-BT-Mesh v1.41 之後
> 目的：App 新增「讀／寫產品序號」功能。**傳輸方式與現有「變數設定」完全相同**，只是換兩個新的 Property ID。

---

## 1. 一句話總結

序號的讀寫**沿用你現在做「變數設定」那一套**（Mesh Sensor 模型的 `sensor_client_get_column` / `column_status`），
只要把 Property ID 換成下面兩個新碼、payload 換成 8-byte 序號即可。

| 功能 | Property ID | 值 | 方向 |
|------|-------------|-----|------|
| 讀序號 | `NODE_GET_SERIAL` | **`0x8072`** | App → 裝置請求，裝置回序號 |
| 寫序號 | `NODE_SET_SERIAL` | **`0x8073`** | App → 裝置送序號，裝置回 ack |

> 對照組（你已經實作過的）：`NODE_SENSOR_SETUP_GET = 0x8070`、`NODE_SENSOR_SETUP_SET = 0x8071`。
> 新序號命令就是「比照辦理」。

---

## 2. 序號格式

- **長度：固定 8 bytes**（ASCII 字串，不含結尾 `\0`）。
- **預設值：`"JNC00000"`**。裝置**未燒錄序號時，讀取會回傳此預設值**。
- 槽位另預留 8 bytes（共 16 bytes），目前未使用，App 不需處理。
- 範例："JNC00001" = `4A 4E 43 30 30 30 30 31`。

---

## 3. 寫序號 — `NODE_SET_SERIAL (0x8073)`

**App → 裝置**（用 `sensor_client_get_column`，property = `0x8073`，column data = 8-byte 序號）

| 欄位 | 內容 |
|------|------|
| property_id | `0x8073` |
| payload（column data）| 8 bytes 序號，例如 `4A 4E 43 30 30 30 30 31` ("JNC00001") |

**裝置 → App**（`column_status`，property = `0x8073`，data = 2-byte ack，little-endian）

| ack 值 | 意義 |
|--------|------|
| `0x0000` | 寫入成功 |
| `0x0001` | flash 寫入失敗 |
| `0x0002` | 長度不足（payload < 8 bytes）|

> 寫入後序號**立即生效並永久保存**（存在晶片 USERDATA flash，恢復原廠 / 重燒韌體都不會被清除）。

---

## 4. 讀序號 — `NODE_GET_SERIAL (0x8072)`

**App → 裝置**（用 `sensor_client_get_column`，property = `0x8072`）

| 欄位 | 內容 |
|------|------|
| property_id | `0x8072` |
| payload | 不需要（裝置忽略內容；送空的或 1 byte 皆可）|

**裝置 → App**（`column_status`，property = `0x8072`，data = 10 bytes）

| 位移 | 長度 | 內容 |
|------|------|------|
| 0 | 2 | property_id 回echo = `72 80`（`0x8072`，**little-endian**）|
| 2 | 8 | 序號 8 bytes（未燒錄時為預設 `"JNC00000"`）|

> ⚠️ 注意：前 2 bytes 是 property_id 的 little-endian 回echo，**這與你現在解析 `NODE_SENSOR_SETUP_GET` 回應的格式一致**（裝置端 `*((uint16*)data)=property_id;` 後接 payload）。解析序號時請取 **第 2~9 bytes**。

---

## 5. 與現有「變數設定」的對照（直接複製即可）

你現在的流程（變數設定）：
```
寫設定: sensor_client_get_column(elem=0, serverAddr, appkey, flags=0xA5, property=0x8071(SETUP_SET), len, _BtAppData)
        → 裝置回 column_status(property=0x8071, ack 2 bytes)
讀設定: sensor_client_get_column(elem=0, serverAddr, appkey, flags=0xA5, property=0x8070(SETUP_GET), ...)
        → 裝置回 column_status(property=0x8070, [property_id(2) | _BtAppData])
```

序號流程（一模一樣，只換 property 與 payload）：
```
寫序號: sensor_client_get_column(elem=0, serverAddr, appkey, flags=0xA5, property=0x8073(SET_SERIAL), 8, serial[8])
        → 裝置回 column_status(property=0x8073, ack 2 bytes)
讀序號: sensor_client_get_column(elem=0, serverAddr, appkey, flags=0xA5, property=0x8072(GET_SERIAL), ...)
        → 裝置回 column_status(property=0x8072, [property_id(2) | serial[8]])
```

---

## 6. 韌體端實作位置（供交叉確認）

| 項目 | 檔案 |
|------|------|
| Property ID 定義 | `protocol/bluetooth/bt_mesh/inc/mesh_device_properties.h`（`NODE_GET_SERIAL=0x8072` / `NODE_SET_SERIAL=0x8073`）|
| 收命令分派 | `user/Mesh_Server.c` → `EvtGetRequestProc()`（依 property_id 路由）|
| 寫序號處理 | `user/Mesh_Server.c` → `ServerReceiveSerial()` |
| 讀序號處理 | `user/Mesh_Server.c` → `ServerResponseSerial()` |
| 儲存層 | `user/node_data.c` → `ReadSerialNumber()` / `WriteSerialNumber()` / `IsSerialNumberProgrammed()` |
| 預設值 / 槽位 | `user/node_data.h` → `SERIAL_NUM_DEFAULT "JNC00000"`、`SERIAL_NUM_LEN 8`、`SERIAL_NUM_UD_ADDR 0x0FE00400` |

---

## 7. 韌體端已驗證項目（上機實測）

- ✅ 寫入序號 → 讀回一致（J-Link 直接讀 flash `0x0FE00400` 確認）。
- ✅ 未燒錄（空白槽位）讀取 → 回報預設 `JNC00000`。
- ✅ 寫入後重開機 / 重燒韌體 → 序號仍保留（存在 USERDATA，不受 PS 清除 / 韌體更新影響）。
- ✅ 寫序號不會破壞同頁的 CTUNE / 校正資料（整頁 read-modify-write）。

> 待 App 端實作後，再做一次「App ↔ 裝置」完整往返測試即完成。

---

## 8. 給 App 端的檢查清單

- [ ] 新增 Property ID 常數：`NODE_GET_SERIAL = 0x8072`、`NODE_SET_SERIAL = 0x8073`。
- [ ] 寫序號：送 `0x8073` + 8-byte 序號；解析回應 ack（0 = 成功）。
- [ ] 讀序號：送 `0x8072`；解析回應，取 **data[2..9]** 這 8 bytes 為序號。
- [ ] 序號輸入限制為 8 個字元（建議 ASCII）。
- [ ] UI 預設顯示讀回值（未設定時會是 `JNC00000`）。

---

有任何欄位/格式疑問，可同時聯絡韌體端確認。
