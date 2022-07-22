## v1.24 #
---
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
---
### 211216 (Richard) #
1. Add Server + Relay Node Number to 45 ~ 50
2. UltraSound Add Temp & RH
3. Add 風速計-宇田FMT95/95

## v1.18 #
---
### 211105 (Richard) #
1. for Android App Setup
2. Modify Gain & Offset report error for App
3. Add function for Utility to get information for 9 register
4. Modify A308M Speed 小數點以下2位
5. Add UltraSound

## v1.16 #
---
### 210802 (Richard) #
1. add Co2 sensor 
2. add Gain & Offset 
3. Add working Timer and Server Class

## v1.15 #
---
### 210623 (Richard) #
 1.  Visual Sensor 
 2. Add IAQS and CW9
 
 ## v1.14 #
---
### 210521 #
1. Add (Skewness, Kurtosis), 
2. Modify UltraSound for WaterLevel
3. Add DO485(specify version),
4. Add PZem(比流器、Specify version), 
5. Add A6D6, 
6. Add Relay status
7. modify AIP status error

## v1.13 #
---
###  210320 (Richard) #
1.  Bluetooth Mesh first final release.



