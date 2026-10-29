#requires -Version 5.1
# Isolated install / uninstall / reinstall / conflict / rollback QA for the Windows Setup package.
# Never touches Program Files: the VST3 root and data root are folders under -Work.
param(
    [Parameter(Mandatory = $true)][string]$SetupExe,
    [Parameter(Mandatory = $true)][string]$ExpectedDllSha256,
    [Parameter(Mandatory = $true)][string]$Work,
    [string]$SevenZip = ''
)
$ErrorActionPreference = 'Continue'
if (-not $SevenZip) { $c = Get-Command 7z.exe -ErrorAction SilentlyContinue; $SevenZip = if ($c) { $c.Source } else { Join-Path $env:ProgramFiles '7-Zip\7z.exe' } }
$Q = $Work
Remove-Item -Recurse -Force -LiteralPath $Q -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $Q | Out-Null
& $SevenZip x -y $SetupExe "-o$(Join-Path $Q 'exe-extract')" | Out-Null
& $SevenZip x -y (Join-Path $Q 'exe-extract\payload.zip') "-o$(Join-Path $Q 'payload')" | Out-Null
$inst = Join-Path $Q 'exe-extract\AngryPig-Install.ps1'
$payload = Join-Path $Q 'payload'
$vst = Join-Path $Q 'vst3-root'
$data = Join-Path $Q 'data'
$bundle = Join-Path $vst 'AngryPigDynamicSaturation.vst3'
$dllRel = 'Contents\x86_64-win\AngryPigDynamicSaturation.vst3'
$expected = $ExpectedDllSha256.ToUpperInvariant()
$script:failures = 0
function Invoke-Inst([string]$Action, [string[]]$Extra = @()) {
    $psArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $inst, '-Action', $Action,
                '-VST3Root', $vst, '-DataRoot', $data, '-PayloadRoot', $payload) + $Extra
    $out = (& powershell.exe @psArgs 2>&1 | Out-String).Trim()
    $code = $LASTEXITCODE
    $label = (@($Action) + $Extra) -join ' '
    Write-Host ("[{0}] exit={1}" -f $label, $code)
    foreach ($line in ($out -split "`r?`n")) { if ($line) { Write-Host ("    " + $line) } }
    return $code
}
function Get-DllHash {
    $p = Join-Path $bundle $dllRel
    if (Test-Path -LiteralPath $p -PathType Leaf) { (Get-FileHash -Algorithm SHA256 -LiteralPath $p).Hash } else { '(none)' }
}
function Check([string]$Name, [bool]$Ok) {
    if (-not $Ok) { $script:failures++ }
    Write-Output ("CHECK {0}: {1}" -f $Name, $(if ($Ok) { 'PASS' } else { 'FAIL' }))
}

Remove-Item -Recurse -Force -LiteralPath $vst, $data -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $vst, $data | Out-Null

Write-Output '== A. install / uninstall / reinstall (isolated VST3 root)'
$c = Invoke-Inst 'Status'
Check 'fresh root: DLL absent' ((Get-DllHash) -eq '(none)')
$c = Invoke-Inst 'Install'
Check 'install without consent refused (exit 4)' ($c -eq 4)
Check 'refusal changed nothing' (-not (Test-Path -LiteralPath $bundle))
$c = Invoke-Inst 'Install' @('-AcceptLicense')
Check 'install exit 0' ($c -eq 0)
Check 'installed DLL SHA-256 = verified build' ((Get-DllHash) -eq $expected)
$c = Invoke-Inst 'Install' @('-AcceptLicense')
Check 'second install = already installed (exit 0)' ($c -eq 0)
$c = Invoke-Inst 'Uninstall'
Check 'uninstall exit 0' ($c -eq 0)
Check 'uninstall removed the bundle' (-not (Test-Path -LiteralPath $bundle))
$c = Invoke-Inst 'Uninstall'
Check 'uninstall again = nothing to remove (exit 0)' ($c -eq 0)
$c = Invoke-Inst 'Install' @('-AcceptLicense')
Check 'reinstall exit 0' ($c -eq 0)
Check 'reinstalled DLL SHA-256 = verified build' ((Get-DllHash) -eq $expected)

Write-Output '== B. conflict with a different build under the same bundle name'
$c = Invoke-Inst 'Uninstall'
Copy-Item -Recurse -LiteralPath (Join-Path $payload 'AngryPigDynamicSaturation.vst3') -Destination $bundle
$planted = Join-Path $bundle $dllRel
$fs = [IO.File]::Open($planted, 'Append', 'Write')
$fs.WriteByte(0)
$fs.Close()
$plantedHash = Get-DllHash
Check 'planted test build differs from verified' ($plantedHash -ne $expected)
$c = Invoke-Inst 'Install' @('-AcceptLicense')
Check 'conflict refused (exit 3)' ($c -eq 3)
Check 'conflict left planted build in place' ((Get-DllHash) -eq $plantedHash)
$c = Invoke-Inst 'Install' @('-AcceptLicense', '-ReplaceExisting')
Check 'replace exit 0' ($c -eq 0)
Check 'replaced DLL SHA-256 = verified build' ((Get-DllHash) -eq $expected)
$bk = Get-ChildItem -LiteralPath (Join-Path $data 'backup') -Directory | Sort-Object Name | Select-Object -Last 1
Check 'backup of planted build exists' ($null -ne $bk -and (Test-Path -LiteralPath (Join-Path $bk.FullName 'backup-meta.json')))
$c = Invoke-Inst 'Rollback' @('-BackupPath', $bk.FullName)
Check 'rollback exit 0' ($c -eq 0)
Check 'rollback restored planted build' ((Get-DllHash) -eq $plantedHash)
$c = Invoke-Inst 'Uninstall'
Check 'uninstall of non-verified build refused (exit 3)' ($c -eq 3)
$c = Invoke-Inst 'Uninstall' @('-ReplaceExisting')
Check 'uninstall with -ReplaceExisting exit 0' ($c -eq 0)
Check 'bundle removed after replace-uninstall' (-not (Test-Path -LiteralPath $bundle))

Write-Output '== C. final state: verified build installed in isolated root'
$c = Invoke-Inst 'Install' @('-AcceptLicense')
Check 'final install exit 0' ($c -eq 0)
Check 'final DLL SHA-256 = verified build' ((Get-DllHash) -eq $expected)
Write-Output '== isolated install.log (tail)'
Get-Content -LiteralPath (Join-Path $data 'install.log') -Tail 30
Write-Output ("RESULT failures=" + $script:failures)
exit $script:failures
