# 「組網後功能欄自動顯示」App 端交接說明

對象：JNC BT Mesh App 開發者
韌體版本：**v1.44 起**（裝置端已修，App 端待處理）
撰寫：2026-06-29

---

## 1. 問題現象

使用者佈建（組網）一台**全新**裝置後，進入 App 的「功能設定」頁：

- **功能欄是空白的**（沒有顯示這台是「感測端 / Server」或目前的模式）。
- 必須**手動點一次「感測端」設定**，功能欄才會出現內容。

使用者期望：組網完成後，App 能**自動**顯示裝置目前的功能/角色，不需手動設定。

---

## 2. 根因定位（雙邊 log 實測）

| 層面 | 狀態 |
|------|------|
| **裝置端** | ✅ **v1.44 已修**。先前裝置完全收不到 App 走 app key 的 sensor 讀取（appkey 未綁到 Sensor Server model → 協議棧丟棄），故所有功能/感測讀取都無回應。v1.44 起裝置在 appkey 加入時自綁，**現在會正常回應所有 sensor 讀取**（已驗證：`NODE_GET_BTM_INFO`、`NODE_SENSOR_SETUP_GET`、序號、感測值皆有回）。 |
| **App 端** | ⚠️ **待處理**。即使裝置已能回應，功能欄仍空白、仍需手動設定 → 表示 **App 並未用「讀取裝置」的結果自動帶入功能欄**，而是只顯示「使用者透過 App 指派過」的值。 |

> 結論：**「自動顯示」需要 App 端改成「連線後主動讀裝置、用回傳值帶入功能欄」**。韌體端無法代為自動帶入（裝置回應中沒有單一「角色」欄位，需由 App 組合判斷，見下）。

---

## 3. 裝置可提供的資料（App 可據此自動帶入）

裝置已能回應以下讀取（皆走 **Mesh Sensor model、app key**，與「變數設定」相同傳輸方式；element 0 / SENSOR_ELEMENT）：

### 3-1. 角色（感測端 Server / 中繼端 Client）→ 由 **DCD（Composition Data）** 判斷

韌體依角色註冊不同 model（開機時決定）：

| 角色 | 註冊的 model（會出現在 DCD） |
|------|------------------------------|
| **感測端 / Server** | Sensor **Server** `0x1100` + Sensor **Setup Server** `0x1101` |
| **中繼端 / Client** | Sensor **Client** `0x1102` |

App 在組網時本來就會讀 DCD（`getDeviceCompositionData`）。**直接用 DCD 的 model 清單判斷角色即可**：有 `0x1100` → 感測端；有 `0x1102` → 中繼端。不需要再向裝置另外問角色。

### 3-2. 模式 / Sensor Class（Auto Scan、DI 模式…）→ 讀 **`NODE_GET_BTM_INFO` 或 `NODE_SENSOR_SETUP_GET`**

| 用途 | Property ID | 回傳內容（重點欄位） |
|------|-------------|----------------------|
| 讀節點資訊 | `NODE_GET_BTM_INFO` = **0x8038** | 機型名、韌體版本、Working Time、**BtmClass(模式)**、電量、狀態… |
| 讀設定（功能設定頁用） | `NODE_SENSOR_SETUP_GET` = **0x8070** | 見下方 `_BtAppData` |

`NODE_SENSOR_SETUP_GET (0x8070)` 回傳結構（前 2 bytes 為 property id，之後為 `_BtAppData`）：

```c
typedef struct _BtAppData_ {
    int16   TempGain, TempOffset;     // 溫度 Gain/Offset (×100)
    int16   RhGain, RhOffset;         // 濕度 Gain/Offset (×100)
    uint16  WorkingTimer;             // 工作週期 (秒)
    uint16  BtmClass;                 // ★模式/Sensor Class (見對照表)
    uint8   BaudrateIndex;            // RS485 baud 索引
    uint8   Rs485ServerDelayBeforeSleep;
    uint16  Rs485ClientBuffTimeoutMs;
    uint16  ProtocolGen;
    uint16  RebootForRs485IdelSecnods;
    int16   RebootMinutes;
} _BtAppData;
```

**`BtmClass` 對照表**（韌體 `SensorClassStr[]`，即 App 該顯示的字串）：

| BtmClass | 顯示 |
|----------|------|
| 0 | NO Sensor |
| 1 | Auto Scan（自動掃描，預設） |
| 2 | PZEM |
| 3 | Visual Sensor |
| 4 | DC600 |
| 5 | FTM94 |
| 6 | BTM-G6 |
| 7 | BTM485 |
| 8 | DI Mode（DI 模式） |

> 新組網、未設定的裝置 `BtmClass` 預設為 **1 (Auto Scan)**。

---

## 4. 建議 App 做法

1. **進入「功能設定」頁時，主動向裝置讀取**，不要只顯示 App 本地記憶值：
   - 角色：用已取得的 **DCD** model 清單判斷（`0x1100`→感測端 / `0x1102`→中繼端）。
   - 模式：讀 **`NODE_SENSOR_SETUP_GET (0x8070)`** 取 `BtmClass`，依上表帶入。
2. 讀到後**自動帶入功能欄**（不需使用者手動點選即顯示）。
3. 讀取失敗（逾時）時可重試幾次再顯示「讀取中／未知」，而非一直空白。

---

## 5. 時序注意事項（重要）

- **組網完成後，裝置會自動重開機一次以套用角色設定**（這是裝置端既有且必要的行為：角色由 App 綁定的 model 決定，需重開生效）。重開約在「App 設定完成或 proxy 斷線後數秒～20 秒」內發生。
- 因此**剛組網的瞬間讀取，裝置可能正在重開、或尚未跑完第一輪感測，回傳會不完整**（例如感測值 `NODE_GET_ALL_SENSOR` 在第一輪前為空）。
- 建議：**組網流程結束、裝置重開並穩定（約 10～30 秒）後**，App 再連線讀取功能/模式帶入；或在功能設定頁提供「重新整理」讓使用者觸發重讀。
- 角色（DCD）與模式（`0x8070`）在裝置穩定後讀取都會正常回應（v1.44 已驗證）。

---

## 6. 參考

- 韌體變更：`CHANGELOG.md` v1.44。
- 屬性 ID 定義：`protocol/bluetooth/bt_mesh/inc/mesh_device_properties.h`（`NODE_GET_BTM_INFO=0x8038`、`NODE_SENSOR_SETUP_GET=0x8070` 等）。
- 結構：`user/Mesh_Server.h`（`_BtAppData`）。
- 模式字串：`app.c` `SensorClassStr[]`。
