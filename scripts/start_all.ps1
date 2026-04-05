param(
    [string]$BuildDir = "build-vs2022",
    [string]$BuildType = "Release",
    [string]$BackendBinDir = "",
    [string]$AlgorithmAddr = "127.0.0.1:50051",
    [string]$StrategyAddr = "127.0.0.1:50052",
    [switch]$SkipNpmInstall,
    [switch]$SkipPortWait
)

$ErrorActionPreference = "Stop"

function Resolve-Executable {
    param(
        [Parameter(Mandatory = $true)][string]$Directory,
        [Parameter(Mandatory = $true)][string[]]$Names
    )

    foreach ($name in $Names) {
        $candidate = Join-Path $Directory $name
        if (Test-Path $candidate) {
            return (Resolve-Path $candidate).Path
        }
    }

    return $null
}

function Split-Address {
    param([Parameter(Mandatory = $true)][string]$Address)

    if ($Address -match "^(?<host>[^:]+):(?<port>\\d+)$") {
        return [PSCustomObject]@{
            Host = $Matches.host
            Port = [int]$Matches.port
        }
    }

    return $null
}

function Wait-ForPort {
    param(
        [Parameter(Mandatory = $true)][string]$Host,
        [Parameter(Mandatory = $true)][int]$Port,
        [int]$TimeoutSeconds = 20
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)

    while ((Get-Date) -lt $deadline) {
        $client = New-Object System.Net.Sockets.TcpClient
        try {
            $async = $client.BeginConnect($Host, $Port, $null, $null)
            if ($async.AsyncWaitHandle.WaitOne(350)) {
                $client.EndConnect($async)
                return $true
            }
        } catch {
            # Keep retrying until timeout.
        } finally {
            $client.Close()
        }

        Start-Sleep -Milliseconds 250
    }

    return $false
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$frontendDir = Join-Path $repoRoot "electron-client"

if (-not (Test-Path $frontendDir)) {
    throw "electron-client directory not found: $frontendDir"
}

$backendDirCandidates = @()
if (-not [string]::IsNullOrWhiteSpace($BackendBinDir)) {
    if ([System.IO.Path]::IsPathRooted($BackendBinDir)) {
        $backendDirCandidates += $BackendBinDir
    } else {
        $backendDirCandidates += (Join-Path -Path $repoRoot -ChildPath $BackendBinDir)
    }
} else {
    $backendDirCandidates += [System.IO.Path]::Combine($repoRoot, $BuildDir, "bin", $BuildType)
    $backendDirCandidates += [System.IO.Path]::Combine($repoRoot, $BuildDir, "bin")
    $backendDirCandidates += [System.IO.Path]::Combine($repoRoot, "build", "bin", $BuildType)
    $backendDirCandidates += [System.IO.Path]::Combine($repoRoot, "build", "bin")
}

$algorithmExe = $null
$strategyExe = $null
$backendDir = $null

foreach ($candidateDir in $backendDirCandidates) {
    if (-not (Test-Path $candidateDir)) {
        continue
    }

    $resolvedAlgorithm = Resolve-Executable -Directory $candidateDir -Names @("algorithm_server.exe", "algorithm_server")
    $resolvedStrategy = Resolve-Executable -Directory $candidateDir -Names @("strategy_server.exe", "strategy_server")

    if ($resolvedAlgorithm -and $resolvedStrategy) {
        $backendDir = (Resolve-Path $candidateDir).Path
        $algorithmExe = $resolvedAlgorithm
        $strategyExe = $resolvedStrategy
        break
    }
}

if (-not $algorithmExe -or -not $strategyExe) {
    throw "Cannot find backend executables. Expected algorithm_server and strategy_server under build output. Try: .\\scripts\\build_windows.ps1 -BuildDir $BuildDir -BuildType $BuildType"
}

if (-not $SkipNpmInstall) {
    Write-Host "`n==> NPM Install" -ForegroundColor Cyan
    Write-Host "npm install"
    Push-Location $frontendDir
    try {
        & npm.cmd install
    } finally {
        Pop-Location
    }

    if ($LASTEXITCODE -ne 0) {
        throw "NPM Install failed with exit code $LASTEXITCODE"
    }
}

Write-Host "`nLaunching backend services..." -ForegroundColor Cyan
$algorithmProc = Start-Process -FilePath "powershell" -ArgumentList @(
    "-NoExit",
    "-Command",
    "& '$algorithmExe'"
) -WorkingDirectory $backendDir -PassThru

$strategyProc = Start-Process -FilePath "powershell" -ArgumentList @(
    "-NoExit",
    "-Command",
    "& '$strategyExe'"
) -WorkingDirectory $backendDir -PassThru

if (-not $SkipPortWait) {
    $algorithmSplit = Split-Address -Address $AlgorithmAddr
    $strategySplit = Split-Address -Address $StrategyAddr

    if ($algorithmSplit) {
        Write-Host "Waiting for algorithm service at $AlgorithmAddr ..." -ForegroundColor DarkCyan
        if (-not (Wait-ForPort -Host $algorithmSplit.Host -Port $algorithmSplit.Port -TimeoutSeconds 20)) {
            Write-Warning "Algorithm service did not open $AlgorithmAddr within timeout. Frontend will still be launched."
        }
    }

    if ($strategySplit) {
        Write-Host "Waiting for strategy service at $StrategyAddr ..." -ForegroundColor DarkCyan
        if (-not (Wait-ForPort -Host $strategySplit.Host -Port $strategySplit.Port -TimeoutSeconds 20)) {
            Write-Warning "Strategy service did not open $StrategyAddr within timeout. Frontend will still be launched."
        }
    }
}

Write-Host "Launching Electron frontend..." -ForegroundColor Cyan
$frontendLaunch = "`$env:ALGORITHM_ADDR='$AlgorithmAddr'; `$env:STRATEGY_ADDR='$StrategyAddr'; npm start"
$frontendProc = Start-Process -FilePath "powershell" -ArgumentList @(
    "-NoExit",
    "-Command",
    $frontendLaunch
) -WorkingDirectory $frontendDir -PassThru

Write-Host "`nAll processes launched." -ForegroundColor Green
Write-Host "Algorithm PID: $($algorithmProc.Id)"
Write-Host "Strategy  PID: $($strategyProc.Id)"
Write-Host "Frontend  PID: $($frontendProc.Id)"
Write-Host "`nTip: close the opened PowerShell windows to stop each service."
