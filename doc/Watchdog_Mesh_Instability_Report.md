# 看門狗 (Watchdog) 導致 v1.41 組網不穩定 — 問題分析與修改報告

| 項目 | 內容 |
|------|------|
| 日期 | 2026-06-25 |
| 影響版本 | **v1.41**（commit `6ec1a6b`）起 |
| 對照基準 | v1.40（commit `585fe3e`）— 組網穩定 |
| 症狀 | v1.40 組網（provisioning / 配網）非常穩定；**v1.41 組網變得非常不穩定** |
| 結論（v1.43 實測更新）| v1.41 新增的 **8 秒看門狗**太短，遇到 **RS485 感測器偵測 `CheckRs485Connect`（無 sensor 回應時阻塞 ~18 秒）** 會逾時 → 裝置每 ~30 秒重啟 → 組網/連線不穩。**已 A/B 實證並修正**。 |
| 狀態 | ✅ 已修正並驗證（v1.43）。⚠️ 下方 §3.1「未配對才不餵狗」為**最初假設、已被實測取代**，正確根因見下方「§0 實測結論」。 |

---

## 0. 實測結論（2026-06-29 更新，取代下方初版假設）

> ⚠️ 本報告 §3.1 最初推測根因是「`watchdog_feed()` 在未配對 `continue` 之後 → 組網期間不餵狗」。
> 後續實機 A/B 測試證實**真正根因是 RS485 感測器偵測的長阻塞**，以下為實測結論。

**真正根因（已實證）：**
- 裝置（Auto Scan 模式、未接 sensor）每次喚醒會呼叫 `CheckRs485Connect()`（`user/bus_drv/bus_rs485.c`）偵測 RS485 感測器；沒回應時迴圈跑滿、**阻塞主迴圈 8~18 秒**，期間不餵狗也不服務 BLE。
- v1.41 的 8 秒看門狗因此逾時 → **每 ~30 秒重啟一次** → 組網/連線不穩。

**A/B 實證：**
- 停用看門狗 → 50 秒 0 重啟（之前每 30 秒一次）→ 證明看門狗就是重啟機制。
- DPRINT 追蹤：`BTM Sensor type` → 隔 8~18 秒 → `RS-485 Disconnect RxCounter:0` → 確認阻塞在 RS485 偵測。

**修正（v1.43，已驗證）：**
1. 看門狗超時 8s → 16s（`app.c`，`WDOG_PERIOD_16S`）。
2. `CheckRs485Connect` 偵測迴圈內每輪 `watchdog_feed()`（`bus_rs485.c`）。
3. 主迴圈 `watchdog_feed()` 移到 `CheckRs485Connect()` 的 `continue` 之前（`app.c`，未配對也餵）。
- 驗證：**100 秒 soak、0 重啟**。

**另一個獨立問題（v1.43 未修，已知）：**
- 同一個 RS485 偵測阻塞，**即使看門狗不重啟，阻塞期間仍會餓死 BLE** → 手機連線 **監督逾時（status=8 / GATT_CONN_TIMEOUT）** → 進「功能設定」時偶發「已中斷與網路的連線」。
- 已用 logcat + COM6 雙端實證（斷線時刻精準落在 RS485 阻塞窗口、約每 60 秒一次）。
- 與看門狗無關，程式碼 v1.40 即存在。規避：節點設「DI 模式 / 指定型號」（非 Auto Scan）；根治需讓掃描不阻塞 BLE（連線中暫停掃描或拆成非阻塞），列為後續 **option B**。

---

## 1. 問題摘要（初版，部分推論已被上方 §0 實測取代）

- v1.40：組網穩定。
- v1.41：組網非常不穩定（節點難以完成配網、容易掉、需反覆嘗試）。
- v1.41 相對 v1.40 的「功能性」新增只有兩項：**看門狗** 與 **Modbus 0x900 讀 BootingSeconds**；另有大量「整體重新排版」。
- 經逐項比對與雙重稽核，**排版未改動韌體邏輯**（見 §6），Modbus 0x900 與組網無關，**唯一能解釋「組網」階段選擇性失效的就是看門狗**。

---

## 2. v1.41 相對 v1.40 的完整改動清單（已用 `git diff -w` 過濾排版確認）

| 改動 | 檔案 | 與組網的關係 |
|------|------|------|
| 🔴 **新增 8 秒看門狗** | `user/watchdog.c` / `user/watchdog.h`、`app.c`、`main.c` | **主因**（§3） |
| 🟠 `DPRINT 0 → 1`（除錯印字全開） | `user/global.h` | 次要加重因子（§3.3） |
| 🟢 Modbus 0x900/0x901 讀 BootingSeconds | `user/ClientModbus.c` | 無關 |
| 🟢 `BootingSeconds` 型別 `int32 → uint32` | `Mesh_Client.c` / `Mesh_Server.c` | 無關（僅用於自動重啟計時） |
| 🟢 刪除 "Peter Test" 等除錯印字 | `bus_i2c.c`、`sensor_server.c` | 無關 |
| 🟢 Mesh_Client / Mesh_Server「整體排版」 | 2 檔 | **經雙重稽核：無邏輯改動**（§6） |
| 🟢 `FW_VER 140 → 141` | `user/global.h` | 版號 |

---

## 3. 根本原因分析

### 3.1 主因：`watchdog_feed()` 在「未配對就 continue」之後，組網期間永遠餵不到

看門狗在開機初始化階段就被啟動（[app.c:159-167](../app.c#L159-L167)，8 秒逾時）：

```c
// app.c  BleMeshNodeInit()
watchdog_config_t wdog_config = {
    .timeout_period = WDOG_PERIOD_8S,     // 8.192 秒
    .warning_percent = WDOG_WARNING_NEVER,
    .enable_warning = false,
    .run_in_debug = false,
    .enable_lockup = true
};
watchdog_init_config(&wdog_config);
```

主迴圈（[app.c:216-253](../app.c#L216-L253)）的關鍵順序如下：

```c
while(1)
{
    if (gecko_event_pending()) { RETARGET_SerialFlush(); }
    pEvent = gecko_wait_event();          // (228) 等待堆疊事件
    bool pass = mesh_bgapi_listener(pEvent);
    if (pass) { ... 事件分派 ... }

    if(!CheckRunDevTask()) continue;      // (235) ★ 未配對 → 直接 continue ★

    /* Feed watchdog to prevent timeout reboot */
    watchdog_feed();                      // (243) ★ 餵狗在 continue 之後，組網期間到不了 ★

    UsartClientProc();
    if(NodeRole == NR_CLIENT)      ClientNodeTask();
    else if(NodeRole==NR_SERVER||NodeRole==NR_SETUP_SERVER) ServerNodeTask();
    else BtMeshSetupTask();
}
```

而 `CheckRunDevTask()`（[app.c:255-262](../app.c#L255-L262)）：

```c
bool CheckRunDevTask()
{
    bool ret_code = TRUE;
    if(!GetMeshNodeStatus(STATUS_PROVISIONED) && (NodeRole != NR_SETUP))
        ret_code = FALSE;        // 尚未配對 且 非 Setup 角色 → 回 FALSE
    return ret_code;
}
```

**問題鏈：**

1. 看門狗在開機 `BleMeshNodeInit()` 就被武裝（armed），8 秒逾時。
2. 組網（provisioning）整個過程中，節點狀態是 **未配對**（`STATUS_PROVISIONED` 尚未成立），而 Server / Client 的角色不是 `NR_SETUP`。
3. 於是 `CheckRunDevTask()` 回 `FALSE` → 主迴圈每圈都在 (235) `continue`，**`watchdog_feed()` (243) 永遠執行不到**。
4. 配網流程（未配對 beacon、PB-ADV/PB-GATT 多次來回、ECDH 金鑰交換、配完後的 model bind / pub-sub / key 設定）通常**需要遠超過 8 秒**的持續活動。
5. → 看門狗逾時 → **配網途中重啟** → 配網失敗 → 重來 → 再次重啟 …… → **「組網非常不穩定」**。

> 一句話：**v1.41 的節點在「還沒配對」這段時間根本不餵狗，所以配網一拖過 8 秒就被自己重啟。**

### 3.2 為什麼問題「只」在組網階段爆發？（看門狗在深睡時是凍結的）

`watchdog_init_config()`（[watchdog.c:121-192](../user/watchdog.c#L121-L192)）組 CTRL 暫存器時，**沒有設定 `EM2RUN` / `EM3RUN` 位元**：

```c
uint32_t ctrl = 0;
ctrl |= (PERSEL << _WDOG_CTRL_PERSEL_SHIFT);  // 逾時
ctrl |= WDOG_CTRL_EN;                          // 啟用
// 沒有 EM2RUN / EM3RUN → EM2/EM3 深睡時看門狗「凍結不計數」
WDOG0->CTRL = ctrl;
```

在 EFR32 Series-1（BGM13）上，`EM2RUN`/`EM3RUN` 預設為 0，代表**深睡（EM2/EM3）時看門狗停止計數**。

這條性質的含意：
- **已配對、會深睡的 Server**：睡很久（SleepingTimer 可達數十秒）也不會被看門狗重啟 —— 因為深睡時看門狗凍結。→ 所以問題**不在睡眠期**。
- **組網（未配對）期間**：節點正被 provisioner 主動掃描/連線，幾乎全程在 EM0/EM1 活躍狀態（看門狗持續計數），加上 §3.1 完全不餵狗 → 8 秒到時必重啟。

兩者合起來，正好解釋了**為何故障精準集中在「組網」這個持續活躍、又最花時間的階段**，而非平時運作或睡眠。

### 3.3 次要加重因子：`DPRINT 0 → 1`

[global.h](../user/global.h) 在 v1.41 把 `DPRINT 0` 改為 `DPRINT 1`，使全韌體的 `dprint()` 全部啟用。`dprint` 走 USART0 @115200 **阻塞式**輸出。組網/事件處理路徑中的大量序列輸出會：
- 拉長每圈主迴圈時間、拖慢事件處理；
- 在 §3.1 不餵狗的前提下，**讓 8 秒更容易被耗盡**；
- 出貨版本一般也不該開啟全量除錯輸出。

> DPRINT 本身通常不是「主因」，但會**加重**主因、讓重啟更快發生。出貨建議改回 0。

### 3.4 附帶風險（即使已配對）

主修正完成後仍需注意：主迴圈任何**單圈**的 role task 若**阻塞 > 8 秒**（例如 RS485/感測器逾時、長時間 flash 操作），仍會觸發看門狗重啟。這屬於「長阻塞操作」風險（CLAUDE.md 已記載），與組網不同源，但建議一併以 §5 的防禦性調整緩解。

---

## 4. 為什麼這個結論精準吻合症狀

| 觀察 | 看門狗假說的解釋 |
|------|------------------|
| v1.40 穩、v1.41 不穩 | v1.40 沒有看門狗；v1.41 才新增 |
| 故障**選擇性**發生在「組網」階段 | 只有「未配對」期間 `watchdog_feed()` 到不了（§3.1）；且該期持續活躍、看門狗有在計數（§3.2） |
| 配對完成後運作正常 | 配對後 `CheckRunDevTask()` 回 TRUE，每圈正常餵狗 |
| 已配對 Server 深睡不會亂重啟 | 深睡時看門狗凍結（未設 EM2RUN，§3.2） |

---

## 5. 需要修改的內容（依優先序）

### ✅ 修改 1（P0，必做）— 把 `watchdog_feed()` 移到 `CheckRunDevTask()` 的 `continue` 之前

**檔案：** `app.c`，主迴圈（約 230~245 行）

**修改前：**
```c
        if (pass) {
            if(BleMeshEventProc(pEvent,BleEventFun) == FALSE)
                if(BleMeshEventProc(pEvent,MeshEventFun) == FALSE)
                    EventIDtoStringProc(pEvent);
        }
        if(!CheckRunDevTask()) continue;          // (235)

        //dprint("Node Role:%d\r\n",NodeRole);

        /* Feed watchdog to prevent timeout reboot */
        //if(BootingSeconds<60)
            watchdog_feed();                       // (243) ← 組網期間到不了

        UsartClientProc();
```

**修改後：**
```c
        if (pass) {
            if(BleMeshEventProc(pEvent,BleEventFun) == FALSE)
                if(BleMeshEventProc(pEvent,MeshEventFun) == FALSE)
                    EventIDtoStringProc(pEvent);
        }

        /* Feed watchdog EVERY loop iteration — 必須在 CheckRunDevTask() 的
         * early-continue 之前。未配對(組網中)的節點會在下一行 continue，若餵狗
         * 留在 continue 之後，組網期間就完全不餵狗，8 秒後被看門狗重啟、打斷配網。*/
        watchdog_feed();

        if(!CheckRunDevTask()) continue;

        UsartClientProc();
```

**重點：** 餵狗必須**每一圈都做**（含未配對期間），與「要不要跑 device task」解耦。

### ☑ 修改 2（P1，建議，出貨版）— `DPRINT` 改回 0

**檔案：** `user/global.h`
```c
//#define DPRINT 1
#define DPRINT 0     // 出貨版關閉全量除錯輸出，避免阻塞式序列拖慢主迴圈
```
> 若仍需現場除錯可暫時保留 1，但組網壓力測試建議在 `DPRINT 0` 下複測。

### ☑ 修改 3（P2，可選防禦性）— 放寬逾時，容忍偶發長阻塞

**檔案：** `app.c`，`wdog_config.timeout_period`
```c
.timeout_period = WDOG_PERIOD_16S,   // 8s → 16s（或 WDOG_PERIOD_32S）
```
可用值（`user/watchdog.h`）：`WDOG_PERIOD_8S`(≈8.2s) / `WDOG_PERIOD_16S`(≈16.4s) / `WDOG_PERIOD_32S`(≈32.8s)。
> 這是「保險」不是「根治」—— 真正的修正是 修改 1。即使放寬逾時，仍應先做修改 1。

### 補充考量（非必改，供討論）
- **是否在組網完成前才武裝看門狗**：另一種設計是配對完成後才啟用看門狗，避免組網期受其影響。但會犧牲「開機即保護」。**建議採用修改 1（每圈餵狗）即可**，不需改動武裝時機，較單純。
- 修改 1 後請順道確認 `ServerNodeTask()` / `ClientNodeTask()` / `UsartClientProc()` 內沒有單圈 > 8 秒的阻塞；若有，需拆分或於該處補餵狗。

---

## 6. 已排除的可疑點（避免重複調查）

v1.41 對 `user/Mesh_Server.c`（+2893 行 diff）與 `user/Mesh_Client.c`（+2069 行 diff）做了「整體重新排版」，commit 訊息自述「**(應該沒改韌體)**」。本次以兩種方法雙重稽核：

1. `git diff -w`（忽略空白）後再逐 hunk 判讀；
2. **token 正規化比對**（去註解、去全部空白、去括號後比對 token 流），讓「單行拆多行」這類排版徹底消失。

**結論：兩檔皆為純排版，無任何影響韌體行為的改動。** 唯一的非排版差異是：
- `BootingSeconds` 型別 `int32 → uint32`（僅用於自動重啟計時，與組網無關，且實際比較結果不變）；
- 幾個**未被呼叫**的前向宣告（`lastSeconds`、`DebugServerNodeStage` / `DebugClinetNodeStage`）；
- 幾行被註解掉的 / 新增的 `dprint` 除錯印字。

> 已驗證所有 mesh/provisioning 關鍵區段（client 輪詢狀態機、IV-index 更新、retry/timeout、sleep/proxy/beacon/LPN 切換、`Cmd_ms_*` 呼叫與引數、timer 常數）在兩版間**完全一致**。**組網回歸不在排版，在看門狗。**

---

## 7. 驗證計畫（修改後）

1. **組網重複測試**：對全新/已 reset 的節點，連續執行 N 次配網（建議 ≥ 20 次），統計成功率；與 v1.40 比較應回到「非常穩定」。
2. **序列埠觀察**：配網過程中監看 COM6（USART0 PA0 @115200），確認**不再出現配網中途的重開機橫幅**（`BTM001: Firmware Version ...` 不應在配網途中反覆出現）。
3. **餵狗涵蓋度**：（可選）在未配對狀態下放置 > 8 秒，確認裝置不重啟，證明組網期已正常餵狗。
4. **已配對長時間運作**：配對後長時間運行（含 Server 深睡循環），確認看門狗仍能在真正當機時保護、但不誤觸發。
5. **回歸**：確認原本看門狗要防的「當機自動重啟」功能仍有效（例如人為製造 > 逾時的阻塞，應重啟）。

---

## 8. 風險與回滾

- **修改 1** 風險極低：只是把既有的 `watchdog_feed()` 呼叫提前，不改變任何 mesh 行為；最壞情況只是看門狗保護範圍變大（更安全）。
- **修改 2 / 3** 為設定值調整，獨立可回滾。
- 回滾方式：還原 `app.c` / `global.h` 對應行即可；本報告所有引用均含 `檔案:行號`，便於核對。

---

## 9. 摘要（TL;DR）

> v1.41 新增 8 秒看門狗，但 `watchdog_feed()`（app.c:243）被放在「未配對就 `continue`」（app.c:235）之後。節點在**組網（未配對）期間完全不餵狗**，配網一拖過 8 秒就被看門狗重啟、打斷配網 → 組網不穩。
> **主修正：把 `watchdog_feed()` 移到 `CheckRunDevTask()` 的 `continue` 之前，讓每圈都餵狗。** 另建議出貨版 `DPRINT` 改回 0，並可選擇把逾時放寬到 16/32 秒作為保險。Mesh_Server/Client 的「排版」已雙重稽核確認無邏輯改動，非肇因。
