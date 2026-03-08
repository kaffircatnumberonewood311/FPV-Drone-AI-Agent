# Build Configuration Summary

## Date: March 8, 2026

## Custom CMake Setup

The nanohawk-agent project has been configured to use a custom CMake version located at:
```
D:\DroneAI\cmake-4.3.0-rc2
```

**Note**: This directory currently contains CMake source code. The build scripts will automatically fall back to the system CMake (version 4.1.1) until the custom version is built.

### To Build Custom CMake (Optional):

```powershell
cd D:\DroneAI\cmake-4.3.0-rc2
.\bootstrap --prefix=D:\DroneAI\cmake-4.3.0-rc2-built
```

This will take 5-10 minutes to complete. After building, the scripts will automatically detect and use it.

## Build Scripts Created

### Main Build Script (Root Directory)
- **`BUILD_GUI.bat`** - Simple, reliable GUI build and launch script

### Scripts Directory
- **`build.bat`** - General purpose build script with preset support
- **`build_and_run_gui.bat`** - GUI-specific build and run
- **`build_gui.ps1`** - PowerShell version with colored output
- **`set_cmake_env.bat`** - Set CMake environment variables
- **`start_gui.bat`** - Updated to include custom CMake path

### Documentation
- **`scripts\README.md`** - Complete guide to all build scripts
- **`QUICKSTART.md`** - Quick start guide for building and running

## How to Build and Launch the GUI

### Simplest Method:

Double-click or run:
```cmd
BUILD_GUI.bat
```

This will:
1. Configure the project with the `dev` preset (GUI enabled)
2. Build the `nanohawk_agent` target
3. Launch the GUI application

### Alternative Methods:

**Using scripts directory:**
```cmd
.\scripts\build_and_run_gui.bat
```

**Using PowerShell:**
```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_gui.ps1
```

**Manual CMake:**
```cmd
cmake --preset dev
cmake --build --preset dev --target nanohawk_agent
.\build\dev\nanohawk_agent.exe
```

## Current Build Status

### CI Build (No GUI) - ? EXISTS
Location: `D:\DroneAI\nanohawk-agent\build\ci\`

Executables found:
- `nanohawk_unit_tests.exe` (3/7/2026 10:28 PM)
- `nanohawk_transport_tests.exe` (3/7/2026 10:28 PM)
- `launcher.exe` (3/8/2026 7:27 AM)
- `nanohawk_config_tests.exe` (3/8/2026 7:28 AM)
- `nanohawk_agent_cli.exe` (3/8/2026 7:34 AM)
- `nanohawk_device_detection.exe` (3/8/2026 7:34 AM)

### Dev Build (With GUI) - ? PENDING
Location: `D:\DroneAI\nanohawk-agent\build\dev\`

Status: Not yet built. Run `BUILD_GUI.bat` to create it.

## Camera Feed Configuration

The GUI is configured to show the live FPV camera feed from your drone in the VideoPane.

### Configuration File:
`config\endpoints.yaml`

```yaml
video:
  uvc_index: 0  # USB video device index (0 = first camera)
```

### If Video Doesn't Appear:

1. Check Device Manager for your USB video capture device
2. Try different `uvc_index` values: 0, 1, or 2
3. Ensure the FPV camera is powered on
4. Verify USB cable supports data transmission

## GUI Components

When launched, the GUI displays:

| Component | Location | Function |
|-----------|----------|----------|
| **VideoPane** | Top | Live FPV camera feed from drone |
| **TelemetryPane** | Right | Battery, attitude (roll/pitch/yaw), RC channels |
| **PromptPane** | Bottom Left | Natural language command input |
| **MissionPane** | Bottom Center | Mission JSON preview and execution |
| **SafetyPane** | Bottom Right | Geofence limits, safety status |

## Prerequisites for Full Functionality

### Required:
- ? Windows 10+ (Confirmed)
- ? CMake 3.26+ (System CMake 4.1.1 available)
- ? C++20 compiler (MinGW-w64 confirmed)
- ? Ninja build system (Confirmed in CMakePresets.json)

### For GUI:
- ?? Qt 6.5+ (Required - check if installed)
- ?? OpenCV 4.8+ (Required - check if installed)
- ?? libcurl 7.85+ (Required - check if installed)

### For Drone Communication:
- ? Drone connected via USB (COM3)
- ?? Betaflight receiver type set to **MSP** (Must configure in Betaflight Configurator)

### For AI Control:
- ?? Local LLM server (llama.cpp) running on port 8080
- ?? GGUF model file in `models\llm\`

## Next Steps

1. **Build the GUI**: Run `BUILD_GUI.bat`
2. **Verify GUI launches**: Check that all panes are visible
3. **Test camera feed**: Confirm video appears in VideoPane
4. **Configure Betaflight**: Set receiver to MSP mode
5. **Start LLM server**: Launch llama.cpp for AI control
6. **First test flight**: Indoor, clear area, simple prompt

## Troubleshooting

### Build Errors

**Qt Not Found:**
```
Install Qt 6.5+ from https://www.qt.io/download
Set Qt6_DIR environment variable to Qt CMake directory
```

**vcpkg Toolchain Error:**
```
Update line 14 in CMakePresets.json:
"toolchainFile": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
or remove it if vcpkg is not installed
```

### Runtime Errors

**Missing DLL:**
```
Copy required DLLs to build\dev\ or add to PATH:
- Qt DLLs (Qt6Core.dll, Qt6Gui.dll, Qt6Widgets.dll)
- OpenCV DLLs (opencv_world4xx.dll)
- MinGW runtime (libgcc_s_seh-1.dll, libstdc++-6.dll)
```

**No Video Feed:**
```
1. Open Device Manager
2. Check "Cameras" or "Imaging Devices"
3. Update uvc_index in config\endpoints.yaml
4. Restart GUI
```

**Drone Not Detected:**
```
1. Check Device Manager for "STM32 Virtual COM Port"
2. Note the COM port number
3. Update config\endpoints.yaml:
   serial_port: COM3  # or your COM port
4. Restart GUI
```

## Files Modified/Created

### New Files:
- `BUILD_GUI.bat` - Root directory quick launch script
- `scripts\build.bat` - General build script
- `scripts\build_and_run_gui.bat` - GUI build and run
- `scripts\build_gui.ps1` - PowerShell version
- `scripts\set_cmake_env.bat` - Environment setup
- `scripts\README.md` - Scripts documentation
- `QUICKSTART.md` - Quick start guide
- `BUILD_SUMMARY.md` - This file

### Modified Files:
- `scripts\start_gui.bat` - Added custom CMake path support

## Support

For issues or questions:
1. Check `QUICKSTART.md` for common solutions
2. Review `scripts\README.md` for detailed build instructions
3. Check `README.md` for architecture and design details
4. Review `docs\CONFIG_WIRING.md` for configuration details

