# Runs the existing Dynamic EXP harness (ap-host-pcm, read-only use) against the isolated candidate DLL.
# Settings match the reports that produced the saved dumps: 44.1 kHz, bin 999, 10 s window, ANGER 100%, COLOR 0, GAIN 0 dB.
param(
  [string]$Dll = 'D:\AP-RE\tmp\dyn-sat-recovery\build\out\AngryPigSpike.vst3\Contents\x86_64-win\AngryPigSpike.vst3',
  [string]$Tag = 'c'
)
$harness = 'D:\Codex\Snapshots\2026-10-26\referenced-chatgpt-conversation-this-is-an\work\dynamic-saturation\harness-build\Release\ap-host-pcm.exe'
$runs = 'D:\AP-RE\tmp\dyn-sat-recovery\runs'
$cfg = @('srate=44100', 'bin=999', 'n=44100', 'total=465000')

function Invoke-Run {
  param([string]$Name, [string]$Amount, [string]$Dbfs = '-12', [hashtable]$Env = @{}, [string[]]$Extra = @())
  $env:AP_DYNAMIC_AMOUNT = $Amount
  foreach ($k in $Env.Keys) { Set-Item -Path ("Env:" + $k) -Value $Env[$k] }
  $argList = @('pcm-probe', $Dll, 'dump-only', '1', '0', '0.5714285714', $Dbfs) + $cfg + $Extra + @("dump=$runs\$Name")
  & $harness @argList *> $null
  foreach ($k in $Env.Keys) { Remove-Item -Path ("Env:" + $k) -ErrorAction SilentlyContinue }
  Remove-Item Env:AP_DYNAMIC_AMOUNT -ErrorAction SilentlyContinue
  $h = (Get-FileHash -LiteralPath "$runs\$Name.ap" -Algorithm SHA256).Hash
  '{0,-14} {1}' -f $Name, $h.Substring(0, 16)
}

New-Item -ItemType Directory -Force -Path $runs | Out-Null
Invoke-Run -Name "$Tag`_a0" -Amount '0'
Invoke-Run -Name "$Tag`_a1" -Amount '1'
Invoke-Run -Name "$Tag`_a1_s2" -Amount '1' -Env @{ AP_DYNAMIC_SEED = '12345' }
Invoke-Run -Name "$Tag`_a1_s3" -Amount '1' -Env @{ AP_DYNAMIC_SEED = '777' }
Invoke-Run -Name "$Tag`_a1_m6" -Amount '1' -Dbfs '-6'
Invoke-Run -Name "$Tag`_a1_m24" -Amount '1' -Dbfs '-24'
Invoke-Run -Name "$Tag`_a1_sil" -Amount '1' -Dbfs '-1000'
Invoke-Run -Name "$Tag`_a1_b1" -Amount '1' -Extra @('block=1')
Invoke-Run -Name "$Tag`_a1_b64" -Amount '1' -Extra @('block=64')
Invoke-Run -Name "$Tag`_a1_b127" -Amount '1' -Extra @('block=127')
