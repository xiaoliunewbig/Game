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
    [switch]$VerifyFrontend,
    [switch]$SkipPathNormalization
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

function Normalize-ProcessPathEnvironment {
    # Some shells (notably conda-initialized ones) can leave both Path and PATH
    # in process env, which breaks MSBuild tool invocation on Windows.
    $pathValue = [System.Environment]::GetEnvironmentVariable("Path", "Process")
    if ([string]::IsNullOrWhiteSpace($pathValue)) {
        $pathValue = [System.Environment]::GetEnvironmentVariable("PATH", "Process")
    }

    [System.Environment]::SetEnvironmentVariable("PATH", $null, "Process")
    [System.Environment]::SetEnvironmentVariable("Path", $pathValue, "Process")
}

if (-not $SkipPathNormalization) {
    Normalize-ProcessPathEnvironment
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
