/**
 * @file config.h
 * @brief All persistent and mutable configuration values.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "consts.h"

enum TankShapes_e {
    TANK_RECTANGLE,
    TANK_CYLINDER
};

enum Valves_e {
    SOURCE_DISPENSE,
    TANK_DISPENSE,
    TANK_DRAIN
};

enum Relays_e {
    RELAYS_MIN,
    RELAY1,
    RELAY2,
    RELAY3,
    RELAYS_MAX
};

struct SystemConfig_t {
    uint32_t sleepIntervalMs;
    uint32_t processSliceUploadIntervalMs;
};

struct ValveConfig_t {
    bool sourceDispenseEnabled;
    bool tankDispenseEnabled;
    bool tankDrainEnabled;
    Relays_e sourceDispenseRelay;
    Relays_e tankDispenseRelay;
    Relays_e tankDrainRelay;
    Valves_e preferredDispenseValve;
};

struct DispenseConfig_t {
    float dataResolutionLiters;
};

struct SourceConfig_t {
    bool enabled;
    float staticFlowRate;
};

struct TankConfig_t {
    bool enabled;
    TankShapes_e shape;
    float dimension1;
    float dimension2;
    float dimension3;
    uint16_t tankTimeout;
};

struct FlowSensorConfig_t {
    bool enabled;
    float defaultPulsesPerLiter;
    float minFlowRate;
    float calibrationTimeout;
    float calibrateMaxVolume;
};

struct PressureSensorConfig_t {
    bool enabled;
    float reportMode;
};

struct PressureSensorCalibrationPoint_t {
    uint16_t analogVoltage;
    uint16_t volume;
};


struct Config_t {
    SystemConfig_t system;
    ValveConfig_t valves;
    DispenseConfig_t dispense;
    SourceConfig_t source;
    TankConfig_t tank;
    FlowSensorConfig_t flowSensor;
    PressureSensorConfig_t pressureSensor;
    PressureSensorCalibrationPoint_t pressureCalibrationTable[MAX_PRESSURE_CALIBRATION_POINTS];
};



#endif
