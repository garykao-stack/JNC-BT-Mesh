<#
  COM6 擷取 (含時間戳) — 把裝置 debug 輸出寫到檔案，每行前面加上 HH:mm:ss.fff
  用法: powershell -ExecutionPolicy Bypass -File scripts\capture_com6.ps1 -OutFile scripts\clean_com6.txt
  以 Stop-Process 結束 (背景執行)。
#>
param(
  [string]$Port = 'COM6',
  [int]$Baud = 115200,
  [string]$OutFile = 'scripts\clean_com6.txt'
)
$sp = New-Object System.IO.Ports.SerialPort $Port, $Baud, ([System.IO.Ports.Parity]::None), 8, ([System.IO.Ports.StopBits]::One)
$sp.ReadTimeout = 500
$sp.DtrEnable = $true
$sp.RtsEnable = $true
$sp.Open()
$sw = New-Object System.IO.StreamWriter($OutFile, $false)
$sw.AutoFlush = $true
$buf = ''
try {
  while ($true) {
    try { $data = $sp.ReadExisting() } catch { $data = '' }
    if ($data) {
      $buf += $data
      while ($buf.Contains("`n")) {
        $idx = $buf.IndexOf("`n")
        $line = $buf.Substring(0, $idx).TrimEnd("`r")
        $buf = $buf.Substring($idx + 1)
        $ts = (Get-Date).ToString('HH:mm:ss.fff')
        $sw.WriteLine("[$ts] $line")
      }
    }
    Start-Sleep -Milliseconds 30
  }
}
finally {
  if ($buf.Length -gt 0) { $sw.WriteLine("[partial] $buf") }
  $sw.Close(); if ($sp.IsOpen) { $sp.Close() }
}
