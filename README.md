軟體工具
---
### IDE #
* IAR 8.3以上版本 
* <a href="http://192.168.0.30:88/download/02 技術部/02 開發產品/02 醫療系統/06-藍芽網路感測及接受系統 (Skynet)/08 程式碼燒錄/Silicon Tools/install-studio-v4_x64.exe"> Simplicity 4</a>

### 燒錄程式 #
* <a href="http://192.168.0.30:88/download/02 技術部/02 開發產品/02 醫療系統/06-藍芽網路感測及接受系統 (Skynet)/08 程式碼燒錄/Silicon Tools/commander.zip">commander</a>

### 專案匯出檔 #
* <a href="http://192.168.0.30:88/download/02 技術部/02 開發產品/02 醫療系統/06-藍芽網路感測及接受系統 (Skynet)/08 程式碼燒錄/Silicon Tools/JNC-BT-Mesh.sls">JNC-BT-Mesh.sls</a>


除了IAR之外，以上檔案都可以在檔管中找到：
* <a href="http://192.168.0.30:88/to?path=\02 技術部\02 開發產品\02 醫療系統\06-藍芽網路感測及接受系統 (Skynet)\08 程式碼燒錄\Silicon Tools">\02 技術部\02 開發產品\02 醫療系統\06-藍芽網路感測及接受系統 (Skynet)\08 程式碼燒錄\Silicon Tools</a>

開發準備
---
1. 安裝 Simplicity 4
2. 安裝 IAR
3. 下載 <a href="http://192.168.0.30:88/download/02 技術部/02 開發產品/02 醫療系統/06-藍芽網路感測及接受系統 (Skynet)/08 程式碼燒錄/Silicon Tools/JNC-BT-Mesh.sls">JNC-BT-Mesh.sls</a>
4. 匯入 JNC-BT-Mesh.sls
    - 啟動 Simplicity 4
    - 打開 File > Import
    - 按下Browse按鈕，選擇 JNC-BT-Mesh.sls 所在的資料夾
    - 隨著導引完成有專案匯入
5. 使用Git Pull 所需版本
6. 進行開發...

編譯
---
1. 在 Simplicity 4 選單中選擇 Project > Build Project

&emsp;&emsp;![編譯](doc/img/complie.PNG)

2. bin檔會產生在專案資料夾中的 「IAR ARM - Default」資料夾中
&emsp;&emsp;![檔名為JNC-BT-Mesh.bin](doc/img/binfile.PNG)

燒錄
---
1. 安裝 <a href="http://192.168.0.30:88/download/02 技術部/02 開發產品/02 醫療系統/06-藍芽網路感測及接受系統 (Skynet)/08 程式碼燒錄/Silicon Tools/commander.zip">commander</a>
2. 接上燒錄器並連接 Skynet

&emsp;&emsp;<img src="doc/img/connect_to_device.JPG" width="70%"></img>

3. 啟動 commander

&emsp;&emsp;<img src="doc/img/burn.PNG" width="70%"></img>

4. 按下左上角 Adapter 右邊的 Connect按鈕 
    - 連結成功後按鈕會顯示為Disconnect
5. 按下左上角 Target 右邊的 Connect按鈕 
    - 連結成功後按鈕會顯示為Reconnect，右方的 Device 會顯示出「BGM13P32F512GA」
6. 選擇左邊的 Flash頁籤
7. 在Flash MCU欄位中，按下Browse，並選擇bin檔(JNC-BT-Mesh.bin)
8. 按下Flash按鈕進行燒錄

更新記錄
---
請參閱 [CHANGELOG.MD](CHANGELOG.md)