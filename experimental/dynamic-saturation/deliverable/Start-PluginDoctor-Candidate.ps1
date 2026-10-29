[CmdletBinding()]
param(
    [switch]$ValidateOnly
)

# Opens Plugin Doctor with the isolated Dynamic Saturation candidate (AB configuration) in chain slot 1.
# Does not install, copy or replace anything. The installed Angry Pig 1.0.0 in Program Files is only checked, never touched.

$ErrorActionPreference = 'Stop'
$doctor = 'C:\Program Files\DDMF\PluginDoctor64.exe'
$dll = Join-Path $PSScriptRoot 'AngryPigSpike.vst3\Contents\x86_64-win\AngryPigSpike.vst3'
$expected = '38593B36A72019FBBC2F179F5EDD4F2E744094D362E027ECFF1E98D4ED4AC28B'
$installed = 'C:\Program Files\Common Files\VST3\AngryPigSpike.vst3\Contents\x86_64-win\AngryPigSpike.vst3'

if (-not (Test-Path -LiteralPath $doctor -PathType Leaf)) { throw "Plugin Doctor missing: $doctor" }
if (-not (Test-Path -LiteralPath $dll -PathType Leaf)) { throw "Candidate DLL missing: $dll" }
$actual = (Get-FileHash -LiteralPath $dll -Algorithm SHA256).Hash
if ($actual -ne $expected) { throw "Candidate DLL SHA-256 mismatch: $actual" }
Write-Output "Verified candidate DLL: $dll"
Write-Output "SHA-256: $actual"
Write-Output 'Configuration: AP_DYNAMIC_AMOUNT=1, AP_DYNAMIC_COMPONENTS=AB (C off)'
if (Test-Path -LiteralPath $installed -PathType Leaf) {
    Write-Output "Installed production (not touched): $((Get-FileHash -LiteralPath $installed -Algorithm SHA256).Hash.Substring(0,16))..."
}
if ($ValidateOnly) { Write-Output 'Validation only; Plugin Doctor was not launched.'; return }

$running = @(Get-Process -Name PluginDoctor64 -ErrorAction SilentlyContinue)
if ($running.Count -gt 0) {
    $ids = ($running | ForEach-Object { $_.Id }) -join ', '
    throw "Close all existing PluginDoctor64 windows/processes first (PID: $ids). No process was closed or replaced."
}

$previousAmount = [Environment]::GetEnvironmentVariable('AP_DYNAMIC_AMOUNT', 'Process')
$previousComponents = [Environment]::GetEnvironmentVariable('AP_DYNAMIC_COMPONENTS', 'Process')
try {
    [Environment]::SetEnvironmentVariable('AP_DYNAMIC_AMOUNT', '1', 'Process')
    [Environment]::SetEnvironmentVariable('AP_DYNAMIC_COMPONENTS', 'AB', 'Process')
    $started = Start-Process -FilePath $doctor -ArgumentList ('"{0}"' -f $dll) -WorkingDirectory (Split-Path -Parent $doctor) -WindowStyle Normal -PassThru
}
finally {
    [Environment]::SetEnvironmentVariable('AP_DYNAMIC_AMOUNT', $previousAmount, 'Process')
    [Environment]::SetEnvironmentVariable('AP_DYNAMIC_COMPONENTS', $previousComponents, 'Process')
}

$observed = $false
$collision = $false
for ($attempt = 0; $attempt -lt 30; $attempt++) {
    Start-Sleep -Milliseconds 300
    $proc = Get-Process -Id $started.Id -ErrorAction SilentlyContinue
    if (-not $proc) { break }
    try {
        $modules = @($proc.Modules | Where-Object { $_.ModuleName -eq 'AngryPigSpike.vst3' })
        $observed = @($modules | Where-Object { $_.FileName -ieq $dll }).Count -gt 0
        $collision = @($modules | Where-Object { $_.FileName -ieq $installed }).Count -gt 0
        if ($observed -or $collision) { break }
    }
    catch { }
}
if (-not $observed) {
    Write-Warning 'The candidate DLL was not observed in this process. Do not read the measurements as the candidate until its loaded module path is confirmed.'
}
else { Write-Output "Verified loaded candidate DLL in PluginDoctor64 PID $($started.Id)." }
if ($collision) {
    Write-Warning 'The same process also loaded the installed Angry Pig 1.0.0. Remove it from any chain and restart Plugin Doctor before comparing.'
}
