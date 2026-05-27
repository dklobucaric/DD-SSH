<#
DD-SSH Windows checksum helper.

Usage from the repository root:
  powershell -ExecutionPolicy Bypass -File scripts\generate-checksums-windows.ps1

Optional:
  powershell -ExecutionPolicy Bypass -File scripts\generate-checksums-windows.ps1 -ArtifactDir dist -OutputFile dist\SHA256SUMS
#>

param(
    [string]$ArtifactDir = "dist",
    [string]$OutputFile = "dist\SHA256SUMS"
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Resolve-Path (Join-Path $scriptDir "..")
$artifactPath = Resolve-Path -Path (Join-Path $projectRoot $ArtifactDir) -ErrorAction SilentlyContinue

if (-not $artifactPath) {
    Write-Error "Artifact directory not found: $ArtifactDir"
}

$outputPath = Join-Path $projectRoot $OutputFile
$outputParent = Split-Path -Parent $outputPath
New-Item -ItemType Directory -Force -Path $outputParent | Out-Null

$extensions = @(".zip", ".deb", ".dmg", ".AppImage", ".msi", ".pkg", ".tgz")
$artifacts = Get-ChildItem -Path $artifactPath -Recurse -File |
    Where-Object {
        $name = $_.Name
        ($extensions -contains $_.Extension -or $name.EndsWith(".tar.gz", [System.StringComparison]::OrdinalIgnoreCase)) -and
        $name -ne "SHA256SUMS"
    } |
    Sort-Object FullName

if (-not $artifacts -or $artifacts.Count -eq 0) {
    Write-Error "No release artifacts found under $artifactPath. Expected .zip, .deb, .dmg, .AppImage, .msi, .pkg, .tar.gz, or .tgz."
}

$lines = foreach ($artifact in $artifacts) {
    $hash = (Get-FileHash -Algorithm SHA256 -Path $artifact.FullName).Hash.ToLowerInvariant()
    $relative = [System.IO.Path]::GetRelativePath($projectRoot.Path, $artifact.FullName) -replace '\\','/'
    "$hash  $relative"
}

$lines | Set-Content -Path $outputPath -Encoding ASCII
Write-Host "[DD-SSH] SHA256 checksums written to: $outputPath"
Get-Content -Path $outputPath
