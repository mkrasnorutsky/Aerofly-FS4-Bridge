# Aerofly Bridge 0.4.1 — Complete SDK command routing

Unsigned Windows x64 build. Download **AeroflyBridge-0.4.1-x64.zip** for the DLL,
installation instructions, command reference and third-party notices.

## Changes

- Route all 197 writable SDK message variants, including elevator trim, parking
  brake, engine controls, aircraft-specific controls, view and simulation controls.
- Preserve SDK Value, Step, Move, Toggle and Event semantics. Use the optional
  `flag` field to select variants with the same control name.
- Support numeric, vector and flight-number string commands.
- Reject malformed JSON, invalid payloads, unsupported variants and read-only
  telemetry writes. Preserve existing dotted propeller/engine aliases.
- Replace incomplete duplicated routing with a shared SDK message catalog.
- Repair upstream build and test configuration issues.

## Validation

- Release x64 build succeeded; Authenticode status: **NotSigned**.
- All 35 CTest cases passed, including exhaustive production-router coverage.
- Real DLL test passed all 197 writable variants over TCP and all 197 again over
  WebSocket, verifying SDK message IDs, types, flags and payloads.
- Actual aircraft response has **not** been verified in Aerofly. Aircraft-specific
  controls depend on the selected aircraft. Read-only telemetry remains read-only.

## Installation and client compatibility

Close Aerofly and other bridge hosts, back up the existing DLL, then copy
`AeroflyBridge.dll` into your actual `Documents/Aerofly FS 4/external_dll` folder.
Documents may be redirected to OneDrive. Keep the filename unchanged.

Start Aerofly and load a flight. Microsoft Visual C++ x64 runtime is required.
Only one bridge host may run at a time.

Existing numeric command requests remain supported for writable controls. Clients
must send newly supported commands to use them; replacing the DLL does not update
the iPhone app. Parking brake is a toggle event, distinct from wheel braking.

See [command documentation](docs/command_support.md) and the included catalog for
details. This community bridge cannot add controls Aerofly's SDK does not expose.
