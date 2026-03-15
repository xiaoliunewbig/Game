param(
    [string]$BuildDir = "build-vs2022",
    [string]$BuildType = "Release",
    [string]$Generator = "Visual Studio 17 2022",
    [string]$Arch = "x64",
    [string]$ToolchainFile = "C:\vcpkg\vcpkg\scripts\buildsystems\vcpkg.cmake",
    [switch]$Clean,
    [switch]$SkipConfigure,
    [switch]$SkipBuild,
    [switch]$SkipTests,
    [switch]$VerifyFrontend
)

$ErrorActionPreference = "Stop"

function Invoke-Step {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string]$FilePath,
        [Parameter(Mandatory = $true)][string[]]$ArgumentList,
        [string]$WorkingDirectory = ""
    )

    Write-Host "`n==> $Name" -ForegroundColor Cyan
    Write-Host "$FilePath $($ArgumentList -join ' ')"

    if ([string]::IsNullOrWhiteSpace($WorkingDirectory)) {
        & $FilePath @ArgumentList
    } else {
        Push-Location $WorkingDirectory
        try {
            & $FilePath @ArgumentList
        } finally {
            Pop-Location
        }
    }

    if ($LASTEXITCODE -ne 0) {
        throw "$Name failed with exit code $LASTEXITCODE"
    }
}

# Detect broken env table (common when conda injects both Path/PATH).
try {
    Get-ChildItem Env: | Out-Null
} catch {
    throw @"
Detected duplicated environment keys (usually Path/PATH conflict from conda shell).
Please run:
  1) conda deactivate
  2) Open a new PowerShell terminal
  3) Re-run this script
"@
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$buildPath = Join-Path $repoRoot $BuildDir

if (-not (Test-Path $ToolchainFile)) {
    throw "Toolchain file not found: $ToolchainFile"
}

if ($Clean -and (Test-Path $buildPath)) {
    Write-Host "Removing build directory: $buildPath" -ForegroundColor Yellow
    Remove-Item -Recurse -Force $buildPath
}

$cmakeExe = "cmake"
$ctestExe = "ctest"

if (-not $SkipConfigure) {
    $configureArgs = @(
        "-S", $repoRoot,
        "-B", $buildPath,
        "-G", $Generator,
        "-A", $Arch,
        "-DCMAKE_TOOLCHAIN_FILE=$ToolchainFile"
    )
    Invoke-Step -Name "Configure" -FilePath $cmakeExe -ArgumentList $configureArgs
}

if (-not $SkipBuild) {
    $buildArgs = @(
        "--build", $buildPath,
        "--config", $BuildType,
        "--parallel"
    )
    Invoke-Step -Name "Build" -FilePath $cmakeExe -ArgumentList $buildArgs
}

if (-not $SkipTests) {
    $testArgs = @(
        "--test-dir", $buildPath,
        "-C", $BuildType,
        "--output-on-failure"
    )
    Invoke-Step -Name "CTest" -FilePath $ctestExe -ArgumentList $testArgs
}

if ($VerifyFrontend) {
    $frontendDir = Join-Path $repoRoot "electron-client"
    if (-not (Test-Path $frontendDir)) {
        throw "electron-client directory not found: $frontendDir"
    }

    Invoke-Step -Name "NPM Install" -FilePath "npm" -ArgumentList @("install") -WorkingDirectory $frontendDir
    Invoke-Step -Name "NPM Verify" -FilePath "npm" -ArgumentList @("run", "verify") -WorkingDirectory $frontendDir
}

Write-Host "`nBuild automation completed successfully." -ForegroundColor Green
