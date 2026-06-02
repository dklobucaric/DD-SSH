<#
DD-SSH Windows portable ZIP helper.

Run after scripts\windows-deploy-release.bat has produced dist\windows-release.

Usage:
  powershell -ExecutionPolicy Bypass -File scripts\windows-package-portable.ps1

Optional overrides:
  powershell -ExecutionPolicy Bypass -File scripts\windows-package-portable.ps1 -Version 0.1.8.7 -Arch x86_64
#>

param(
    [string]$Version = "",
    [string]$Arch = "x86_64",
    [string]$DistDir = "dist\windows-release",
    [string]$OutputDir = "dist"
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Resolve-Path (Join-Path $scriptDir "..")

function Get-ProjectVersion {
    $cmakeFile = Join-Path $projectRoot "CMakeLists.txt"
    if (-not (Test-Path $cmakeFile)) {
        return "0.0.0"
    }

    $content = Get-Content -Path $cmakeFile -Raw
    $match = [regex]::Match($content, 'set\s*\(\s*DD_SSH_VERSION_STRING\s+"dev\s+([^"]+)"')
    if ($match.Success) {
        return $match.Groups[1].Value
    }

    $match = [regex]::Match($content, 'set\s*\(\s*DD_SSH_VERSION_STRING\s+"v?([0-9][^"]*)"')
    if ($match.Success) {
        return $match.Groups[1].Value
    }

    return "0.0.0"
}

if ([string]::IsNullOrWhiteSpace($Version)) {
    $Version = Get-ProjectVersion
}

$distPath = Resolve-Path -Path (Join-Path $projectRoot $DistDir) -ErrorAction SilentlyContinue
if (-not $distPath) {
    Write-Error "Windows portable folder not found: $DistDir. Run scripts\windows-deploy-release.bat first."
}

$exePath = Join-Path $distPath "dd-ssh.exe"
if (-not (Test-Path $exePath)) {
    Write-Error "dd-ssh.exe not found in $DistDir. Run scripts\windows-deploy-release.bat first."
}

$outputPath = Join-Path $projectRoot $OutputDir
New-Item -ItemType Directory -Force -Path $outputPath | Out-Null

$zipName = "DD-SSH-$Version-Windows-$Arch-portable.zip"
$zipPath = Join-Path $outputPath $zipName

if (Test-Path $zipPath) {
    Remove-Item -Force $zipPath
}

Write-Host "[DD-SSH] Package version: $Version"
Write-Host "[DD-SSH] Creating Windows portable ZIP: $zipPath"
Compress-Archive -Path (Join-Path $distPath "*") -DestinationPath $zipPath -CompressionLevel Optimal
Write-Host "[DD-SSH] Windows portable ZIP ready: $zipPath"
