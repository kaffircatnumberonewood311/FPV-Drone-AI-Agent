# Build Scripts

This directory contains scripts for building and running the nanohawk-agent project with a custom CMake version.

## Custom CMake Configuration

The project is configured to use CMake from: `D:\DroneAI\cmake-4.3.0-rc2`

## Available Scripts

### build.bat
Main build script using custom CMake.

Usage:
```powershell
# Build with CI preset (default, no GUI)
.\scripts\build.bat

# Build with dev preset (includes GUI)
.\scripts\build.bat dev
```

### build_and_run_gui.bat
Builds and launches the GUI application.

Usage:
```powershell
.\scripts\build_and_run_gui.bat
```

This script will:
1. Configure with the `dev` preset
2. Build the GUI target
3. Launch the GUI application

### set_cmake_env.bat
Sets up the CMake environment variables for manual commands.

Usage:
```powershell
# Set environment in current session
call .\scripts\set_cmake_env.bat

# Then run CMake commands manually
cmake --preset dev
cmake --build --preset dev
```

### start_gui.bat (Updated)
Runs the GUI executable if already built. Now includes custom CMake path.

### Other Scripts
- `start_cli_interactive.bat` - Run CLI in interactive mode
- `start_launcher.bat` - Run the system tray launcher
- `start_llm_server.bat` - Start the local LLM server
- `start_sitl.bat` - Start SITL simulation
- `demo_cli.bat` - Run CLI demo

## Quick Start

### First Time Build

```powershell
# Build everything (CLI + tests, no GUI)
.\scripts\build.bat

# Or build with GUI
.\scripts\build.bat dev
```

### Build and Run GUI

```powershell
.\scripts\build_and_run_gui.bat
```

### Show Camera Feed from Drone

Prerequisites:
1. Drone connected via USB
2. FPV camera feed available via USB capture device
3. Built with GUI enabled (dev preset)

Steps:
```powershell
# 1. Build with GUI
.\scripts\build_and_run_gui.bat

# 2. GUI will launch and should show:
#    - VideoPane: Live FPV camera feed
#    - TelemetryPane: Battery, attitude, RC channels
#    - PromptPane: Natural language command input
#    - MissionPane: Mission preview and execution
#    - SafetyPane: Geofence and safety limits
```

The video feed is configured in `config\endpoints.yaml`:
```yaml
video:
  uvc_index: 0  # USB video device index (0 = first camera)
```

If you see no feed, try changing `uvc_index` to 1 or 2.

## Manual Build Commands

If you prefer to run CMake manually:

```powershell
# Set environment
call .\scripts\set_cmake_env.bat

# Configure
cmake --preset dev

# Build
cmake --build --preset dev

# Run tests
ctest --preset dev

# Run GUI
.\build\dev\nanohawk_agent.exe
```

## Troubleshooting

**CMake not found**
- Verify that `D:\DroneAI\cmake-4.3.0-rc2\bin\cmake.exe` exists
- Run `.\scripts\set_cmake_env.bat` and then `cmake --version`

**GUI build fails**
- Ensure Qt 6.5+ is installed and findable by CMake
- Check that `NANOHAWK_BUILD_GUI` is ON in the preset

**No video feed in GUI**
- Check Device Manager for USB video capture device
- Try different `uvc_index` values in `config\endpoints.yaml`
- Ensure the FPV camera is powered on

**Drone not detected**
- Check Device Manager for STM32 Virtual COM Port
- Update `serial_port` in `config\endpoints.yaml` if needed
- Ensure USB cable supports data (not charge-only)

