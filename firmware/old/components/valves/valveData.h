#ifndef VALVE_DATA_H
#define VALVE_DATA_H

#include "enums.h"

enum class DripValveManagerStateId_e {
    Idle,
    DispenseStart,
    DispenseSource,
    DispenseTank,
    DispenseExit,
    DrainTank,
    Deactivate
};

enum class DripValveManagerEventId_e {
    DispenseStart,
    DrainStart,
    Deactivate
};

/**
 * @brief Describes the current process of the manager.
 */
enum VdgValveProcess_e {
    DRIP_VALVES_MIN,

    /** No process is ongoing. */
    DRIP_VALVES_IDLE,
    /** The dispense process is ongoing. */
    DRIP_VALVES_DISPENSE,
    /** The drain process is ongoing. */
    DRIP_VALVES_DRAIN,

    DRIP_VALVES_MAX
};

/**
 * Dispense process definitions.
 */


/**
 * @brief Describes a type of target for a dispense process.
 */
enum class DripDispenseProcessTargetType_e {
    /** The goal of the process is a volume output. */
    Liters,
    /** The goal of the process is a duration. */
    Seconds
};

/**
 * @brief Describes the target of a current dispensation process.
 */
struct VdgDispenseProcessTarget_t {
    /** The type of target. */
    DripDispenseProcessTargetType_e targetType;
    /** 
     * The target output volume in liters, 
     * or the target duration in seconds, 
     * depending on the value of targetType. 
     */
    float target;
    /** Maximum duration of the process in miliseconds. */
    uint32_t timeoutMs;
};

/**
 * @brief Describes the realtime variables of a dispensation process;
 */
struct VdgDispenseProcessSlice_t {
    /** Currently open valve. */
    DripValves_e openValve;
    /** Current time in miliseconds. */
    uint32_t timeMs;
    /** Current output volume in liters. */
    float outputVolume;
    /** Current flow rate in liters per second. */
    float flowRate;
    /** Current tank level in meters. */
    float tankLevel;
    /** Current tank volume in meters. */
    float tankVolume;
};

/**
 * @brief Describes a summary of the process variables for a whole dispensation process.
 */
struct VdgDispenseProcessSummary_t {
    /** Total time of the process in milseconds. */
    uint32_t durationMs;
    /** Total volume in liters. */
    float outputVolume;
    /** Total volume output from the tank. */
    float outputTankVolume;
    /** The initial and final tank level in meters. */
    float initialTankLevel;
    float finalTankLevel;
    /** The inital and final tank volume in liters. */
    float initialTankVolume;
    float finalTankVolume;
};

/** 
 * @brief Holds all dispense process related data. 
 */
struct VdgDispenseProcessData_t {
    VdgDispenseProcessTarget_t target;
    VdgDispenseProcessSlice_t slice;
    VdgDispenseProcessSummary_t summary;
};

/**
 * Drain process definitions.
 */

/**
 * @brief Describes a type of target fo a drain process.
 */
enum VdgDrainProcessTargetType_e {
    /** The goal of the process is a tank level. */
    DRIP_DRAIN_PROCESS_TARGET_LEVEL,
    /** The goal of the process is a duration. */
    DRIP_DRAIN_PROCESS_TARGET_TIME
};

/** 
 * @brief Describes the target of a drain process.
 */
struct VdgDrainProcessTarget_t {
    /** The type of target. */
    VdgDrainProcessTargetType_e targetType;
    /**
     * The target level in meters,
     * or the target duration in seconds,
     * depending on the value of targetType.
     */
    float target = 0.0f;
    /** Maximum duration of the process in miliseconds. */
    uint32_t timeout = 0U;
};

/** 
 * @brief Describes the realtime variables of a dispensation process.
 */
struct VdgDrainProcessSlice_t {
    /** Current time in miliseconds. */
    uint32_t timeMs = 0U;
    /** Current tank level in meters. */
    float tankLevel = 0.0f;
    /** Current tank volume in meters. */
    float tankVolume = 0.0f;
};

/**
 * @brief Describes a summary of the process variables for a whole drain process.
 */
struct VdgDrainProcessSummary_t {
    /** Total time of the process in milseconds. */
    uint32_t durationMs = 0U;
    /** The initial and final tank level in meters. */
    float initialTankLevel = 0.0f;
    float finalTankLevel = 0.0f;
    /** The inital and final tank volume in liters. */
    float initialTankVolume = 0.0f;
    float finalTankVolume = 0.0f;
};

/** 
 * @brief Holds all drain process related data. 
 */
struct VdgDrainProcessData_t {
    VdgDrainProcessTarget_t target;
    VdgDrainProcessSlice_t slice;
    VdgDrainProcessSummary_t summary;
};

#endif