/**
 * @file config.h
 * @brief All persistent and mutable configuration values.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <optional>

#include "consts.h"
#include "enums.h"

/**
 * @brief General system configuration.
 */
struct DripSystemConfig_t {
    /** 
     * @brief The duration to wait in between checking for 
     * new messages in the idle state in miliseconds. 
     * Set to zero for no delay. 
     * */
    std::optional<uint32_t> sleepIntervalMs = 1000U;
    /** 
     * @brief The interval in miliseconds between uploads of process slice data messages 
     * In the dispense and drain processes.
     */
    std::optional<uint32_t> processSliceUploadIntervalMs = 100U;
};

/**
 * @brief Valve configuration.
 */
struct DripValveConfig_t {
    /** @brief The relay of the source dispense valve. Set to RELAY_NONE when unconfigured. */
    std::optional<Relays_e> sourceDispenseRelay = RELAY1;
    /** @brief The relay of the tank dispense valve. Set to RELAY_NONE when unconfigured. */
    std::optional<Relays_e> tankDispenseRelay = RELAY2;
    /** @brief The relay of the tank drain valve. Set to RELAY_NONE when unconfigured. */
    std::optional<Relays_e> tankDrainRelay = RELAY3;
    /** 
     * @brief Sets a preference between SOURCE_DISPENSE and TANK_DISPENSE. 
     * If TANK_DISPENSE, the tank will always be used first in dispensation.
     * If SOURCE_DISPENSE, the source will always be used first in dispensation.
     */
    std::optional<Valves_e> preferredDispenseValve = TANK_DISPENSE;
};

/**
 * @brief Source configuration.
 * The water source is assumed to be constant-pressure
 * and inexhaustible, like a home water supply.
 */
struct DripSourceConfig_t {
    /** 
     * @brief If true, a source is being used. 
     * The source dispense relay in the Valve Config must be set.
     */
    std::optional<bool> enabled = true;
    /**
     * @brief The flow rate of the source in liters per minute.
     * Only used if a flow sensor is not enabled.
     */
    std::optional<float> staticFlowRateLpm = 0.0f;
};

/**
 * @brief Tank configuration.
 * The water tank is assumed to be variable-pressure
 * and exhaustible, like a rain barrel.
 */
struct DripTankConfig_t {
    /**
     * @brief If true, a tank is being used.
     * The tank dispense valve in the Valve Config must be set.
     */
    std::optional<bool> enabled = true;
    /**
     * @brief The shape of the tank, used for pressure-based volume estimation.
     * Controls the interpretation of the three dimensions.
     */
    std::optional<TankShapes_e> shape = TANK_CYLINDER;
    /**
     * @brief First dimension of the tank, in meters.
     * TANK_CYLINDER: Radius
     * TANK_RECTANGLE: Length
     */
    std::optional<float> dimension1 = 0.0f;
    /**
     * @brief Second dimension of the tank, in meters.
     * TANK_CYLINDER: Height
     * TANK_RECTANGLE: Width
     */
    std::optional<float> dimension2 = 0.0f;
    /**
     * @brief Third dimension of the tank, in meters.
     * TANK_CYLINDER: N/A
     * TANK_RECTANGLE: Height
     */
    std::optional<float> dimension3 = 0.0f;
    /**
     * @brief Optional override for tank volume.
     */
    std::optional<float> volume = 0.0f;
    /**
     * @brief TODO
     */
    std::optional<uint16_t> tankTimeoutMs;
};

/**
 * @brief Flow sensor configuration.
 */
struct DripFlowSensorConfig_t {
    /** @brief If true, a flow sensor is connected. */
    std::optional<bool> enabled = true;
    /** @brief The number of pulses per liter to use before calibration. */
    std::optional<float> defaultPulsesPerLiter = 0U;
    /** @brief The number of pulses per liter to use after calibration. */
    std::optional<float> calibratedPulsesPerLiter = 0U;
    /** @brief The lowest flow measurement from the sensor to trust in liters per second. */
    std::optional<float> minFlowRateLps = 0.0f;
    /** @brief The number of miliseconds to wait in the calibration process before a measurement is received. */
    std::optional<float> calibrationTimeoutMs = 120000U;
    /** @brief The maximum amount of volume to allow per calibration step in liters. */
    std::optional<float> calibrateMaxVolume = 6.0f;
};

/**
 * @brief An entry in the pressure sensor calibration table.
 */
struct DripPressureSensorCalibrationPoint_t {
    /** @brief The analog voltage measured by the ADC. */
    std::optional<uint16_t> analogVoltage;
    /** @brief The volume present in the tank. */
    std::optional<uint16_t> volume;
};

/**
 * @brief Pressure sensor configuration.
 */
struct DripPressureSensorConfig_t {
    /** @brief If true, a pressure sensor is connected. */
    std::optional<bool> enabled = true;
    /** 
     * @brief If true, the device will report the pressure reading on a fixed interval 
     * in addition to in response to a query.
     */
    std::optional<bool> broadcastEnabled = true;
    /** @brief If broadcastEnabled, the interval in miliseconds to report the pressure. */
    std::optional<float> broadcastIntervalMs = 30000U;
    /** @brief The pressure calibration table. */
    std::optional<DripPressureSensorCalibrationPoint_t> pressureCalibrationTable[MAX_PRESSURE_CALIBRATION_POINTS];
};

/** @brief Device configuration. */
struct DripConfig_t {
    DripSystemConfig_t system;
    DripValveConfig_t valves;
    DripSourceConfig_t source;
    DripTankConfig_t tank;
    DripFlowSensorConfig_t flowSensor;
    DripPressureSensorConfig_t pressureSensor;
};

#endif
