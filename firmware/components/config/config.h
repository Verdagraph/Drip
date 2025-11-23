/**
 * @file config.h
 * @brief All persistent and mutable configuration values.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "consts.h"
#include "enums.h"

/**
 * @brief General system configuration.
 */
struct SystemConfig_t {
    /** 
     * @brief The duration to wait in between checking for 
     * new messages in the idle state in miliseconds. 
     * Set to zero for no delay. 
     * */
    uint32_t sleepIntervalMs = 1000U;
    /** 
     * @brief The interval in miliseconds between uploads of process slice data messages 
     * In the dispense and drain processes.
     */
    uint32_t processSliceUploadIntervalMs = 100U;
};

/**
 * @brief Valve configuration.
 */
struct ValveConfig_t {
    /** @brief If true, the source dispense valve is being used. */
    bool sourceDispenseEnabled = true;
    /** @brief If true, the tank dispense valve is being used. */
    bool tankDispenseEnabled = true;
    /** @brief If true, the tank drain valve is being used. */
    bool tankDrainEnabled = true;
    /** @brief The relay of the source dispense valve. Set to RELAY_NONE when unconfigured. */
    Relays_e sourceDispenseRelay = RELAY1;
    /** @brief The relay of the tank dispense valve. Set to RELAY_NONE when unconfigured. */
    Relays_e tankDispenseRelay = RELAY2;
    /** @brief The relay of the tank drain valve. Set to RELAY_NONE when unconfigured. */
    Relays_e tankDrainRelay = RELAY3;
    /** 
     * @brief Sets a preference between SOURCE_DISPENSE and TANK_DISPENSE. 
     * If TANK_DISPENSE, the tank will always be used first in dispensation.
     * If SOURCE_DISPENSE, the source will always be used first in dispensation.
     */
    Valves_e preferredDispenseValve = TANK_DISPENSE;
};

/** */
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
    PressureSensorCalibrationPoint_t pressureCalibrationTable[MAX_PRESSURE_CALIBRATION_POINTS];
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
};



#endif
