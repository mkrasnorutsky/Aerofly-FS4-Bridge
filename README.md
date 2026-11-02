# Aerofly FS4 Bridge DLL (Public)

This fork includes complete routing for all 197 writable SDK message variants,
including trim, parking brake and typed commands. See
[command support and validation](docs/command_support.md) for the protocol,
coverage and limitations. Build with CMake to resolve dependencies.

A multi-interface bridge (Shared Memory, TCP, WebSocket) between Aerofly FS4 and external apps.

## Quick install (users)
1. Download the DLL from GitHub Releases: https://github.com/jlgabriel/Aerofly-FS4-Bridge/releases
2. Copy `AeroflyBridge.dll` to:
   `%USERPROFILE%\Documents\Aerofly FS 4\external_dll\AeroflyBridge.dll`
3. Start Aerofly FS4 and load a flight.

---

## 📦 Simplified Version Available: AeroflyReader DLL

For users who only need to **read flight data** without controlling the aircraft, we offer a lightweight alternative: **AeroflyReader.dll**.

This simplified DLL is ideal for flight loggers, map trackers, stream overlays, and monitoring dashboards where aircraft control is not required.

### Quick Comparison

| Feature | AeroflyReader (Simplified) | AeroflyBridge (Full) |
|---------|---------------------------|----------------------|
| **Lines of code** | ~850 | ~9,000 |
| **Data reading** | ✅ | ✅ |
| **Aircraft control** | ❌ | ✅ |
| **Shared Memory** | ✅ | ✅ |
| **TCP Streaming** | ✅ | ✅ |
| **TCP Commands** | ❌ | ✅ |
| **WebSocket** | ❌ | ✅ |
| **Variables available** | ~50 core | 358 extended |

### When to Use Each Version

- **AeroflyReader (Simplified)**: Flight logging, position tracking, real-time monitors, stream overlays, home cockpit instruments (read-only)
- **AeroflyBridge (Full)**: External control panels, autopilot integrations, command-based applications, web dashboards with controls

### Quick Install (Simplified DLL)

A pre-built DLL is available for immediate use:
```powershell
copy simplified\bin\AeroflyReader.dll "$env:USERPROFILE\Documents\Aerofly FS 4\external_dll\"
```

📁 **See [`simplified/README.md`](simplified/README.md) for full documentation, examples, and build instructions.**

---

## Build from source (developers)

### 📋 **Prerequisites**
⚠️ **Important**: You must download the Aerofly SDK header before compiling:

1. **Download** `tm_external_message.h` from [IPACS Developer Site](https://www.aerofly.com/developers/) (section "External DLL")
2. **Place** the file in the project root directory
3. **Note**: This file is not included in the repository due to IPACS licensing restrictions

### 🔨 **Build Steps**

#### Option 1: CMake (Recommended)

**Using PowerShell script:**
```powershell
# Build Release
.\scripts\build.ps1

# Build Debug
.\scripts\build.ps1 -Config Debug

# Build and install
.\scripts\build.ps1 -Install

# Build and run tests
.\scripts\build.ps1 -Config Debug -Test

# Clean rebuild
.\scripts\build.ps1 -Rebuild
```

**Using CMake directly:**
```powershell
# Configure
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build Release
cmake --build build --config Release

# Build Debug
cmake --build build --config Debug

# Run tests
cmake --build build --config Debug --target run_tests

# Install
cmake --install build --config Release
```

Output: `build/Release/AeroflyBridge.dll` or `build/Debug/AeroflyBridge.dll`

#### Option 2: Legacy Batch Script

```cmd
scripts\compile.bat
```
Output: `dist\AeroflyBridge.dll` (dist/ is gitignored)

## Project Structure

### Core Files
- **`aerofly_bridge_dll.cpp`** - Main DLL source code
- **`tm_external_message.h`** - Aerofly SDK header (download from aerofly.com/developers)
- **`LICENSE`** - MIT License
- **`CHANGELOG.md`** - Version history and release notes

### Directories
- **`docs/`** - Complete documentation and guides (14 files)
- **`tutorials/`** - Step-by-step tutorial guides (5 files)
- **`python/`** - Python example applications (4 files)
- **`web/`** - Standalone HTML demos for WebSocket (3 files)
- **`scripts/`** - Build and release scripts (3 files)
- **`reference/`** - JSON schema and specifications (1 file)
- **`cmake/`** - CMake configuration modules (1 file)
- **`include/`** - Header files (logging system)
- **`src/`** - Source files (logging implementation)
- **`tests/`** - Unit and integration tests (6 test suites)
- **`simplified/`** - Lightweight read-only DLL version (AeroflyReader)
- **`dist/`** - Local build outputs (gitignored)
- **`build/`** - CMake build outputs (gitignored)

## Documentation
See `docs/README.md` for full documentation and index.

## Quick Start Examples

### Web Demos (`web/` folder)
- **`airspeed_envelope.html`** - Horizontal airspeed tape with performance arcs
- **`altitude_monitor.html`** - Minimal altitude display with color-coded cautions
- **`copilot_checklist.html`** - Co-Pilot checklist dashboard

### Python Applications (`python/` folder)
- **`aerofly_fs4_maps_bridge.py`** - Position tracker using TCP data stream
- **`aerofly_realtime_monitor.py`** - Tk desktop monitor (shared memory)
- **`flight_logger.py`** - Comprehensive flight data logger with CSV export
- **`master_control_panel.py`** - Command/control panel (TCP commands)

### Tutorial Guides (`tutorials/` folder)
- **`tutorial_python_flight_logger.md`** - Flight logger to CSV
- **`tutorial_python_maps_bridge.md`** - Maps integration and position tracking
- **`tutorial_websocket_airspeed_envelope.md`** - Airspeed tape with VS0/VFE/VNO/VNE
- **`tutorial_websocket_altitude_monitor.md`** - Minimal altitude monitor (web)
- **`tutorial_websocket_copilot_checklist.md`** - Co-Pilot checklist (web)

Examples quick start:
- Python: `python/python/aerofly_realtime_monitor.py AeroflyBridge_offsets.json`
- Python (commands): `python/python/master_control_panel.py`
- Web: open files in `web/` directly in your browser (require `AEROFLY_BRIDGE_WS_ENABLE=1`)

## Testing

Run automated tests:
```powershell
.\scripts\build.ps1 -Config Debug -Test
```

See [docs/testing.md](docs/testing.md) for details.

## Logging

The bridge includes structured logging with spdlog. Logs are written to:
- Console: Visual Studio Debug Output
- File: `%USERPROFILE%\Documents\Aerofly FS 4\logs\bridge_YYYYMMDD.log`

Configure via environment variables:
```powershell
$env:AEROFLY_BRIDGE_LOG_LEVEL = "debug"  # trace|debug|info|warn|error|critical
```

See [docs/logging.md](docs/logging.md) for details.

## Release

See `docs/release_guide.md` for building the DLL and publishing a GitHub Release.

## AI Authorship

All source code and all documentation in this repository were generated by artificial intelligence (Claude Sonnet 4 and ChatGPT 5), using web tooling and the Cursor IDE.

## License

Licensed under the MIT License. Legal note: Despite AI authorship, the project remains licensed under MIT with no additional restrictions. See the LICENSE file for details.

## Releases (what they are)
GitHub Releases are versioned publications where you upload compiled binaries. They are NOT a folder in the repo. Create a release in the repository’s “Releases” tab (e.g., v1.0.0) and attach `dist\AeroflyBridge.dll` as an asset.

See `CHANGELOG.md` for version history.

---
Copyright (c) 2025 Juan Luis Gabriel

