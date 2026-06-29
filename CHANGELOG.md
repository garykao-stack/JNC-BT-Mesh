### ALL DEVICE v1.43  #
#### 20260629 (高) #
1. 修正 v1.41 看門狗導致的「重啟迴圈」問題（組網/連線不穩的主因）。
    - 原因：v1.41 新增的 8 秒看門狗，遇到 RS485 感測器偵測（`CheckRs485Connect`，無 sensor 回應時可阻塞達 ~18 秒）的長阻塞會逾時重啟 → 裝置每 ~30 秒重啟一次。
    - 修正：
        a. 看門狗超時 8s → 16s。
        b. 在 `CheckRs485Connect` 偵測迴圈內每輪餵狗，使合法的慢偵測不會餓死看門狗。
        c. 主迴圈在「未配對(組網中)」期間也餵狗（移到 `CheckRs485Connect` 的 continue 之前）。
    - 實測：100 秒 soak 0 重啟（修正前每 ~30 秒重啟一次）。詳見 `doc/Watchdog_Mesh_Instability_Report.md`。
2. 【已知未修問題】Auto Scan 模式且未接 sensor 時，每 ~60 秒喚醒的感測器偵測會卡住 BLE → 進「功能設定」時偶發「與網路斷線」(BLE 監督逾時 status=8)。與看門狗無關，程式碼 v1.40 即存在。規避：該節點設為「DI 模式 / 指定型號」(非 Auto Scan)。

### ALL DEVICE v1.42  #
#### 20260625 (高) #
1. 新增產品序號 (Product Serial Number) 功能
    - 序號長度 8 bytes (預設 "JNC00000")，另預留 8 bytes (共 16 bytes)。
    - 儲存於晶片內部 USERDATA flash page (位址 0x0FE00400)，恢復原廠 (flash_ps_erase_all) / 重燒韌體皆不會被清除；以整頁 read-modify-write 寫入，保留同頁的 CTUNE / 校正資料。
    - App 可透過藍牙 (Mesh Sensor model，與「變數設定」相同傳輸方式) 讀寫:
        - 讀序號: Property ID NODE_GET_SERIAL (0x8072)
        - 寫序號: Property ID NODE_SET_SERIAL (0x8073)
    - 未燒錄序號時讀取回報預設值 "JNC00000"。
    - App 端交接文件: doc/Serial_Number_App_Handoff.md。
2. 新增開發輔助文件與工具: CLAUDE.md / README 補上 IAR 編譯快捷鍵與序列埠 (USART0 PA0/PA1 @115200) Debug 說明、scripts/serial_monitor.ps1 序列埠監看工具。

### ALL DEVICE v1.41  #
#### 20260306 (明) #
1. 增加看門狗 watchdog.c / watchdog.h，目前設定超時八秒重啟
2. modbus在0x900增加讀取 BootingSeconds
3. Mesh_Client / Mesh_Server 整體重新排版(應該沒改韌體)，其他部分涵式也重新排版

### ALL DEVICE v1.40  #
#### 20241105 (珊) #
1. 新增讀取 DI 狀態。目前僅電路板 JNC-BT-MESH V1.2 有 DI 功能，其它電路板一樣可燒此版本。
2. 可以通過 Client 或 Server 的站號用 Modbus FC2 讀取 DI 狀態。
    - 對 Client 讀取之設定值
        BT ID-1~255 DI 狀態: 100002~100256
    - 對 Server 讀取之設定值
        DI 狀態: 100001
3. 使用 JNC BT Mesh APP 1.0.7.11 以上版本，Server 模式選擇"DI模式"。避免掃描超時無法回應 Client。
    APP 設定 Server 為 DI 模式:
      - 點"參數設定"
      - 模式選"DI模式"
      - 按"設定"
4. Server 模式選"自動掃描(Skynet)" 且有接 sensor 的 Skynet 系列(T/R,CO2,PM2.5,TVOC) 可回傳 DI 狀態，先預留此功能。
5. Server 模式選"自動掃描(Skynet)"，若不接 sensor 不會回傳 DI 狀態，避免忘記設定造成回應率低的問題。
6. DI 模式狀態指示燈: Server 不接 sensor 開機時不會紅藍燈交替閃爍。
7. 純中繼的節點(不接任何 sensor)選擇"DI模式"再開"中繼功能"，可避免掃描超時無法回應 Client。

### ALL DEVICE v1.39  #
#### 20240807 (珊) #
1. BT-Mesh 比流計歸零，不需要每次開機都清除，故註解掉。

### ALL DEVICE v1.38  #
#### 20231123 (曾) #
1. 當IvIndex更新失敗時(錯誤碼:0x181)，強制以手動方式指定IvIndex更新

#### 20231122 (曾) #
1. Client的定時重啟改為短暫進入休眠200ms(在廣播指令前觸發) ※定時重啟(分)最大值為32767，約為22天
2. Server同上，但觸發時間設在回應Client BT-Mesh指令之後
3. 減少部份編譯提示訊息

### ALL DEVICE v1.37  #
#### 20231114 (珊) #
1. CO2 sensor 改使用 S8，較穩定。
2. 修正電池模式從休眠醒來後 CO2, PM2.5, TVOC 讀值異常:
    - 修正 GetDeviceInfoDelay: 提早醒來等 sensor 穩定。
    - 增加 PreReadDelay: 時間內每秒讀取一筆，可供後續運算使用。
    - 增加 syncTime: 工作週期超過 Client 時，跳過等待時間趕快讀，並減少下一次睡眠時間。
3. 增加可從 app 設定 CO2, PM25, TVOC 的 gain offset:
    - 溫度 gain offset 借用給 CO2, PM25, TVOC。
    - 濕度 gain offset 借用給 PM10。

### ALL DEVICE v1.36  #
#### 20230912 (珊) #
1. skynet 增加可以讀取 PM2.5 Sensor PM2.5,PM10 (PMSA003.c 和 PMSA003.h)。
2. skynet 增加可以讀取 TVOC Sensor (SGPxx.c 和 SGPxx.h)。
3. skynet 修正 BATTERY/TEMP/RH/CO2/PM25/TVOC modbus address 及資料類型等改成統一 modbus 表，原本的暫時保留。
### A308 V1.37 #
#### 20230830 (曾) #
1. Server在A308模式中，即便接著市電也會進入休眠
2. 發送RS485 TX訊息前不論Uart是否在忙錄中都將狀態重置以發送訊息(原流程為直接跳過不發送)

### A308 V1.36 #
#### 20230426 (曾) #
1. 修正A308通訊狀態不會正確回傳「3:Server不存在」的錯誤
2. Client通訊表增加Server類型(0x500~0x5ff)

#### 20230425 (曾) #
1. Server若在A308通訊完成之前接收到Client的廣播指令時，會直接結束讀取流程，回應讀取失敗訊息後，等待休眠指令後進入休眠
2. 在BTM_A308的Server下，套用Baudrate設定而非固定的9600
3. Client可讀取A308通訊狀態(0x400~0x4ff)(0:成功, 1:BT資料接收中, 2:A308讀取失敗, 3:Server不存在)
4. 修正Clinet通訊間隔設定10分鐘時會無效的問題
5. Client可接收的Server數由10增加到16個(不含Relay)

### ALL DEVICE V1.35 #
#### 20230413 (曾) #
1. 增加Client重啟時間，會依設定時間(分)重啟

### ALL DEVICE V1.34 #
#### 20230412 (曾) #
1. 修正Client Modbus回傳內部記憶體的電池電量異常的問題

#### 20230411 (曾) #
1. 修正Server休眠時間最大只能設定160秒的問題

### ALL DEVICE V1.33 #
#### 20230306 (陞) #
1. 於global.h檔中增加溫濕度si7021跟SHT3x定義可以選擇，用於切換兩不同溫濕度Sensor，並增加SHT3x.c及SHT3x.h兩個檔案。

### ALL DEVICE V1.32 #
#### 20230215 (成) #
1. 修正以I6 DO指令啟動測試模式時有很大的機會失敗的問題

#### 20230214 (成) #
1. 當通訊測試次數超過300次後，重置計數器

### ALL DEVICE V1.31 #
#### 20230118 (成) #
1. BT通訊測試模式預設時間由1分鐘增加到3分鐘
2. 承上，當Modbus以 DO控制方式下達測試指令時，若常下已處於測試模式則中斷目前的測試，之後進行新的測試流程
3. 修正Client發送測試指令給Server未根據發送結果進行後續動作的問題

#### 20230112 (成) #
1. 擴充Client Modbus通訊，可讀取電池電量&命令次數&回應次數與其他設定參數
2. 增加BT通訊測試模式
3. 在此模式下Server不會進入休眠
4. 承上，為縮短測試時間，測試模式Client下約每4秒廣播一次指令
5. 提供Windows測試程式展示

#### 20221230 (成) #
1. 修正當RS485發送TX失敗後，未把TxCount歸零而造成無法再發送資料的問題
2. 增加RS485閒置重啟功能，可設定在n秒內，若未接收到任何Rx資料時重啟(預設關閉)
3. 修正在Client暫存器中讀取到錯誤位置時，回應的內容格式不正確的問題。

### ALL DEVICE V1.30 #
#### 20221213 (成) #
1. 新增在透傳模式下讀取Server電量的方法(也通用於Skynet唷!)

### ALL DEVICE V1.29 #
#### 20221108 (成) #
1. 將透傳工作時間計入休眠週期中以簡化週期循環
2. 改善休眠時間超過10分鐘後可能會沒有作用並造成睡眠週期異常的問題。
3. 修改Skynet在市電模式下的讀取週期判斷
    - 所有Server都是市電供電且全都是Skynet模式，每2秒讀取一次Skynet內容
    - 所有Server都是市電供電但其中含有逶傳模式時，以所設定的通訊週期(預設為60秒)讀取Skyent內容
4. 改善Client在逶傳模式中當Modbus指令間隔小於透傳暫存觸發時間時，可能會造成回應錯位的問題。
5. 透傳暫存觸發時間預設值由500ms改為300ms

### ALL DEVICE V.28 #
#### 20221104 (成) #
1. 將透傳功能合併到Skynet版本(需設定)
    - Client根據指令的ID是否存在Skynet設備來決定處理模式
    - 當Skynet設備存在，回應原本Skynet訊息
    - Skynet設備不存在，以透傳方式處理
    - 當模式為Client or 模式為透傳時，指定Baudrate功能會生效
2. BTM指令版本更新到3版

#### 20221027 (成) #
1. 增加開啟設備後燈號全亮一秒再繼續開機的動作。

### RS485/BTM(透傳) v1.27 20220928-1 (成)  #
1. 修正RS485設備不存在後會不定時回傳暫存值的問題。

### SYKNET v1.27 20220928 (成)  #
1. 移除Skynet必須要在溫濕度Sensor存在時才能讀取CO2的限制。
2. 增加測試模式定義NODE_DISCONNECT_DETECT_COUNT，當值大於0時，直接指定斷線判斷的未回應次數

## v1.26 #
### 20220921 (成) #
1. 修正透傳模式中，Server與Slave設備斷線時，Client會回傳暫存值而造成Master無法判斷為斷線的問題。
    - Client連續5次Modbus指令未回則判斷為斷線，不會回應Master的指令。

### 20220906 (成) #
1. 移除在Evt_mn_changed_ivupdate_state事件處理中，不必要的手動指定iv index動作。

### 20220905 (成) #
1. 修正IvIndex不會自動更新的問題，觸發更新條件為：
    - 當傳送序號低於0x100
    - Client發送GET_ALL_SENSOR指令時回應錯誤碼為0xc03
    - Server發送GET_ALL_SENSOR指令時回應錯誤碼為0xc03
2. 修正IvIndex每次更新會直接跳2號的問題。 
3. 版本編號更新
    - 標準版SKYNET: v1.26
    - BT_MESH_G6: v1.02
    - ULTRA_SOUND_SKYNET: v1.02

## v1.25 #
### 20220831 : RS485/BTM v1.1 (成) #
1. 將綠燈改為uart通訊燈，當收到RX訊息亮起，重置RX Buff時熄滅。
2. 藍燈新增功能
    - 原為角色指示燈：Client>亮, Server>滅
    - 當Server發送Node訊息錯誤碼為0xc03時(limit_reached)，以0.5秒速度閃礫

### 20220826 : RS485/BTM v1.1 (成) #
1. 透傳模式增加Buff將已讀取過的內容暫存，當Client達成以下條件時回傳：
    - Server超過設定時間未回傳結果(預設500ms)
    - Server進入休眠後(Client時間設定必須與Server一致)
2. Server回傳Modbus內容增加資訊(位置&長度)，當回應超過Timeout時間時，雖不轉傳Modbus回應，但跟據位置&長度將讀值儲存下來
3. Server在回傳Node資訊後，不立即進入休眠模式，保持開啟狀態(時間可設定，預設3分鐘)以執行透傳動作。
4. 為Uart的Tx動作增加Timeout(bytes * 10ms)設定，當超過設定時間傳送動作依然未完成時強制結束傳送流程，避免uart因此停止回應。

### 20220819 : RS485/BTM v1.1 (成) #
1. 在接收到新的Modbus指令後，原來Rx的接收內容將會被丟棄，避免I6讀取發生錯位的問題。

### 20220818 : A308 v1.2 (成) #
1. Client模式下的A308接收Buff由4組改回5組
2. Server模式中，當發送完A308設備資訊後，達成以下任一條件才會進入休眠。(若直接進入休眠Client就無法重新要求遺失資料)
    - 30秒(原為20秒)未接收到其他指令
    - 收到NODE_A308_GET_FINISHED property訊息

### 20220817 : A308 v1.2 (成) #
1. 修正在讀取到設備資訊之前對Client下Modbus指令會造成RS485停止回應的問題。

### 20220816 (成) #
1. 優化透傳速度
    - Server回傳方式由指定Address改為廣播方式發送可加快回應速度與減少發送錯誤
    - 將透傳處理流程獨立出來，不與讀取即時值一起處理，加快反應速度
2. 修正Client啟用A308客製模式時無法正常讀取設備資訊的問題。
3. Server啟用Relay模式後，在回應電池狀態後等待4秒再進休眠模式(原為2秒，但與Client流程為等待4秒)，這4秒含傳送&送試時間

### 20220810 (成) #
1. Sensor功能增加JY-GD15(風速風量變送器)
2. 增加非阻塞流程來讀取JY-GD15的資料
3. 讀取BT-Mesh設備資訊時，直接回傳結果，不參與GET_ALL_SENSOR流程，以加快APP顯示速度
4. 當APP以簡易設定方式來存取設定時，直接進行處理，加快APP處理進度同時不會影響讀取Sensor&休眠流程。

## v1.24 #
### 220809 : RS485/BTM v1.1 (成) #
1. 增加設備訊息Property(PROP_NODE_INFO:0xffff)，內含:
    - Sernsor類型
    - 狀態旗標
    - 電池電量
    - 韌體版本編號
    - 通訊協定版本
    - 設備名稱(上限30字)
2. App設定增加Baudrate設定(for RS485/BTM)
3. 增加非Setup模式下提供APP存取設定的指令(Read/Write Column)
4. 非Setup模式下讀取設定後，可指定暫停工作秒數(0~255秒)，其間不進入休眠，也不回應廣播指令
5. 在RS485/BTM模式下，Property:GET_ALL_SENSOR回傳內容同JNC_BT_MESH
6. GET_ALL_SENSOR指令中的Property統一回傳NODE_GET_ALL_SENSOR_GEN2(0x803a)，提供App識別是否支援新版通訊

### 220801 : A308M v1.01 (成) #
1. 修改A308讀取機制
    - Client:
        - 在接收Server狀態回應後，發出NS_SEND_INFO_ACK訊號讓Server端知道Client已收到回應
        - 修改讀取機制，未即時回應讀取設備狀態指令的Server如在讀取其他裝置的流程中有回應狀態，會將該Server加入此次讀取流程中
    - Server
        - 在發送完設備訊息後，若未接收到Client發出的NS_SEND_INFO_ACK訊號，每2秒會再發送一次設備訊息給Client，避免訊號丟失
        - 從休眠狀態喚醒後，多等待2秒再讀取A308數值(設定義A308_SLEEP_MODE=1)。
2. A308來源表減少讀取內容，以減少傳送時間
3. 增加NODE_INFO Property，用以統一回傳設備狀態(for APP，未測試)
4. 增加可在一般狀態下存取設定參數的指令:NODE_SENSOR_SETUP_GET/NODE_SENSOR_SETUP_SET


### 220722 : A308M v1.00 (成) #
1. 增加A308訊號模擬。在Server模式中，不向A308讀取數值，直接以ID填滿所有暫存器
2. Client擴充讀取緩衝區到5組，A308數量上限由14降為10組
3. 修正A308 ID對應表未正確初始化的問題。
4. 修正A308 Server模式下開啟Relay功能時，在本機資料傳送完成後即進入休眠的問題。
    - 在發送本機資訊後(SNS_SEND_INFO)，含後續傳送A308資料(SNS_SEND_A308_INFO)時間共(30*15)秒後才會進入休眠
    - 承上，收到Client發送的讀取完成訊息(NS_A308_GET_FINISHED)也會進入休眠

### 220714 v1.00 (成) #
1. RS485/BTM Transmitter
    - 修正加入A308客製功能後，RS485/BTM轉傳模式無法正常運作的問題。
    - 修正開始Modbus透傳開始後，電池電量不會再更新的問題。

### 220713 : A308M v1.00 (成) #
1. A308
    - BTM通訊中增加讀取旗標(32bit)，可指定要讀取的分段資料
    - 承上，增加補傳遺失資料機制，補送遺失的部份(5次)。
    - Server: 發送完A308資訊後，在進入睡眠模式前，增加10秒的等待，令Client有機會要求重送遺失資料。
    - Server: 發送完基礎資訊後，等待(15*30)秒後再進入睡眠。此時間應可讓純Relay傳完所有Server(15組)資料。
    - Client: Modbus新增純Relay節點的電池電量讀取。(位置: 0x308)
    - Client: Modbus新增Server電池電量讀取。(位置: 0x31，原先0x308與A308資料衝突)
    - Client: 在所有Server中的A308資料表都接收完成後，廣播完成訊息，令所有待機的節點進入睡眠。

### 220708 : A308M v1.01 (成) #
1. 擴充A308(需定義「BTM_A308」)
    - Server: Modbus讀取來源時將所有資料讀入(9次Modbus通訊)
    - Client: 讀需設備清單指令後，依結果分批將設備資訊讀取回來
    - Client: 建立接收緩衝區，將單一設備讀取完畢後再一次移到目標區中
2. 增加MESH/485直接傳輸模式(需定義「BTM_TRANSMITTER」)
    - 由於加入A308擴充部份，未知對此功能有什麼影響，待測。


### 220623 : G6S-BT v1.01 (成) #
1. 修正在自動模式下的第四段強度無法透過BTM正確回傳目前狀態的問題。

### 220617 : JNC-BT-Mesh v1.24 (成) #
1. 取消對資料夾 IAR ARM - Default - BGM13P32F512GA的版本追蹤
2. 取消對.vscode資料夾的版本追蹤
3. 修正在Server模式在開機時如未讀取到設備後，就再也不會與通訊的問題。
4. 承上，未讀取到設備時會進入中繼模式，在每次喚醒時會嘗試讀取設備資訊。

## v1.20 #

### 211216 (Richard) #
1. Add Server + Relay Node Number to 45 ~ 50
2. UltraSound Add Temp & RH
3. Add 風速計-宇田FMT95/95

## v1.18 #

### 211105 (Richard) #
1. for Android App Setup
2. Modify Gain & Offset report error for App
3. Add function for Utility to get information for 9 register
4. Modify A308M Speed 小數點以下2位
5. Add UltraSound

## v1.16 #

### 210802 (Richard) #
1. add Co2 sensor 
2. add Gain & Offset 
3. Add working Timer and Server Class

## v1.15 #

### 210623 (Richard) #
 1.  Visual Sensor 
 2. Add IAQS and CW9
 
 ## v1.14 #

### 210521 #
1. Add (Skewness, Kurtosis), 
2. Modify UltraSound for WaterLevel
3. Add DO485(specify version),
4. Add PZem(比流器、Specify version), 
5. Add A6D6, 
6. Add Relay status
7. modify AIP status error

## v1.13 #

###  210320 (Richard) #
1.  Bluetooth Mesh first final release.



