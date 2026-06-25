<#
  JNC-BT-Mesh — 序列埠 Debug 監看工具 (Serial Monitor)
  ----------------------------------------------------------------
  韌體 debug 輸出走 USART0 (VCOM)：PA0 = TX / PA1 = RX，8N1，115200 bps。
  把產品的 debug TX(PA0) 接到 USB-UART 轉換器，會在電腦出現一個 COM 埠。

  用法 (PowerShell)：
    powershell -ExecutionPolicy Bypass -File scripts\serial_monitor.ps1
    powershell -ExecutionPolicy Bypass -File scripts\serial_monitor.ps1 -Port COM6 -Baud 115200

  按 Ctrl+C 結束。
#>
param(
  [string]$Port = 'COM6',
  [int]$Baud = 115200
)

Write-Host "==== JNC-BT-Mesh Serial Monitor ====" -ForegroundColor Cyan
Write-Host ("連線: {0} @ {1} 8N1   (Ctrl+C 結束)" -f $Port, $Baud) -ForegroundColor Cyan
Write-Host "Debug 來源: USART0 (VCOM) PA0=TX / PA1=RX" -ForegroundColor DarkGray
Write-Host "------------------------------------"

$sp = New-Object System.IO.Ports.SerialPort $Port, $Baud, ([System.IO.Ports.Parity]::None), 8, ([System.IO.Ports.StopBits]::One)
$sp.ReadTimeout = 500
$sp.DtrEnable = $true
$sp.RtsEnable = $true
try {
  $sp.Open()
}
catch {
  Write-Host ("無法開啟 {0}: {1}" -f $Port, $_.Exception.Message) -ForegroundColor Red
  Write-Host "請確認: (1) 埠號正確 (2) 沒有其他終端機程式佔用該埠 (3) 裝置已接上" -ForegroundColor Yellow
  return
}
try {
  while ($true) {
    try {
      $data = $sp.ReadExisting()
      if ($data) { [Console]::Write($data) }
    }
    catch [TimeoutException] { }
    Start-Sleep -Milliseconds 50
  }
}
finally {
  if ($sp.IsOpen) { $sp.Close() }
  Write-Host "`n------------------------------------"
  Write-Host "序列埠已關閉。" -ForegroundColor Cyan
}
