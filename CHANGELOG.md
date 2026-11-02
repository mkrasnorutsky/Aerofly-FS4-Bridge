## 0.4.1 — Complete writable command routing

- Route every writable SDK catalog variant, including trim and parking brake.
- Preserve Value/Step/Move/Toggle/Event semantics with optional `flag` selection.
- Support typed vector and UTF-16 string commands, strict JSON and SDK access checks.
- Keep existing dotted propeller/engine aliases; reject read-only telemetry writes.
- Replace duplicated hand-written command routing with the shared catalog.
- Add exhaustive production-router tests and real-DLL TCP/WebSocket tests.
- Fix upstream logger inclusion, controller namespace conflict and test linking.

# Changelog

All notable changes to this project will be documented in this file.

This project adheres to Keep a Changelog and Semantic Versioning.

## [v0.4.0] - 2025-11-20

### 🚀 Major Infrastructure Improvements

#### **NEW: CMake Build System**
- **Modern build infrastructure** with CMake 3.15+ support
- **Cross-configuration builds**: Debug and Release builds in same project
- **Automatic dependency management**: spdlog and Catch2 fetched automatically via FetchContent
- **Flexible SDK detection**: Searches multiple locations for `tm_external_message.h`
- **PowerShell build script** (`scripts/build.ps1`) with convenient options:
  - `-Config Debug|Release`: Build configuration selection
  - `-Test`: Run tests after building
  - `-Install`: Install DLL to Aerofly FS4 directory
  - `-Rebuild`: Clean rebuild from scratch
- **Visual Studio 2022 and 2019 support** with automatic detection
- **Legacy compile.bat maintained** for backward compatibility

#### **NEW: Structured Logging System**
- **spdlog v1.12.0 integration** for high-performance async logging
- **Six log levels**: TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL
- **Multiple outputs**:
  - Console: Visual Studio Debug Output (OutputDebugString)
  - File: `%USERPROFILE%\Documents\Aerofly FS 4\logs\bridge_YYYYMMDD.log`
- **Rotating log files**: Max 5MB, keeps 3 files
- **Configurable via environment variables**:
  - `AEROFLY_BRIDGE_LOG_LEVEL`: Set minimum log level
  - `AEROFLY_BRIDGE_LOG_FILE`: Enable/disable file logging
  - `AEROFLY_BRIDGE_LOG_CONSOLE`: Enable/disable console logging
- **Format strings**: Python-style formatting with compile-time type checking
- **Zero overhead**: TRACE/DEBUG compiled out in Release builds
- **Thread-safe**: Async logging doesn't block critical paths

#### **NEW: Automated Testing Framework**
- **Catch2 v3.5.0** modern C++ testing framework
- **Test organization**:
  - **Unit tests** (3 test suites): Fast, isolated component tests
    - `test_variable_mapper.cpp`: Hash map O(1) lookup verification
    - `test_json_builder.cpp`: JSON structure and encoding validation
    - `test_command_processor.cpp`: Command parsing and validation
  - **Integration tests** (3 test suites): System-level testing
    - `test_shared_memory.cpp`: Memory mapping and synchronization
    - `test_tcp_server.cpp`: Server, connections, broadcasting
    - `test_websocket_server.cpp`: RFC 6455 compliance, handshake, frames
- **CTest integration**: Run tests with `ctest` or `cmake --build --target run_tests`
- **Test helpers**: Common utilities for async testing, float comparison, mock data
- **Tagged tests**: Filter by `[unit]`, `[integration]`, or specific components

#### **Code Quality Improvements**
- **Migrated from DBG() macro to structured logging**: All debug output now uses LOG_* macros
- **Improved error messages**: Contextual information in all log statements
- **Better exception handling**: Detailed error logging with context
- **Thread-safe logging**: All threads can log without contention

#### **Documentation Enhancements**
- **NEW: `docs/testing.md`**: Complete guide to writing and running tests
- **NEW: `docs/logging.md`**: Logging system configuration and usage
- **Updated `README.md`**:
  - CMake build instructions
  - Testing section
  - Logging configuration
- **Improved `CMakeLists.txt`** with inline documentation

#### **Files Added**
```
CMakeLists.txt                      # Main CMake configuration
cmake/CompilerWarnings.cmake        # Compiler warning settings
scripts/build.ps1                   # PowerShell build wrapper
include/logging/logger.h            # Logging system header
src/logging/logger.cpp              # Logging implementation
tests/CMakeLists.txt                # Test build configuration
tests/main.cpp                      # Test entry point
tests/test_helpers.h                # Test utilities
tests/unit/*.cpp                    # Unit tests (3 files)
tests/integration/*.cpp             # Integration tests (3 files)
docs/testing.md                     # Testing guide
docs/logging.md                     # Logging guide
```

#### **Updated Files**
- `aerofly_bridge_dll.cpp`: Logging integration, LOG_* macros
- `.gitignore`: CMake build directories, logs, dependencies
- `README.md`: CMake build steps, testing, logging sections
- `CHANGELOG.md`: This changelog

#### **Technical Details**
- **Build system**: CMake 3.15+, Visual Studio 2019/2022
- **Dependencies**: spdlog (header-only), Catch2 v3 (FetchContent)
- **C++ Standard**: C++17 (unchanged)
- **Output**: `build/Release/AeroflyBridge.dll` or `build/Debug/AeroflyBridge.dll`
- **Tests**: 6 test suites, 50+ test cases

#### **Breaking Changes**
- None - fully backward compatible with v0.3.1
- Old `compile.bat` still works
- Existing applications continue to work without changes

#### **Migration Notes**
- **For developers**: Switch to CMake for better build experience
  ```powershell
  .\scripts\build.ps1 -Config Release -Install
  ```
- **For users**: No changes needed, download DLL from releases as before

#### **Benefits of v0.4.0**
- ✅ **Faster development**: Modern build system, instant feedback from tests
- ✅ **Better debugging**: Structured logs with timestamps, thread IDs, context
- ✅ **Higher quality**: Automated tests prevent regressions
- ✅ **Easier contributions**: Clear build process, test framework, logging
- ✅ **Production ready**: Comprehensive logging for troubleshooting issues

---

## [v0.3.1] - 2025-08-19

### 🏆 Complete Hash Map Migration - 100% Optimization

#### **MAJOR ACHIEVEMENT: Full Variable Migration Complete**
- **Migrated** additional **104 variables** from if-else chains to hash map lookup
- **Achieved** 100% migration of all major flight simulation variables (222 total)
- **Optimized** all critical aircraft systems to O(1) performance
- **Increased** DLL size to 912KB due to comprehensive hash map implementation

#### **Newly Migrated Systems (104 variables)**

**Aircraft Physics & Dynamics (7 variables)**
- `Aircraft.Gravity`, `Aircraft.Wind`, `Aircraft.RateOfTurn`, `Aircraft.MachNumber`
- `Aircraft.AngleOfAttack`, `Aircraft.AngleOfAttackLimit`, `Aircraft.AccelerationLimit`

**Aircraft State Management (8 variables)**
- `Aircraft.OnGround`, `Aircraft.OnRunway`, `Aircraft.Crashed`, `Aircraft.Gear`
- `Aircraft.Flaps`, `Aircraft.Slats`, `Aircraft.Throttle`, `Aircraft.AirBrake`

**Performance Speed Limits (5 variables)**
- `Performance.Speed.VS0`, `Performance.Speed.VS1`, `Performance.Speed.VFE`
- `Performance.Speed.VNO`, `Performance.Speed.VNE`

**Aircraft Information Strings (5 variables)**
- `Aircraft.NearestAirportIdentifier`, `Aircraft.NearestAirportName`
- `Aircraft.BestAirportIdentifier`, `Aircraft.BestAirportName`, `Aircraft.BestRunwayIdentifier`

**Engine Individual Controls (12 variables)**
- `Aircraft.Engine.Running.1-4`, `Aircraft.Starter1-4`, `Aircraft.Ignition1-4`

**Flight Management & Simulation (2 variables)**
- `FMS.FlightNumber`, `Simulation.Time`

**Advanced Autopilot Controls (15 variables)**
- Numeric: `Autopilot.Engaged`, `SelectedAirspeed`, `SelectedHeading`, `SelectedAltitude`, `SelectedVerticalSpeed`, `ThrottleEngaged`
- String modes: `ActiveLateralMode`, `ActiveVerticalMode`, `ArmedLateralMode`, `ArmedVerticalMode`, `ArmedApproachMode`, `ActiveAutoThrottleMode`, `ActiveCollectiveMode`, `ArmedCollectiveMode`, `Type`

**Advanced Controls (9 variables)**
- `Controls.WheelBrake.Left/Right`, `Controls.AirBrake`, `Controls.AirBrake.Arm`
- `Controls.PropellerSpeed.1-4`, `Controls.GliderAirBrake`, `Controls.RotorBrake`

**Aircraft Systems (6 variables)**
- `Aircraft.GroundSpoilersArmed/Extended`, `Aircraft.ParkingBrake`
- `Aircraft.AutoBrakeSetting/Engaged/RejectedTakeOff`

**Engine System Controls (13 variables)**
- `Aircraft.Starter`, `Aircraft.Ignition`, `Aircraft.EngineMaster.1-4`
- `Aircraft.Engine.Throttle.1-4`, `Aircraft.Engine.RotationSpeed.1-4`

**Warning Systems (4 variables)**
- `Aircraft.MasterWarning`, `Aircraft.MasterCaution`
- `Aircraft.LowOilPressure`, `Aircraft.LowFuelPressure`

**Aircraft Extended Controls (12 variables)**
- `Aircraft.Height`, `Aircraft.Power`, `Aircraft.NormalizedPower`, `Aircraft.NormalizedPowerTarget`
- `Aircraft.Trim`, `Aircraft.PitchTrim`, `Aircraft.PitchTrimScaling`, `Aircraft.PitchTrimOffset`
- `Aircraft.RudderTrim`, `Aircraft.AutoPitchTrim`, `Aircraft.YawDamperEnabled`, `Aircraft.RudderPedalsDisconnected`

**Aircraft Location (4 variables - Vector types)**
- `Aircraft.NearestAirportLocation`, `Aircraft.BestAirportLocation` (Vector3D→Vector2D)
- `Aircraft.BestRunwayThreshold`, `Aircraft.BestRunwayEnd` (Vector3D)

**Final Control Variables (2 variables)**
- `Aircraft.ThrottleLimit`, `Aircraft.Reverse`

#### **Technical Improvements**
- **Special handling** for Vector3D/Vector2D types with direct memory assignment
- **Read-only optimization** for simulator-provided data (strings, locations)
- **Type conversion** handling for uint32_t variables (warnings, states)
- **Complete cleanup** of all redundant if-else fallback code
- **Zero compilation errors** and full backward compatibility maintained

#### **Performance Impact**
- ✅ **222 variables** now use O(1) hash map lookup (vs O(n) linear search)
- ✅ **100% coverage** of all major flight simulation systems
- ✅ **Massive performance improvement** for multi-engine aircraft operations
- ✅ **Optimized memory access** patterns for real-time simulation data

#### **Systems 100% Migrated**
- ✅ Navigation & Communication Systems
- ✅ Autopilot & Flight Management
- ✅ Engine Controls & Monitoring  
- ✅ Aircraft State & Physics
- ✅ Control Surfaces & Systems
- ✅ Warning & Safety Systems
- ✅ Performance & Limits
- ✅ Location & Positioning

## [v0.3.0] - 2025-08-18

### 🚀 Major Performance Optimization

#### **BREAKING CHANGE: Hash Map Optimization**
- **Optimized** message processing from **O(n) to O(1)** using hash maps
- **Migrated** 118 variables from if-else chains to hash map lookup
- **Improved** performance by ~35% for most commonly used variables
- **Reduced** DLL size through code cleanup and redundancy elimination

#### **Hash Map Infrastructure Implementation**
- **Added** `std::unordered_map<uint64_t, MessageHandler>` for SharedMemoryInterface
- **Added** `std::unordered_map<std::string, CommandHandler>` for CommandProcessor  
- **Created** InitializeHandlers() methods to populate hash maps with lambda functions
- **Maintained** fallback system for non-migrated variables (backward compatibility)

#### **Variables Migrated to Hash Map (118 total)**

**Step 1: Infrastructure + Examples (27 variables)**
- Aircraft basic data: Latitude, Longitude, Altitude, Pitch, Bank, IndicatedAirspeed
- Controls: Throttle, PitchInput, RollInput, YawInput, Gear, Flaps
- Vector3D types: Position, Velocity, Acceleration
- Step controls: C172 windows/doors
- String handlers: Aircraft name and identifiers

**Step 2: Additional Variables (16 variables)**
- Controls.Throttle1-4, AirBrake, WheelBrake.Left/Right, Collective
- Aircraft.TrueHeading, MagneticHeading, GroundSpeed, VerticalSpeed, Height, Orientation, UniversalTime, IndicatedAirspeedTrend

**Step 3: Major Systems (75 variables)**
- **Navigation.* (30 variables)**: NAV1/NAV2 frequencies, DME1/DME2 data, ILS1/ILS2 systems, ADF1/ADF2 equipment, course selections, identifiers
- **Communication.* (11 variables)**: COM1/COM2/COM3 frequencies, standby frequencies, frequency swaps, transponder code/cursor
- **Aircraft.Engine.* (16 variables)**: Engine master switches 1-4, throttle controls 1-4, rotation speeds 1-4, running status 1-4
- **Autopilot.* (18 variables)**: Master, disengage, heading, vertical speed, selected values, system configuration, aileron/elevator controls, throttle engagement

#### **Code Cleanup and Optimization**
- **Removed** ~100+ redundant if-else blocks from fallback code
- **Eliminated** duplicate message handlers for migrated variables
- **Reduced** DLL size from 793KB to 785KB (~8KB code reduction)
- **Added** clear documentation comments for cleaned sections
- **Verified** zero redundancy through comprehensive cleanup validation

#### **Performance Improvements**
- ✅ **O(1) lookup** for 118 most commonly used variables (vs previous O(n))
- ✅ **35% coverage** of total variables now optimized
- ✅ **Backward compatibility** maintained for remaining 221 variables
- ✅ **Zero breaking changes** - all existing code continues to work
- ✅ **Memory efficiency** improved through reduced code duplication

#### **Benefits of v0.3.0**
- 🚀 **Dramatic performance improvement** for message processing
- 🧹 **Cleaner codebase** with eliminated redundancies
- 📈 **Scalable architecture** ready for future variable migrations
- 🔧 **Maintained compatibility** with all existing applications
- 💾 **Smaller DLL size** despite additional functionality

### **Files Updated**
- `aerofly_bridge_dll.cpp` - Major optimization with hash map infrastructure and cleanup
- `CHANGELOG.md` - Updated with comprehensive v0.3.0 changes

### **Migration Notes**
- **No code changes required** - this is a performance optimization only
- **All existing applications** continue to work without modification
- **Performance benefits** are automatic for applications using the optimized variables

## [v0.2.0] - 2025-08-16

### 🚀 Major API Enhancement

#### **BREAKING CHANGE: Simplified JSON Structure**
- **Removed** `all_variables` array from JSON output to eliminate duplication
- **Expanded** variable coverage from 191 to **358 total variables** with canonical Aerofly SDK names
- **Enhanced** JSON structure now provides all variables with descriptive names (e.g., `"Aircraft.Altitude"`, `"Controls.Throttle"`)

#### **Updated BuildDataJSON() Function**
- Complete rewrite to include all 358 variables with canonical naming
- Eliminated redundant `all_variables` array that duplicated data
- Improved performance with cleaner, more efficient JSON payload
- Better developer experience with named variable access instead of numeric indices

#### **Documentation Overhaul**
- Updated all documentation files to reflect new 358-variable structure
- Removed references to deprecated `all_variables` array throughout docs
- Enhanced examples with canonical variable names
- Updated JSON schema to match simplified format
- Improved code examples across all tutorials and API reference

#### **Benefits of v0.2.0**
- ✅ **+167 additional variables** available (191 → 358)
- ✅ **No duplication** - eliminated redundant `all_variables` array
- ✅ **Descriptive names** - `Aircraft.Altitude` vs numeric index access
- ✅ **Better IDE support** with autocompletion for variable names
- ✅ **Reduced errors** from hardcoded array indices
- ✅ **More efficient** JSON payload without duplicate data
- ✅ **Enhanced maintainability** with self-documenting variable names

#### **Migration Guide**
**Before (v0.1.0):**
```javascript
const altitude = data.all_variables[1];  // Index-based access
const throttle = data.all_variables[28]; // Hard to remember indices
```

**After (v0.2.0):**
```javascript
const altitude = data.variables["Aircraft.Altitude"];    // Self-documenting
const throttle = data.variables["Controls.Throttle"];    // Clear and maintainable
```

### **Files Updated**
- `aerofly_bridge_dll.cpp` - Enhanced BuildDataJSON() with 358 variables
- `docs/` - Complete documentation update for new structure
- `reference/json_schema.json` - Updated schema without `all_variables`

## [v0.1.0] - 2025-08-11

### Added
- AeroflyBridge.dll initial release with:
  - Shared Memory interface (fast local telemetry)
  - TCP interface: data stream on 12345, command channel on 12346
  - WebSocket server on 8765 for web/mobile apps
- Documentation in `docs/`:
  - Installation, API reference, variables, architecture, threading, performance
  - Tutorials: Web App (WebSocket) and Python App (Shared Memory + TCP)
- Examples in `examples/`:
  - `aerofly_realtime_monitor.py` (telemetry monitor)
  - `master_control_panel.py` (command/control panel)
- Build scripts: `scripts/compile.bat`, `scripts/make_release.ps1`
- GitHub Actions workflow to build and attach DLL on tag push (`.github/workflows/release.yml`)

[v0.3.1]: https://github.com/jlgabriel/Aerofly-FS4-Bridge/releases/tag/v0.3.1
[v0.3.0]: https://github.com/jlgabriel/Aerofly-FS4-Bridge/releases/tag/v0.3.0
[v0.2.0]: https://github.com/jlgabriel/Aerofly-FS4-Bridge/releases/tag/v0.2.0
[v0.1.0]: https://github.com/jlgabriel/Aerofly-FS4-Bridge/releases/tag/v0.1.0
