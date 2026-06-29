# 「中繼端(Sensor Client)也要能讀/寫序號」— 韌體端需求交接文件

對象：JNC BT Mesh **韌體**工程師
撰寫：2026-06-29（App 端發起）
相關：[Serial_Number_App_Handoff.md](Serial_Number_App_Handoff.md)（序號讀寫原始規格）、[Function_Display_App_Handoff.md](Function_Display_App_Handoff.md)（角色判定 / v1.44 appkey 自綁）

---

## 0. 一句話需求

> 目前**只有「感測端 / Sensor Server」裝置讀得到序號**；「中繼端 / Sensor Client」裝置讀寫序號全部逾時。
> 希望**所有裝置（含中繼端）都能讀/寫序號**。經 App 端定位，這需要**韌體端讓中繼端 build 也具備能回應 `0x8072/0x8073` 的接收窗口**——App 端無法單方面解決（下詳）。

---

## 1. 名詞對齊（避免雞同鴨講）

| 角色 | 註冊的 Sensor model | 文件用語 | App UI 字串 |
|------|----------------------|----------|-------------|
| 感測端 | **Sensor Server `0x1100`**（+ Setup Server `0x1101`）| 感測端 / Server | 感測端 |
| 中繼端 | **Sensor Client `0x1102`** | 中繼端 / Client | 主機端 |

> 「中繼端 = 主機端 = Sensor Client = `0x1102`」是同一件事。本文一律稱「中繼端」。

---

## 2. 現象（App 端 logcat 實測，2026-06-29）

同一支手機、同一個 mesh 網路，連續讀多台裝置：

| 裝置 unicast | 註冊的 Sensor model | `0x8072` 讀序號結果 |
|---|---|---|
| **2** | **只有 `0x1102` Sensor Client** | ❌ **全部 `Mesh error: Time out`**（41 次嘗試皆逾時）|
| 3 | `0x1100` Sensor Server | ✅ 回 `72 80 4A 4E 43 30 30 30 30 33` → **`JNC00003`** |
| 4 | `0x1100` Sensor Server | ✅ 回 `72 80 4A 4E 43 30 30 30 30 34` → **`JNC00004`** |
| 5–10 | `0x1100` Sensor Server | ✅ 正常 |

整輪實測：`NODE_GET_SERIAL ok` 共 **67 次**（全部來自感測端）；對中繼端（位址 2）的序號讀取 **0 次成功**。
同台中繼端的 `0x8038`（系統資訊）、`0x8030`（即時值）也**一律逾時** → 證明該裝置對「所有 Sensor Server 類讀取」皆無回應，並非序號專屬問題。

> 補充：中繼端不是「沒有序號」。每台實體裝置 flash 都有燒序號（`SERIAL_NUM_UD_ADDR 0x0FE00400`，預設 `JNC00000`，見 [Serial_Number_App_Handoff.md](Serial_Number_App_Handoff.md) §6）。問題是**它沒有能在 mesh 上回應序號讀取的 model**。

---

## 3. 根因定位

序號讀寫走 `sensor_client_get_column`（Sensor **Column Get**，opcode `0x8233`），裝置端的處理在 **Sensor Server 那條路**：

| 項目 | 檔案 / 函式（來自 [Serial_Number_App_Handoff.md](Serial_Number_App_Handoff.md) §6）|
|------|------|
| 收命令分派（依 property_id 路由）| `user/Mesh_Server.c` → `EvtGetRequestProc()` |
| 讀序號處理 | `user/Mesh_Server.c` → `ServerResponseSerial()` |
| 寫序號處理 | `user/Mesh_Server.c` → `ServerReceiveSerial()` |
| 儲存層 | `user/node_data.c` → `ReadSerialNumber()` / `WriteSerialNumber()` |

**Sensor Column Get 只會送達「有註冊 Sensor Server 類 model」的裝置。** 中繼端 build 只註冊了 Sensor Client（`0x1102`），element 上**沒有任何 model 會收到這個 column-get** → 協議棧直接丟棄 → App 端逾時。

→ 結論：**這是裝置端「沒有接收窗口」的問題，App 端無法繞過**（你不可能讀一個裝置沒有 model 去回應的值）。

---

## 4. 需求（請韌體端擇一實作）

### 路線 1（建議，改動小）— 讓中繼端 build 也開「序號接收窗口」

在**中繼端的韌體 build** 上：

1. 在 **element 0** 註冊能接收 Sensor Column Get（`0x8233`）的 model，使 `EvtGetRequestProc()` 能收到 `0x8072 / 0x8073`。
   （實作上即「讓中繼端也具備 Sensor Server 那條收命令路徑」；`ServerResponseSerial` / `ServerReceiveSerial` 已現成，等於只是把窗口開在中繼端上。）
2. **在 appkey 加入時自動綁定**該 model（沿用感測端 v1.44 的「appkey 自綁」做法，見 [Function_Display_App_Handoff.md](Function_Display_App_Handoff.md) §2），否則協議棧仍會以「appkey 未綁該 model」為由丟棄。
3. 序號讀寫格式**與感測端完全相同**（[Serial_Number_App_Handoff.md](Serial_Number_App_Handoff.md) §3–§5，property `0x8072/0x8073`，8-byte ASCII）。

> 改完後 **App 端零改動**，現有讀寫流程即可對中繼端生效。

⚠️ **請與 App 端確認的副作用**：若直接在中繼端註冊 **Sensor Server `0x1100`**，App 的角色判定會把它**歸類成「感測端」**（App 是看 DCD 有無 `0x1100` 判角色）。若不希望中繼端在 UI 變成感測端，請走路線 2，或與 App 端約定用別的方式判角色。

### 路線 2（乾淨但改動大）— 序號改掛「所有裝置都註冊的專用 vendor model」

把序號讀寫從 Sensor model 移到一個**感測端/中繼端 build 都會註冊**的 vendor model，讓序號與角色徹底解耦。
- 優點：以後任何角色都讀得到，且不污染 App 的角色判定。
- 代價：韌體 + App 兩端都要改傳輸層，工較大。

---

## 5. 驗收條件（韌體改完後一起測）

對**中繼端**裝置，用 App「功能設定」操作：

- [ ] **讀序號**：進頁自動讀回序號（未燒錄回預設 `JNC00000`，已燒錄回實際值），不再逾時。
- [ ] **寫序號**：送 8-byte（例 `JNC12345`）→ 裝置回 ack `0x0000`（成功）；錯誤碼比照 [Serial_Number_App_Handoff.md](Serial_Number_App_Handoff.md) §3（`0x0001` flash 失敗 / `0x0002` 長度不足）。
- [ ] **持久化**：寫入後重讀一致；跨 proxy 重連 / 重開機後仍保留。
- [ ] **感測端不回歸**：原本感測端的序號讀寫維持正常（不被本次改動影響）。

---

## 6. App 端現況（供交叉確認，韌體端不需處理）

- 序號讀寫程式碼**不分角色**，CONFIG 分頁對**任何**裝置都會自動觸發讀取——是裝置不回，不是 App 擋掉。
  - 真正送命令：[SensorPolling.kt](../src/main/java/com/siliconlabs/bluetoothmesh/App/Fragments/Device/Config/SensorPolling.kt) `readDeviceSerial()` / `writeDeviceSerial()`（property `0x8072 / 0x8073`，走 `ControlElement(element 0, 第一個 boundAppKey).getSensorStatus(SensorColumnStatus)`）。
  - UI：[DeviceConfigFragment.kt](../src/main/java/com/siliconlabs/bluetoothmesh/App/Fragments/Device/Config/DeviceConfigFragment.kt)。
- 因此**路線 1 改完，App 端不需動**。
- App 端可能配合做的小事（與本需求獨立）：中繼端逾時時顯示「此型別不支援序號」之類明確訊息，避免一直跳「讀取失敗」。等韌體開窗口後自然就會讀到。

---

## 7. 參考

- 序號讀寫原始規格、格式、韌體檔案位置：[Serial_Number_App_Handoff.md](Serial_Number_App_Handoff.md)。
- 角色由開機註冊的 model 決定、v1.44 appkey 自綁機制：[Function_Display_App_Handoff.md](Function_Display_App_Handoff.md)（§2、§3-1、§5）。
- Property ID：`NODE_GET_SERIAL = 0x8072`、`NODE_SET_SERIAL = 0x8073`（`protocol/bluetooth/bt_mesh/inc/mesh_device_properties.h`）。
