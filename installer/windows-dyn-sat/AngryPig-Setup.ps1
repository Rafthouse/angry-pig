#requires -Version 5.1
<#
.SYNOPSIS
  Angry Pig Dynamic Saturation 1.1.1 Beta 1 setup window (Windows PowerShell 5.1, WinForms).
.DESCRIPTION
  Shipped inside the self-extracting setup EXE, next to AngryPig-Install.ps1 and payload.zip.
  The window only drives AngryPig-Install.ps1; the install logic stays in that script.
  -VST3Root, -DataRoot, -SelfTest and -Shot are for sandbox testing and are not used by end users.
#>
param(
    [string]$VST3Root = (Join-Path $env:CommonProgramFiles 'VST3'),
    [string]$DataRoot = (Join-Path $env:ProgramData 'AngryPigDynamicSaturation-Beta1'),
    [switch]$SelfTest,
    [string]$Shot = ''
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$installer = Join-Path $here 'AngryPig-Install.ps1'
$payloadZip = Join-Path $here 'payload.zip'
$payloadRoot = Join-Path $here 'payload'
if (-not (Test-Path -LiteralPath $payloadRoot)) {
    Expand-Archive -LiteralPath $payloadZip -DestinationPath $payloadRoot
}
$bundleTarget = Join-Path $VST3Root 'AngryPigDynamicSaturation.vst3'
$needsAdmin = $VST3Root.StartsWith($env:ProgramFiles, [StringComparison]::OrdinalIgnoreCase)

function Test-Admin {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    (New-Object Security.Principal.WindowsPrincipal $id).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Join-Arguments([string[]]$Items) {
    ($Items | ForEach-Object { if ($_ -match '\s') { '"' + $_ + '"' } else { $_ } }) -join ' '
}

# Runs one installer action. Asks for administrator rights (one UAC prompt) only when the
# VST3 folder is under Program Files and this window is not already elevated.
function Invoke-Setup([string]$Action, [bool]$Replace) {
    $list = @('-NoProfile', '-ExecutionPolicy', 'RemoteSigned', '-File', $installer,
              '-Action', $Action, '-VST3Root', $VST3Root, '-DataRoot', $DataRoot, '-PayloadRoot', $payloadRoot)
    if ($Action -eq 'Install') { $list += '-AcceptLicense' }
    if ($Replace) { $list += '-ReplaceExisting' }
    if ($needsAdmin -and $Action -ne 'Status' -and -not (Test-Admin)) {
        try {
            $p = Start-Process -FilePath 'powershell.exe' -ArgumentList (Join-Arguments $list) -Verb RunAs -WindowStyle Hidden -Wait -PassThru
            return [pscustomobject]@{ Code = $p.ExitCode; Text = '(ran with administrator rights; details are in install.log)' }
        } catch {
            return [pscustomobject]@{ Code = 5; Text = 'Administrator permission was not granted.' }
        }
    }
    $text = (& powershell.exe @list | Out-String)
    return [pscustomobject]@{ Code = $LASTEXITCODE; Text = $text.Trim() }
}

function Get-StatusLine([string]$Prefix) {
    $r = Invoke-Setup 'Status' $false
    $line = $r.Text -split "`r?`n" | Where-Object { $_ -like "$Prefix*" } | Select-Object -First 1
    if (-not $line) { return '' }
    return ($line -replace "^$Prefix\s*", '').Trim()
}

function Get-StateKind {
    $state = Get-StatusLine 'State:'
    if (-not $state) { return 'unknown' }
    return ($state -split '\s+')[0]
}

function Get-Message([int]$Code) {
    switch ($Code) {
        0 { 'Done.' }
        1 { 'Unexpected error. See the log for details: ' + (Join-Path $DataRoot 'install.log') }
        2 { 'The package failed its integrity check. Nothing was changed. Get a fresh copy of the setup.' }
        3 { 'A different AngryPigDynamicSaturation.vst3 is installed. It must be replaced to continue.' }
        4 { 'Tick the terms checkbox before installing.' }
        5 { 'Administrator permission is required for the VST3 folder. Nothing was changed.' }
        6 { 'The copied bundle failed verification. The previous state was restored when a backup existed.' }
        7 { 'No rollback backup was found.' }
        8 { 'A host still has Angry Pig open (for example Ableton Live). Close it and run Setup again. Nothing was changed.' }
        default { "Setup finished with code $Code." }
    }
}

if ($SelfTest) {
    if ($needsAdmin -or $VST3Root.StartsWith(${env:ProgramFiles(x86)}, [StringComparison]::OrdinalIgnoreCase)) {
        throw 'SelfTest runs only against a sandbox VST3 folder outside Program Files.'
    }
    $steps = @(@('Status', $false), @('Install', $false), @('Status', $false), @('Uninstall', $false), @('Status', $false))
    foreach ($s in $steps) {
        $r = Invoke-Setup $s[0] $s[1]
        Write-Output ('{0} -> code {1}' -f $s[0], $r.Code)
    }
    exit 0
}

$bg = [Drawing.Color]::FromArgb(14, 14, 14)
$panel = [Drawing.Color]::FromArgb(30, 30, 30)
$red = [Drawing.Color]::FromArgb(179, 38, 30)
$gold = [Drawing.Color]::FromArgb(212, 169, 60)
$ink = [Drawing.Color]::FromArgb(232, 232, 232)
$muted = [Drawing.Color]::FromArgb(150, 150, 150)

function New-Label([string]$Text, [int]$X, [int]$Y, [int]$W, [int]$H, [Drawing.Color]$Color, [float]$Size, [Drawing.FontStyle]$Style) {
    $l = New-Object Windows.Forms.Label
    $l.Text = $Text
    $l.Location = New-Object Drawing.Point($X, $Y)
    $l.Size = New-Object Drawing.Size($W, $H)
    $l.ForeColor = $Color
    $l.Font = New-Object Drawing.Font('Segoe UI', $Size, $Style)
    return $l
}

function New-Button([string]$Text, [int]$X, [int]$Y, [int]$W, [Drawing.Color]$Back) {
    $b = New-Object Windows.Forms.Button
    $b.Text = $Text
    $b.Location = New-Object Drawing.Point($X, $Y)
    $b.Size = New-Object Drawing.Size($W, 38)
    $b.FlatStyle = 'Flat'
    $b.FlatAppearance.BorderColor = $gold
    $b.FlatAppearance.BorderSize = 1
    $b.BackColor = $Back
    $b.ForeColor = $ink
    $b.Font = New-Object Drawing.Font('Segoe UI', 10, [Drawing.FontStyle]::Bold)
    return $b
}

$form = New-Object Windows.Forms.Form
$form.Text = 'Angry Pig Dynamic Saturation 1.1.1 Beta 1 Setup'
$form.StartPosition = 'CenterScreen'
$form.FormBorderStyle = 'FixedDialog'
$form.MaximizeBox = $false
$form.BackColor = $bg
$form.ClientSize = New-Object Drawing.Size(600, 530)

$title = New-Label 'ANGRY PIG' 24 16 552 44 $gold 26 'Bold'
$subtitle = New-Label 'Windows VST3 - Dynamic Saturation 1.1.1 Beta 1 - unsigned - Created by Mykyta Shchur' 26 62 552 20 $muted 9 'Regular'
$accentBar = New-Object Windows.Forms.Panel
$accentBar.Location = New-Object Drawing.Point(0, 92)
$accentBar.Size = New-Object Drawing.Size(600, 4)
$accentBar.BackColor = $red

$lblTarget = New-Label 'Target' 24 110 90 20 $muted 9 'Regular'
$valTarget = New-Label $bundleTarget 120 110 456 20 $ink 9 'Regular'
$lblState = New-Label 'Installed' 24 134 90 20 $muted 9 'Regular'
$valState = New-Label '...' 120 134 456 20 $ink 9 'Regular'
$lblWebView = New-Label 'WebView2' 24 158 90 20 $muted 9 'Regular'
$valWebView = New-Label '...' 120 158 456 20 $ink 9 'Regular'

$accept = New-Object Windows.Forms.CheckBox
$accept.Text = 'I understand this build is unsigned and want to continue.'
$accept.Location = New-Object Drawing.Point(24, 192)
$accept.Size = New-Object Drawing.Size(552, 24)
$accept.ForeColor = $ink
$accept.BackColor = $bg

$btnInstall = New-Button 'Install' 24 228 150 $red
$btnUninstall = New-Button 'Uninstall' 186 228 150 $panel
$btnRefresh = New-Button 'Refresh' 348 228 120 $panel
$btnClose = New-Button 'Close' 480 228 96 $panel

$lblLog = New-Label 'Log' 24 282 200 20 $muted 9 'Regular'
$log = New-Object Windows.Forms.TextBox
$log.Multiline = $true
$log.ReadOnly = $true
$log.ScrollBars = 'Vertical'
$log.BackColor = $panel
$log.ForeColor = $ink
$log.BorderStyle = 'FixedSingle'
$log.Font = New-Object Drawing.Font('Consolas', 9)
$log.Location = New-Object Drawing.Point(24, 304)
$log.Size = New-Object Drawing.Size(552, 170)

$message = New-Label '' 24 486 552 36 $ink 9 'Regular'

function Set-Message([string]$Text) { $message.Text = $Text }

function Add-Log([string]$Text) {
    if ($Text) { $log.AppendText($Text + [Environment]::NewLine) }
}

function Update-View {
    $state = Get-StatusLine 'State:'
    $kind = if ($state) { ($state -split '\s+')[0] } else { 'unknown' }
    $valState.Text = switch ($kind) {
        'rc1' { 'This version is installed' }
        'other' { 'A different bundle is installed: ' + $state }
        'missing' { 'Not installed' }
        default { 'Unknown' }
    }
    $valWebView.Text = Get-StatusLine 'WebView2:'
}

function Confirm-Action([string]$Text) {
    [Windows.Forms.MessageBox]::Show($Text, 'Angry Pig Dynamic Saturation 1.1.1 Beta 1 Setup', 'YesNo', 'Question') -eq 'Yes'
}

$btnInstall.Add_Click({
    if (-not $accept.Checked) { Set-Message (Get-Message 4); return }
    $replace = $false
    if ((Get-StateKind) -eq 'other') {
        if (-not (Confirm-Action 'A different AngryPigDynamicSaturation.vst3 is installed. It will be copied to ProgramData\AngryPigDynamicSaturation-Beta1\backup and replaced. Continue?')) { return }
        $replace = $true
    }
    $r = Invoke-Setup 'Install' $replace
    Add-Log ('Install -> code ' + $r.Code)
    Add-Log $r.Text
    Set-Message (Get-Message $r.Code)
    Update-View
})

$btnUninstall.Add_Click({
    $kind = Get-StateKind
    if ($kind -eq 'missing') { Set-Message 'Nothing to remove.'; return }
    if (-not (Confirm-Action 'Remove Angry Pig Dynamic Saturation from the VST3 folder? A backup copy stays in ProgramData\AngryPigDynamicSaturation-Beta1\backup.')) { return }
    $r = Invoke-Setup 'Uninstall' ($kind -ne 'rc1')
    Add-Log ('Uninstall -> code ' + $r.Code)
    Add-Log $r.Text
    Set-Message (Get-Message $r.Code)
    Update-View
})

$btnRefresh.Add_Click({ Update-View; Set-Message 'Status refreshed.' })
$btnClose.Add_Click({ $form.Close() })

$form.Controls.AddRange(@($title, $subtitle, $accentBar, $lblTarget, $valTarget, $lblState, $valState,
    $lblWebView, $valWebView, $accept, $btnInstall, $btnUninstall, $btnRefresh, $btnClose, $lblLog, $log, $message))

if ($Shot) {
    $form.Show()
    Update-View
    [Windows.Forms.Application]::DoEvents()
    Start-Sleep -Milliseconds 800
    $bmp = New-Object Drawing.Bitmap $form.Width, $form.Height
    $form.DrawToBitmap($bmp, (New-Object Drawing.Rectangle 0, 0, $form.Width, $form.Height))
    $bmp.Save($Shot, [Drawing.Imaging.ImageFormat]::Png)
    $form.Close()
    exit 0
}

$form.Add_Shown({ Update-View })
[void][Windows.Forms.Application]::Run($form)

