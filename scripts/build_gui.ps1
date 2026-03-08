# Build and launch GUI with custom CMake
$ErrorActionPreference = "Stop"

$CUSTOM_CMAKE_GUI = "D:\DroneAI\cmake-4.3.0-rc2\bin\cmake-gui.exe"
$CUSTOM_CMAKE = Join-Path (Split-Path $CUSTOM_CMAKE_GUI -Parent) "cmake.exe"
$CUSTOM_CMAKE_ALT = "D:\DroneAI\cmake-4.3.0-rc2-built\bin\cmake.exe"
$SYSTEM_CMAKE_DEFAULT = "C:\Program Files\CMake\bin\cmake.exe"

$SYSTEM_CMAKE = $null
if (Test-Path $SYSTEM_CMAKE_DEFAULT) {
    $SYSTEM_CMAKE = $SYSTEM_CMAKE_DEFAULT
} else {
    $cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmakeCmd -and $cmakeCmd.Source -and -not $cmakeCmd.Source.StartsWith("D:\DroneAI\cmake-4.3.0-rc2", [System.StringComparison]::OrdinalIgnoreCase)) {
        $SYSTEM_CMAKE = $cmakeCmd.Source
    }
}

# Clear stale CMAKE_ROOT before probing any CMake candidate.
if ($env:CMAKE_ROOT) {
    Write-Host "Clearing inherited CMAKE_ROOT: $env:CMAKE_ROOT" -ForegroundColor Yellow
    Remove-Item Env:CMAKE_ROOT -ErrorAction SilentlyContinue
}

function Test-CMakeLayout {
    param([string]$CmakeExe)

    if (-not $CmakeExe -or -not (Test-Path $CmakeExe)) {
        return $false
    }

    $binDir = Split-Path $CmakeExe -Parent
    $rootDir = Split-Path $binDir -Parent
    $shareDir = Join-Path $rootDir "share"

    if (-not (Test-Path $shareDir)) {
        return $false
    }

    $moduleRoots = Get-ChildItem -Path $shareDir -Directory -Filter "cmake-*" -ErrorAction SilentlyContinue
    foreach ($moduleRoot in $moduleRoots) {
        if (Test-Path (Join-Path $moduleRoot.FullName "Modules\CMake.cmake")) {
            return $true
        }
    }

    return $false
}

function Get-UsableCMake {
    param([string[]]$Candidates)

    foreach ($candidate in $Candidates) {
        if (-not $candidate) {
            continue
        }

        if ($candidate -ne "cmake" -and -not (Test-CMakeLayout -CmakeExe $candidate)) {
            Write-Host "Skipping CMake candidate with invalid install layout: $candidate" -ForegroundColor Yellow
            continue
        }

        $probeOutput = $null
        # Probe through cmd with CMAKE_ROOT cleared so poisoned shells do not pass bad candidates.
        if ($candidate -eq "cmake") {
            $probeOutput = cmd /c "set CMAKE_ROOT= && cmake --version 2>&1"
        } else {
            $probeOutput = cmd /c ('set CMAKE_ROOT= && "' + $candidate + '" --version 2>&1')
        }

        $probeText = ($probeOutput | Out-String)
        if ($probeText -match "Could not find CMAKE_ROOT|Modules directory not found") {
            Write-Host "Skipping unusable CMake candidate (CMAKE_ROOT/modules error): $candidate" -ForegroundColor Yellow
            continue
        }

        if ($LASTEXITCODE -eq 0) {
            return $candidate
        }

        Write-Host "Skipping unusable CMake candidate: $candidate" -ForegroundColor Yellow
    }

    throw "No usable CMake executable found."
}

$CMAKE_EXE = Get-UsableCMake -Candidates @($CUSTOM_CMAKE, $CUSTOM_CMAKE_ALT, $SYSTEM_CMAKE)

# Validate selected CMake one more time with CMAKE_ROOT cleared.
if ($CMAKE_EXE -eq "cmake") {
    cmd /c "set CMAKE_ROOT= && cmake --version >nul 2>&1"
} else {
    cmd /c ('set CMAKE_ROOT= && "' + $CMAKE_EXE + '" --version >nul 2>&1')
}
if ($LASTEXITCODE -ne 0) {
    throw "Selected CMake is unusable with a clean CMAKE_ROOT environment: $CMAKE_EXE"
}

if ($CMAKE_EXE -eq $CUSTOM_CMAKE) {
    Write-Host "Using custom CMake (from GUI path): $CUSTOM_CMAKE_GUI" -ForegroundColor Green
} elseif ($CMAKE_EXE -eq $CUSTOM_CMAKE_ALT) {
    Write-Host "Using custom CMake: $CUSTOM_CMAKE_ALT" -ForegroundColor Green
} else {
    Write-Host "Using fallback CMake: $CMAKE_EXE" -ForegroundColor Yellow
    # Prevent accidental resolution of broken rc2 cmake from PATH later in this session.
    $env:Path = (($env:Path -split ';' | Where-Object { $_ -and ($_ -ne "D:\DroneAI\cmake-4.3.0-rc2\bin") }) -join ';')
}

$PROJECT_ROOT = "D:\DroneAI\nanohawk-agent"
Set-Location $PROJECT_ROOT

Write-Host "`n=================================================="
Write-Host "Building nanohawk-agent GUI"
Write-Host "=================================================="
Write-Host "CMake: $CMAKE_EXE"
Write-Host "Project: $PROJECT_ROOT"
Write-Host ""

# Show CMake version
& $CMAKE_EXE --version

Write-Host "`nConfiguring with dev preset..." -ForegroundColor Cyan
& $CMAKE_EXE --preset dev
if ($LASTEXITCODE -ne 0) {
    Write-Host "`nConfiguration failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "`nBuilding GUI target..." -ForegroundColor Cyan
& $CMAKE_EXE --build --preset dev --target nanohawk_agent
if ($LASTEXITCODE -ne 0) {
    Write-Host "`nBuild failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "`n=================================================="
Write-Host "Build completed! Launching GUI..."
Write-Host "==================================================" -ForegroundColor Green
Write-Host ""

$GUI_EXE = "$PROJECT_ROOT\build\dev\nanohawk_agent.exe"

if (Test-Path $GUI_EXE) {
    Write-Host "Launching: $GUI_EXE" -ForegroundColor Green
    & $GUI_EXE
} else {
    Write-Host "ERROR: GUI executable not found at $GUI_EXE" -ForegroundColor Red
    exit 1
}
