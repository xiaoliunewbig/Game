param(
    [string]$BuildDir = "build-vs2022",
    [string]$BuildType = "Release",
    [string]$ToolchainFile = "C:\vcpkg\vcpkg\scripts\buildsystems\vcpkg.cmake",
    [switch]$Clean,
    [switch]$SkipPathNormalization
)

$ErrorActionPreference = "Stop"

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildScript = Join-Path $scriptRoot "build_windows.ps1"

& $buildScript `
    -BuildDir $BuildDir `
    -BuildType $BuildType `
    -ToolchainFile $ToolchainFile `
    -VerifyFrontend `
    -Clean:$Clean `
    -SkipPathNormalization:$SkipPathNormalization

if ($LASTEXITCODE -ne 0) {
    throw "Full verify failed with exit code $LASTEXITCODE"
}

Write-Host "`nFull backend + frontend verification completed." -ForegroundColor Green
