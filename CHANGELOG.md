## v1.24 #
---
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



