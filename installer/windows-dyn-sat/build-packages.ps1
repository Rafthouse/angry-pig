#requires -Version 5.1
<#
.SYNOPSIS
  Builds the Angry Pig Dynamic Saturation Windows packages from a built VST3 bundle.
.DESCRIPTION
  Input is the bundle produced by the CMake build (build\out\AngryPigSpike.vst3). The bundle is
  copied under its release name AngryPigDynamicSaturation.vst3, so it installs next to Angry Pig 1.0.0
  (AngryPigSpike.vst3) instead of over it. File bytes are not changed; only the bundle and DLL are renamed.

  Output in -OutDir:
    AngryPig-DynamicSaturation-<Version>-Windows-Setup.exe     IExpress setup (AngryPig-Setup.ps1 + AngryPig-Install.ps1)
    AngryPig-DynamicSaturation-<Version>-Windows-Portable.zip  bundle + readme + third-party notices
    SHA256SUMS.txt

  Needs: Windows PowerShell 5.1, 7-Zip (7z.exe), iexpress.exe (part of Windows).
#>
param(
    [Parameter(Mandatory = $true)][string]$Bundle,
    [Parameter(Mandatory = $true)][string]$OutDir,
    [string]$Version = '1.1.1-Beta',
    [string]$Notices = '',
    [string]$SevenZip = ''
)
$ErrorActionPreference = 'Stop'
if (-not $Notices) { $Notices = Join-Path $PSScriptRoot '..\..\notices\THIRD_PARTY_NOTICES.txt' }

if (-not $SevenZip) {
    $cmd = Get-Command 7z.exe -ErrorAction SilentlyContinue
    $SevenZip = if ($cmd) { $cmd.Source } else { Join-Path $env:ProgramFiles '7-Zip\7z.exe' }
}
if (-not (Test-Path -LiteralPath $SevenZip)) { throw "7z.exe not found: $SevenZip" }
$iexpress = Join-Path $env:SystemRoot 'System32\iexpress.exe'
if (-not (Test-Path -LiteralPath $iexpress)) { throw "iexpress.exe not found: $iexpress" }

$name = 'AngryPigDynamicSaturation'
$srcDll = Join-Path $Bundle 'Contents\x86_64-win\AngryPigSpike.vst3'
if (-not (Test-Path -LiteralPath $srcDll -PathType Leaf)) { throw "Bundle DLL not found: $srcDll" }

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
$OutDir = (Resolve-Path -LiteralPath $OutDir).Path
$work = Join-Path $OutDir 'work'
if (Test-Path -LiteralPath $work) { Remove-Item -Recurse -Force -LiteralPath $work }
$payloadSrc = Join-Path $work 'payload-src'
$stage = Join-Path $work 'stage'
$portableRoot = Join-Path $work 'portable'
New-Item -ItemType Directory -Force -Path $payloadSrc, $stage, $portableRoot | Out-Null

# 1. Release bundle: same bytes, release name.
$relBundle = Join-Path $payloadSrc "$name.vst3"
Copy-Item -Recurse -LiteralPath $Bundle -Destination $relBundle
Rename-Item -LiteralPath (Join-Path $relBundle 'Contents\x86_64-win\AngryPigSpike.vst3') -NewName "$name.vst3"
$dllHash = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $relBundle "Contents\x86_64-win\$name.vst3")).Hash
if ($dllHash -ne (Get-FileHash -Algorithm SHA256 -LiteralPath $srcDll).Hash) { throw 'DLL changed while renaming' }

# 2. Manifest (SHA-256 of every bundle file), read by AngryPig-Install.ps1.
$files = Get-ChildItem -LiteralPath $relBundle -Recurse -File | Sort-Object FullName | ForEach-Object {
    [pscustomobject]@{ path = $_.FullName.Substring($relBundle.Length).TrimStart('\'); sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $_.FullName).Hash }
}
$manifest = [pscustomobject]@{ bundle = "$name.vst3"; version = $Version; signed = $false; files = @($files) }
[IO.File]::WriteAllText((Join-Path $payloadSrc 'payload-manifest.json'), ($manifest | ConvertTo-Json -Depth 5), (New-Object Text.UTF8Encoding($true)))

# 3. Setup stage and IExpress package.
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'AngryPig-Setup.ps1'), (Join-Path $PSScriptRoot 'AngryPig-Install.ps1'), (Join-Path $PSScriptRoot 'README-INSTALL.txt') -Destination $stage
Push-Location $payloadSrc
& $SevenZip a -tzip -mx=9 (Join-Path $stage 'payload.zip') "$name.vst3" 'payload-manifest.json' | Out-Null
Pop-Location
if ($LASTEXITCODE -ne 0) { throw '7z failed for payload.zip' }

$setupExe = Join-Path $OutDir "AngryPig-DynamicSaturation-$Version-Windows-Setup.exe"
if (Test-Path -LiteralPath $setupExe) { Remove-Item -Force -LiteralPath $setupExe }
$sed = @"
[Version]
Class=IEXPRESS
SEDVersion=3
[Options]
PackagePurpose=InstallApp
ShowInstallProgramWindow=0
HideExtractAnimation=1
UseLongFileName=1
InsideCompressed=0
CAB_FixedSize=0
CAB_ResvCodeSigning=0
RebootMode=N
InstallPrompt=
DisplayLicense=
FinishMessage=
TargetName=$setupExe
FriendlyName=Angry Pig Dynamic Saturation $Version Setup
AppLaunched=powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -WindowStyle Hidden -File AngryPig-Setup.ps1
PostInstallCmd=<None>
AdminQuietInstCmd=
UserQuietInstCmd=
SourceFiles=SourceFiles
[Strings]
FILE0="AngryPig-Setup.ps1"
FILE1="AngryPig-Install.ps1"
FILE2="README-INSTALL.txt"
FILE3="payload.zip"
[SourceFiles]
SourceFiles0=$stage\
[SourceFiles0]
%FILE0%=
%FILE1%=
%FILE2%=
%FILE3%=
"@
$sedPath = Join-Path $work 'setup.sed'
[IO.File]::WriteAllText($sedPath, $sed, (New-Object Text.ASCIIEncoding))
$p = Start-Process -FilePath $iexpress -ArgumentList @('/N', '/Q', $sedPath) -Wait -PassThru -WindowStyle Hidden
if (-not (Test-Path -LiteralPath $setupExe)) { throw "IExpress did not produce $setupExe (exit $($p.ExitCode))" }

# 4. Portable ZIP.
$portName = "AngryPig-DynamicSaturation-$Version-Windows-Portable"
$portDir = Join-Path $portableRoot $portName
New-Item -ItemType Directory -Force -Path $portDir | Out-Null
Copy-Item -Recurse -LiteralPath $relBundle -Destination $portDir
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'README-PORTABLE.txt') -Destination $portDir
Copy-Item -LiteralPath $Notices -Destination (Join-Path $portDir 'THIRD_PARTY_NOTICES.txt')
$zip = Join-Path $OutDir "$portName.zip"
if (Test-Path -LiteralPath $zip) { Remove-Item -Force -LiteralPath $zip }
Push-Location $portableRoot
& $SevenZip a -tzip -mx=9 $zip $portName | Out-Null
Pop-Location
if ($LASTEXITCODE -ne 0) { throw '7z failed for the portable zip' }

# 5. Checksums.
$lines = foreach ($f in @($setupExe, $zip)) { '{0} *{1}' -f (Get-FileHash -Algorithm SHA256 -LiteralPath $f).Hash.ToLowerInvariant(), (Split-Path -Leaf $f) }
[IO.File]::WriteAllLines((Join-Path $OutDir 'SHA256SUMS.txt'), $lines)
"DLL SHA-256: $dllHash"
$lines
