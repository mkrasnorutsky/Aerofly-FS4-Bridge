#pragma once
// Compatibility aliases from the original command router. Resolve through SDK access checks.
inline std::string CommandAlias(std::string name) {
    if(name=="Aircraft.Engine.Running.1") return "AircraftEngineRunning1";
    if(name=="Aircraft.Engine.Running.2") return "AircraftEngineRunning2";
    if(name=="Aircraft.Engine.Running.3") return "AircraftEngineRunning3";
    if(name=="Aircraft.Engine.Running.4") return "AircraftEngineRunning4";
    if(name=="Controls.PropellerSpeed.1") return "ControlsPropellerSpeed1";
    if(name=="Controls.PropellerSpeed.2") return "ControlsPropellerSpeed2";
    if(name=="Controls.PropellerSpeed.3") return "ControlsPropellerSpeed3";
    if(name=="Controls.PropellerSpeed.4") return "ControlsPropellerSpeed4";
    if(name=="Aircraft.EngineMaster.1") return "AircraftEngineMaster1";
    if(name=="Aircraft.EngineMaster.2") return "AircraftEngineMaster2";
    if(name=="Aircraft.EngineMaster.3") return "AircraftEngineMaster3";
    if(name=="Aircraft.EngineMaster.4") return "AircraftEngineMaster4";
    if(name=="Aircraft.Engine.Throttle.1") return "AircraftEngineThrottle1";
    if(name=="Aircraft.Engine.Throttle.2") return "AircraftEngineThrottle2";
    if(name=="Aircraft.Engine.Throttle.3") return "AircraftEngineThrottle3";
    if(name=="Aircraft.Engine.Throttle.4") return "AircraftEngineThrottle4";
    if(name=="Aircraft.Engine.RotationSpeed.1") return "AircraftEngineRotationSpeed1";
    if(name=="Aircraft.Engine.RotationSpeed.2") return "AircraftEngineRotationSpeed2";
    if(name=="Aircraft.Engine.RotationSpeed.3") return "AircraftEngineRotationSpeed3";
    if(name=="Aircraft.Engine.RotationSpeed.4") return "AircraftEngineRotationSpeed4";
    if(name=="Aircraft.MasterWarning") return "WarningsMasterWarning";
    if(name=="Aircraft.MasterCaution") return "WarningsMasterCaution";
    if(name=="Aircraft.LowOilPressure") return "WarningsLowOilPressure";
    if(name=="Aircraft.LowFuelPressure") return "WarningsLowFuelPressure";
    return name;
}
