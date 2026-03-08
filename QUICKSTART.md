# Quick Start Guide - Building and Running GUI

## Current Status

The nanohawk-agent project has been configured with build scripts that support using a custom CMake version from `D:\DroneAI\cmake-4.3.0-rc2`.

## Build Scripts Created

### 1. `scripts\build_gui.ps1` (PowerShell)
Complete build and launch script for the GUI.

### 2. `scripts\build_and_run_gui.bat` (Batch)
Batch file version for Windows command prompt.

### 3. `scripts\build.bat` (Batch)
General build script with preset support.

## How to Build and Launch GUI

### Option 1: Using PowerShell (Recommended)

```powershell
cd D:\DroneAI\nanohawk-agent
powershell -ExecutionPolicy Bypass -File .\scripts\build_gui.ps1
```

### Option 2: Using Batch File

```cmd
cd D:\DroneAI\nanohawk-agent
.\scripts\build_and_run_gui.bat
```

### Option 3: Manual CMake Commands

```powershell
cd D:\DroneAI\nanohawk-agent

# Configure
cmake --preset dev

# Build GUI
cmake --build --preset dev --target nanohawk_agent

# Run GUI
.\build\dev\nanohawk_agent.exe
```

## Camera Feed Setup

Once the GUI launches, the camera feed from your USB-connected drone should appear automatically in the VideoPane.

### Prerequisites:
1. **Drone connected via USB** - The FPV camera feed must be available through a USB video capture device
2. **Correct video device index** - Check `config\endpoints.yaml`:
   ```yaml
   video:
     uvc_index: 0  # Try 0, 1, or 2
   ```

### Troubleshooting Video Feed:

If no video appears:

1. **Check Device Manager**:
   - Open Device Manager (Win + X, then M)
   - Look under "Cameras" or "Imaging Devices"
   - Note the index of your USB video capture device

2. **Update video index**:
   ```yaml
   # In config\endpoints.yaml
   video:
     uvc_index: 1  # Try different values: 0, 1, or 2
   ```

3. **Verify camera power**:
   - Ensure the FPV camera is powered on
   - Check USB cable supports data (not charge-only)

## GUI Components

When the GUI launches, you'll see:

- **VideoPane** (Top): Live FPV camera feed from the drone
- **TelemetryPane** (Right): Real-time battery voltage, attitude (roll/pitch/yaw), RC channels
- **PromptPane** (Bottom Left): Natural language command input for AI control
- **MissionPane** (Bottom Center): Mission preview and execution controls
- **SafetyPane** (Bottom Right): Geofence limits, battery guards, safety status

## Next Steps After Successful Build

1. **First launch**: Verify video feed appears
2. **Check telemetry**: Confirm battery and attitude readings
3. **Configure Betaflight**: Set receiver type to **MSP** in Betaflight Configurator
4. **Test simple mission**: Type "Hover at 0.5 meters for 3 seconds" in the PromptPane

## CMake Configuration

The build scripts automatically detect and use CMake in this order:
1. `D:\DroneAI\cmake-4.3.0-rc2-built\bin\cmake.exe` (if built from source)
2. `D:\DroneAI\cmake-4.3.0-rc2\bin\cmake.exe` (if pre-built binary exists)
3. System CMake (fallback)

Currently, the cmake-4.3.0-rc2 directory contains source code. To build it:

```powershell
cd D:\DroneAI\cmake-4.3.0-rc2
.\bootstrap --prefix=D:\DroneAI\cmake-4.3.0-rc2-built
# Wait for bootstrap to complete (may take 5-10 minutes)
# Then the build scripts will automatically use the built version
```

## Common Issues

### Build Fails
- Ensure Qt 6.5+ is installed
- Check that Ninja build system is available
- Verify vcpkg path in `CMakePresets.json` line 14: `"toolchainFile": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"`

### GUI Won't Start
- Check for missing DLLs (Qt, OpenCV, MinGW runtime)
- Run from the build directory to ensure DLLs are found
- Check logs in `logs\` directory

### Drone Not Detected
- Verify COM port in Device Manager (should be COM3 for STM32)
- Update `config\endpoints.yaml` if port differs
- Ensure Betaflight firmware is installed on the drone

## Documentation

- Full README: `README.md`
- Build scripts documentation: `scripts\README.md`
- Configuration details: `docs\CONFIG_WIRING.md`

