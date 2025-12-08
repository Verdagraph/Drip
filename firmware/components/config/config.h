/**
 * @file config.h
 * @brief All persistent and mutable configuration values.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "etl/optional.h"

#include "consts.h"
#include "enums.h"

/** @brief Defines an identity type for defining both optional and non-optional members without code duplication. */
template <typename T>
using Identity = T;

/**
 * @brief General system configuration.
 */
template <template <typename> typename TWrap> 
struct DripSystemConfig_t {
    /** 
     * @brief The duration to wait in between checking for 
     * new messages in the idle state in miliseconds. 
     * Set to zero for no delay. 
     * */
    TWrap<uint32_t> sleepIntervalMs;
    /** 
     * @brief The interval in miliseconds between uploads of process slice data messages 
     * In the dispense and drain processes.
     */
    TWrap<uint32_t> processSliceUploadIntervalMs;
};

/**
 * @brief Valve configuration.
 */
template <template <typename> typename TWrap> 
struct DripValveConfig_t {
    /** @brief The relay of the source dispense valve. Set to RELAY_NONE when unconfigured. */
    TWrap<DripRelays_e> sourceDispenseRelay;
    /** @brief The relay of the tank dispense valve. Set to RELAY_NONE when unconfigured. */
    TWrap<DripRelays_e> tankDispenseRelay;
    /** @brief The relay of the tank drain valve. Set to RELAY_NONE when unconfigured. */
    TWrap<DripRelays_e> tankDrainRelay;
    /** 
     * @brief Sets a preference between SOURCE_DISPENSE and TANK_DISPENSE. 
     * If TANK_DISPENSE, the tank will always be used first in dispensation.
     * If SOURCE_DISPENSE, the source will always be used first in dispensation.
     */
    TWrap<DripValves_e> preferredDispenseValve;
};

/**
 * @brief Source configuration.
 * The water source is assumed to be constant-pressure
 * and inexhaustible, like a home water supply.
 */
template <template <typename> typename TWrap> 
struct DripSourceConfig_t {
    /** 
     * @brief If true, a source is being used. 
     * The source dispense relay in the Valve Config must be set.
     */
    TWrap<bool> enabled;
    /**
     * @brief The flow rate of the source in liters per minute.
     * Only used if a flow sensor is not enabled.
     */
    TWrap<float> staticFlowRateLpm;
};

/**
 * @brief Tank configuration.
 * The water tank is assumed to be variable-pressure
 * and exhaustible, like a rain barrel.
 */
template <template <typename> typename TWrap> 
struct DripTankConfig_t {
    /**
     * @brief If true, a tank is being used.
     * The tank dispense valve in the Valve Config must be set.
     */
    TWrap<bool> enabled;
    /**
     * @brief The shape of the tank, used for pressure-based volume estimation.
     * Controls the interpretation of the three dimensions.
     */
    TWrap<TankShapes_e> shape;
    /**
     * @brief First dimension of the tank, in meters.
     * TANK_CYLINDER: Radius
     * TANK_RECTANGLE: Length
     */
    TWrap<float> dimension1;
    /**
     * @brief Second dimension of the tank, in meters.
     * TANK_CYLINDER: Height
     * TANK_RECTANGLE: Width
     */
    TWrap<float> dimension2;
    /**
     * @brief Third dimension of the tank, in meters.
     * TANK_CYLINDER: N/A
     * TANK_RECTANGLE: Height
     */
    TWrap<float> dimension3;
    /**
     * @brief Optional override for tank volume.
     */
    TWrap<float> volume;
    /**
     * @brief TODO
     */
    TWrap<uint16_t> tankTimeoutMs;
};

/**
 * @brief Flow sensor configuration.
 */
template <template <typename> typename TWrap> 
struct DripFlowSensorConfig_t {
    /** @brief If true, a flow sensor is connected. */
    TWrap<bool> enabled;
    /** @brief The number of pulses per liter to use before calibration. */
    TWrap<float> defaultPulsesPerLiter;
    /** @brief The number of pulses per liter to use after calibration. */
    TWrap<float> calibratedPulsesPerLiter;
    /** @brief The lowest flow measurement from the sensor to trust in liters per second. */
    TWrap<float> minFlowRateLps;
    /** @brief The number of miliseconds to wait in the calibration process before a measurement is received. */
    TWrap<float> calibrationTimeoutMs;
    /** @brief The maximum amount of volume to allow per calibration step in liters. */
    TWrap<float> calibrateMaxVolume;
};

/**
 * @brief An entry in the pressure sensor calibration table.
 */
template <template <typename> typename TWrap> 
struct DripPressureSensorCalibrationPoint_t {
    /** @brief The analog voltage measured by the ADC. */
    uint16_t analogVoltage;
    /** @brief The volume present in the tank. */
    uint16_t volume;
};

/**
 * @brief Pressure sensor configuration.
 */
template <template <typename> typename TWrap> 
struct DripPressureSensorConfig_t {
    /** @brief If true, a pressure sensor is connected. */
    TWrap<bool> enabled;
    /** 
     * @brief If true, the device will report the pressure reading on a fixed interval 
     * in addition to in response to a query.
     */
    TWrap<bool> broadcastEnabled;
    /** @brief If broadcastEnabled, the interval in miliseconds to report the pressure. */
    TWrap<float> broadcastIntervalMs;
    /** @brief The pressure calibration table. */
    std::optional<DripPressureSensorCalibrationPoint_t> pressureCalibrationTable[MAX_PRESSURE_CALIBRATION_POINTS];
};

/** @brief Device configuration. */
template <template <typename> typename TWrap> 
struct DripConfigBase_t {
    DripSystemConfig_t<TWrap> system;
    DripValveConfig_t<TWrap> valves;
    DripSourceConfig_t<TWrap> source;
    DripTankConfig_t<TWrap> tank;
    DripFlowSensorConfig_t<TWrap> flowSensor;
    DripPressureSensorConfig_t<TWrap> pressureSensor;
};

/** 
 * @brief Define a base config with all attributes guarnteed to be defined,
 * and a version with all attributes as optional, for the purposes of updating.
 */
using DripConfig_t = DripConfigBase_t<Identity>;
using DripConfigUpdate_t = DripConfigBase_t<etl::optional>;

#endif
