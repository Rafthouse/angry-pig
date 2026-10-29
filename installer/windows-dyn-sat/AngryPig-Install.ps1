#requires -Version 5.1
<#
.SYNOPSIS
  Angry Pig Dynamic Saturation 1.1.1 Beta 1 installer core (unsigned build).

.DESCRIPTION
  Actions: Status (default), Install, Uninstall, Rollback.
  Touches only <VST3Root>\AngryPigDynamicSaturation.vst3 and <DataRoot>\. Never edits other plugins.
  Existing bundles that are not this build (for example Windows Build 01) are never overwritten
  silently: Install and Uninstall refuse with exit code 3 unless -ReplaceExisting is passed,
  and in that case the old bundle is copied to <DataRoot>\backup\<stamp>\ first.

  Exit codes:
    0  success (or nothing to do)
    1  unexpected error (see install.log)
    2  payload or backup integrity check failed
    3  conflict with an existing different bundle (re-run with -ReplaceExisting)
    4  consent missing (Install requires -AcceptLicense)
    5  elevation required for the target folder
    6  post-copy verification failed; previous state restored when a backup existed
    7  rollback backup not found
    8  target bundle is in use by another process (for example a running DAW); nothing was changed

.EXAMPLE
  .\AngryPig-Install.ps1 -Action Status
  .\AngryPig-Install.ps1 -Action Install -AcceptLicense
  .\AngryPig-Install.ps1 -Action Install -AcceptLicense -ReplaceExisting
  .\AngryPig-Install.ps1 -Action Rollback -BackupPath <DataRoot>\backup\<stamp>
  .\AngryPig-Install.ps1 -Action Uninstall
#>
[CmdletBinding()]
param(
    [ValidateSet('Install', 'Uninstall', 'Status', 'Rollback')]
    [string]$Action = 'Status',
    [string]$VST3Root = (Join-Path $env:CommonProgramFiles 'VST3'),
    [string]$PayloadRoot = (Join-Path $PSScriptRoot 'payload'),
    [string]$DataRoot = (Join-Path $env:ProgramData 'AngryPigDynamicSaturation-Beta1'),
    [string]$BackupPath = '',
    [switch]$AcceptLicense,
    [switch]$ReplaceExisting
)

$ErrorActionPreference = 'Stop'

$BundleName = 'AngryPigDynamicSaturation.vst3'
$DllRel = 'Contents\x86_64-win\AngryPigDynamicSaturation.vst3'
$WebView2Client = '{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}'
$LogFile = Join-Path $DataRoot 'install.log'
$Stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$target = Join-Path $VST3Root $BundleName
$needsAdmin = $VST3Root.StartsWith($env:ProgramFiles, [StringComparison]::OrdinalIgnoreCase)

function Write-Log([string]$Text) {
    if (-not (Test-Path -LiteralPath $DataRoot)) { New-Item -ItemType Directory -Force -Path $DataRoot | Out-Null }
    $line = '{0} {1} {2}' -f (Get-Date -Format 'yyyy-MM-ddTHH:mm:ss'), $Action, $Text
    Add-Content -LiteralPath $LogFile -Value $line -Encoding UTF8
    Write-Host $Text
}

function Get-Hash([string]$Path) {
    (Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash
}

function Read-Manifest {
    $path = Join-Path $PayloadRoot 'payload-manifest.json'
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "payload-manifest.json not found in $PayloadRoot" }
    Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
}

# Returns the list of mismatches between a bundle folder and the manifest. Empty list = match.
function Test-Bundle([string]$BundleRoot, $Manifest) {
    $bad = @()
    foreach ($f in $Manifest.files) {
        $p = Join-Path $BundleRoot $f.path
        if (-not (Test-Path -LiteralPath $p -PathType Leaf)) { $bad += "missing: $($f.path)"; continue }
        if ((Get-Hash $p) -ne $f.sha256) { $bad += "sha mismatch: $($f.path)" }
    }
    return ,$bad
}

# Builds a manifest from a folder's current contents (used to verify a backup copy during rollback).
function Get-FolderManifest([string]$Root) {
    $files = Get-ChildItem -LiteralPath $Root -Recurse -File | ForEach-Object {
        [pscustomobject]@{ path = $_.FullName.Substring($Root.Length).TrimStart('\'); sha256 = Get-Hash $_.FullName }
    }
    [pscustomobject]@{ files = @($files) }
}

function Get-State($Manifest) {
    if (-not (Test-Path -LiteralPath $target)) {
        return [pscustomobject]@{ Kind = 'missing'; Sha = ''; Build01 = $false }
    }
    $dll = Join-Path $target $DllRel
    $sha = ''
    if (Test-Path -LiteralPath $dll -PathType Leaf) { $sha = Get-Hash $dll }
    $expected = ($Manifest.files | Where-Object { $_.path -eq $DllRel } | Select-Object -First 1).sha256
    $mismatch = Test-Bundle $target $Manifest
    $kind = 'other'
    if ($sha -and ($sha -eq $expected) -and ($mismatch.Count -eq 0)) { $kind = 'rc1' }
    # Windows Build 01 DLL is documented only as BD47F9...CB8 (full hash not recorded).
    $build01 = ($sha -like 'BD47F9*') -and ($sha -like '*CB8')
    return [pscustomobject]@{ Kind = $kind; Sha = $sha; Build01 = [bool]$build01 }
}

function Test-Admin {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    (New-Object Security.Principal.WindowsPrincipal $id).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# Returns the first file in a bundle that another process holds open, or $null.
# Checked before anything is removed, so a locked bundle is refused with nothing changed.
function Get-LockedFile([string]$BundleRoot) {
    foreach ($f in Get-ChildItem -LiteralPath $BundleRoot -Recurse -File -Force) {
        try {
            $stream = [IO.File]::Open($f.FullName, 'Open', 'ReadWrite', 'None')
            $stream.Close()
        } catch {
            return $f.FullName
        }
    }
    return $null
}

function Get-WebView2Status {
    foreach ($root in @('HKLM:\SOFTWARE\WOW6432Node', 'HKLM:\SOFTWARE', 'HKCU:\SOFTWARE')) {
        $key = Join-Path $root "Microsoft\EdgeUpdate\Clients\$WebView2Client"
        if (Test-Path -LiteralPath $key) {
            $ver = (Get-ItemProperty -LiteralPath $key -ErrorAction SilentlyContinue).pv
            if ($ver) { return "present (version $ver)" }
        }
    }
    return 'NOT DETECTED. Install the Microsoft Edge WebView2 Runtime yourself; this script does not install it.'
}

# Copies the current bundle into <DataRoot>\backup\<stamp>\ together with a metadata file.
function Save-Backup($State) {
    $dir = Join-Path (Join-Path $DataRoot 'backup') $Stamp
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
    Copy-Item -Recurse -LiteralPath $target -Destination (Join-Path $dir $BundleName)
    $meta = [pscustomobject]@{ sha256 = $State.Sha; kind = $State.Kind; created = $Stamp; source = $target }
    $meta | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $dir 'backup-meta.json') -Encoding UTF8
    Write-Log "backup=$dir previous_sha=$($State.Sha) previous_kind=$($State.Kind)"
    return $dir
}

# Stages a copy of a bundle next to the target, verifies it, then swaps it into place.
function Put-Bundle([string]$SourceBundle, $Manifest, [bool]$TargetExists, [string]$BackupDir) {
    New-Item -ItemType Directory -Force -Path $VST3Root | Out-Null
    $staging = Join-Path $VST3Root ".$BundleName.staging-$Stamp"
    Copy-Item -Recurse -LiteralPath $SourceBundle -Destination $staging
    $bad = Test-Bundle $staging $Manifest
    if ($bad.Count -gt 0) {
        Remove-Item -Recurse -Force -LiteralPath $staging
        $bad | ForEach-Object { Write-Host $_ }
        Write-Log 'result=copy-verify-failed target-unchanged'
        exit 6
    }
    if ($TargetExists) {
        $locked = Get-LockedFile $target
        if ($locked) {
            Remove-Item -Recurse -Force -LiteralPath $staging
            Write-Host "In use by another process: $locked"
            Write-Log 'result=target-in-use target-unchanged'
            exit 8
        }
        Remove-Item -Recurse -Force -LiteralPath $target
    }
    Rename-Item -LiteralPath $staging -NewName $BundleName
    $bad = Test-Bundle $target $Manifest
    if ($bad.Count -gt 0) {
        Remove-Item -Recurse -Force -LiteralPath $target
        if ($BackupDir) { Copy-Item -Recurse -LiteralPath (Join-Path $BackupDir $BundleName) -Destination $target }
        $bad | ForEach-Object { Write-Host $_ }
        Write-Log "result=post-install-verify-failed restored=$([bool]$BackupDir)"
        exit 6
    }
}

try {
    if ($Action -eq 'Status') {
        $manifest = Read-Manifest
        $st = Get-State $manifest
        Write-Host "Target:      $target"
        Write-Host "State:       $($st.Kind)  sha256=$($st.Sha)"
        Write-Host "Build 01:    $($st.Build01)"
        Write-Host "WebView2:    $(Get-WebView2Status)"
        if (Test-Path -LiteralPath $LogFile) { Get-Content -LiteralPath $LogFile -Tail 5 }
        exit 0
    }

    $manifest = Read-Manifest
    $payloadBundle = Join-Path $PayloadRoot $BundleName

    if ($needsAdmin -and -not (Test-Admin)) {
        Write-Log 'result=elevation-required (run from an elevated PowerShell)'
        exit 5
    }

    if ($Action -eq 'Install') {
        if (-not $AcceptLicense) {
            Write-Log 'result=consent-missing (pass -AcceptLicense)'
            exit 4
        }
        $bad = Test-Bundle $payloadBundle $manifest
        if ($bad.Count -gt 0) {
            $bad | ForEach-Object { Write-Host $_ }
            Write-Log 'result=payload-integrity-failed'
            exit 2
        }
        $st = Get-State $manifest
        if ($st.Kind -eq 'rc1') {
            Write-Log "result=already-installed sha=$($st.Sha)"
            exit 0
        }
        if ($st.Kind -ne 'missing' -and -not $ReplaceExisting) {
            Write-Log "result=conflict existing_sha=$($st.Sha) build01=$($st.Build01)"
            Write-Host 'A different AngryPigDynamicSaturation.vst3 is already installed. Re-run with -ReplaceExisting to back it up and replace it.'
            exit 3
        }
        $backupDir = $null
        if ($st.Kind -ne 'missing') { $backupDir = Save-Backup $st }
        Put-Bundle $payloadBundle $manifest ($st.Kind -ne 'missing') $backupDir
        Write-Log "result=installed sha=$(Get-Hash (Join-Path $target $DllRel))"
        Write-Host "WebView2:    $(Get-WebView2Status)"
        exit 0
    }

    if ($Action -eq 'Uninstall') {
        $st = Get-State $manifest
        if ($st.Kind -eq 'missing') {
            Write-Log 'result=nothing-to-remove'
            exit 0
        }
        if ($st.Kind -ne 'rc1' -and -not $ReplaceExisting) {
            Write-Log "result=conflict-uninstall existing_sha=$($st.Sha) build01=$($st.Build01)"
            Write-Host 'The installed bundle is not this build. Re-run with -ReplaceExisting to back it up and remove it.'
            exit 3
        }
        $locked = Get-LockedFile $target
        if ($locked) {
            Write-Host "In use by another process: $locked"
            Write-Log 'result=target-in-use target-unchanged'
            exit 8
        }
        if ($st.Kind -ne 'rc1') { $null = Save-Backup $st }
        Remove-Item -Recurse -Force -LiteralPath $target
        Write-Log "result=removed previous_sha=$($st.Sha) (backups kept)"
        exit 0
    }

    if ($Action -eq 'Rollback') {
        if (-not $BackupPath) {
            Write-Host 'Rollback needs -BackupPath <DataRoot>\backup\<stamp>'
            exit 7
        }
        $bundleBak = Join-Path $BackupPath $BundleName
        $metaPath = Join-Path $BackupPath 'backup-meta.json'
        if (-not (Test-Path -LiteralPath $bundleBak) -or -not (Test-Path -LiteralPath $metaPath)) {
            Write-Log "result=backup-not-found path=$BackupPath"
            exit 7
        }
        $meta = Get-Content -LiteralPath $metaPath -Raw | ConvertFrom-Json
        if ((Get-Hash (Join-Path $bundleBak $DllRel)) -ne $meta.sha256) {
            Write-Log 'result=backup-integrity-failed'
            exit 2
        }
        $st = Get-State $manifest
        $currentBackup = $null
        if ($st.Kind -ne 'missing') { $currentBackup = Save-Backup $st }
        Put-Bundle $bundleBak (Get-FolderManifest $bundleBak) ($st.Kind -ne 'missing') $currentBackup
        Write-Log "result=rolled-back sha=$(Get-Hash (Join-Path $target $DllRel)) from=$BackupPath"
        exit 0
    }
}
catch {
    Write-Log "result=error $($_.Exception.Message)"
    exit 1
}
