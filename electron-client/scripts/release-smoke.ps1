param(
    [string]$ProjectRoot = "D:\Game\Game\electron-client",
    [switch]$UseNoEdit,
    [switch]$SkipLaunchCheck
)

$ErrorActionPreference = "Stop"

function Invoke-Step {
    param(
        [string]$Name,
        [scriptblock]$Action
    )

    Write-Host $Name
    & $Action
    if ($LASTEXITCODE -ne 0) {
        throw "Step failed: $Name (exit code $LASTEXITCODE)"
    }
}

function Get-ArtifactCandidate {
    param([string]$DistPath)

    Get-ChildItem -Path $DistPath -Recurse -File |
        Where-Object { $_.Extension -in ".exe", ".msi", ".zip" } |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1
}

Push-Location $ProjectRoot
try {
    Invoke-Step "[1/7] Verify JavaScript syntax" {
        node --check "main.js"
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        node --check "preload.js"
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        node --check "src/renderer.js"
    }

    Invoke-Step "[2/7] Verify npm scripts" {
        npm run verify
    }

    Invoke-Step "[3/7] Install deps if needed" {
        if (-not (Test-Path "node_modules")) {
            npm install
        }
    }

    if ($UseNoEdit) {
        Invoke-Step "[4/7] Build Windows package (noedit)" {
            npm run dist:win:noedit
        }
    } else {
        Invoke-Step "[4/7] Build Windows package" {
            npm run dist:win
        }
    }

    Write-Host "[5/7] Check build artifacts"
    $distPath = Join-Path $ProjectRoot "dist"
    if (-not (Test-Path $distPath)) {
        throw "dist folder was not generated"
    }

    $artifact = Get-ArtifactCandidate -DistPath $distPath
    if (-not $artifact) {
        throw "No Windows artifact (.exe/.msi/.zip) found in dist"
    }

    Write-Host "[6/7] Validate artifact metadata"
    $artifactSizeBytes = [int64]$artifact.Length
    $artifactSizeMB = [Math]::Round($artifactSizeBytes / 1MB, 2)
    if ($artifactSizeBytes -lt 5MB) {
        throw "Artifact is unexpectedly small (<5MB): $($artifact.FullName)"
    }

    $hash = Get-FileHash -Path $artifact.FullName -Algorithm SHA256

    $launchStatus = "SKIPPED"
    $launchDetails = "Not requested"

    if (-not $SkipLaunchCheck) {
        Write-Host "[7/7] Launch probe (win-unpacked executable)"
        $unpackedExe = Get-ChildItem -Path (Join-Path $distPath "win-unpacked") -Filter "*.exe" -File -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -ne "elevate.exe" } |
            Select-Object -First 1

        if (-not $unpackedExe) {
            $launchStatus = "SKIPPED"
            $launchDetails = "win-unpacked executable not found"
        } else {
            $proc = $null
            try {
                $proc = Start-Process -FilePath $unpackedExe.FullName -PassThru
                Start-Sleep -Seconds 8
                if ($proc -and -not $proc.HasExited) {
                    $launchStatus = "PASS"
                    $launchDetails = "Process started and remained alive for probe window"
                    Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
                } else {
                    $launchStatus = "PASS"
                    $launchDetails = "Process started and exited during probe window"
                }
            } catch {
                $launchStatus = "FAIL"
                $launchDetails = $_.Exception.Message
                throw "Launch probe failed: $launchDetails"
            } finally {
                if ($proc -and -not $proc.HasExited) {
                    Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
                }
            }
        }
    }

    $reportPath = Join-Path $distPath "artifact-report.txt"
    $report = @(
        "Artifact Report",
        "GeneratedAt: $([DateTime]::Now.ToString('yyyy-MM-dd HH:mm:ss'))",
        "ArtifactPath: $($artifact.FullName)",
        "ArtifactSizeBytes: $artifactSizeBytes",
        "ArtifactSizeMB: $artifactSizeMB",
        "ArtifactSHA256: $($hash.Hash)",
        "LaunchProbe: $launchStatus",
        "LaunchProbeDetails: $launchDetails"
    ) -join [Environment]::NewLine
    Set-Content -Path $reportPath -Value $report -Encoding utf8

    Write-Host "Release smoke check passed. Artifact: $($artifact.FullName)"
    Write-Host "Artifact report: $reportPath"
}
finally {
    Pop-Location
}
