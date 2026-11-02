# Complete SDK command routing

This build routes all **197 writable variants** in the bundled message catalog:
the 339-message IPACS SDK sample catalog plus the bridge's aircraft-specific
extensions. It does not make read-only telemetry writable or add controls that
Aerofly does not expose. Availability and behavior depend on the selected aircraft.

## Protocol

Existing numeric requests remain valid. TCP port 12346 takes one JSON object per
connection. WebSocket port 8765 accepts the same object in a text frame.

```json
{"variable":"Controls.Trim","value":-0.25}
```

When a name has several SDK variants, the default is `Value` if available;
otherwise the first writable variant in SDK catalog order is used. Select an
explicit variant with the optional, case-sensitive `flag` field:

```json
{"variable":"Controls.Trim","flag":"Step","value":-1}
{"variable":"Controls.Trim","flag":"Move","value":0.1}
{"variable":"Controls.ParkingBrake","flag":"Toggle","value":1}
```

Only flags actually declared for that control are accepted. The parking brake
above is a **toggle event**, not an absolute brake position: send once per press,
not continuously while held. Wheel brakes remain separate absolute controls.
Clients can also use SDK symbols as aliases (`ControlsTrimStep`, for example).
Previous dotted engine/propeller aliases are retained, subject to SDK access.

All writable catalog types are supported: finite numbers, vectors as arrays of
three/four finite numbers, and the FMS flight-number string (UTF-8 JSON converted
to the SDK's null-terminated UTF-16 payload, at most 31 UTF-16 code units).

```json
{"variable":"View.FreePosition","value":[1,2,3]}
{"variable":"Simulation.SettingOrientation","value":[1,0,0,0]}
{"variable":"FlightManagementSystem.FlightNumber","value":"F2G123"}
```

No universal 0–1 clamp is applied: signed axes, negative steps and radio
frequencies require different ranges. Values retain the SDK control's own units
and semantics. Unknown/read-only names, unsupported flag variants, malformed
JSON, wrong types, nonfinite numbers and oversized strings are discarded.

`reference/command_catalog.json` lists every writable variant, its message ID,
type and flag. Its values are **test fixtures**, not recommended operational
settings or range definitions.

## Validation

Build with CMake and `BUILD_TESTING=ON`, then run:

```powershell
ctest --test-dir build -C Release --output-on-failure
python tests/test_live_commands.py --dll build/Release/AeroflyBridge.dll --catalog build/Release/CommandContractTests.exe
```

Close Aerofly and other bridge hosts before the live DLL test. It loads the real
DLL and tests all 197 variants through TCP and again through WebSocket, checking
the SDK output IDs, types, flags and payloads. It does not simulate an aircraft
or establish that every aircraft responds. The older upstream tests contain
placeholders; the new contract and live DLL tests exercise the production router.

The iPhone client must separately send the newly available commands. Replacing
the DLL alone does not enable controls the client currently omits.
