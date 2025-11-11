#ifndef VALVE_MANAGER_H
#define VALVE_MANAGER_H

#include "config.h"
#include "configManager.h"
#include "gpioDriver.h"
#include "dataContainer.h"

#define VDG_DRAIN_PROCESS_MAX_TIMEOUT_MIN 120

/**
 * @brief Describes the current process of the manager.
 */
enum VdgValveProcess_e {
    VDG_VALVES_MIN,

    /** No process is ongoing. */
    VDG_VALVES_IDLE,
    /** The dispense process is ongoing. */
    VDG_VALVES_DISPENSE,
    /** The drain process is ongoing. */
    VDG_VALVES_DRAIN,

    VDG_VALVES_MAX
};

/**
 * @brief Describes which valve is currently open.
 */
enum VdgOpenValveState_e {
    VDG_VALVE_OPEN_NONE,
    VDG_VALVE_OPEN_TANK_DISPENSE,
    VDG_VALVE_OPEN_TANK_DRAIN,
    VDG_VALVE_OPEN_SOURCE_DISPENSE
};

/**
 * Dispense process definitions.
 */

/**
 * @brief Describes possible finite-state-machine states for the dispense process.
 */
enum VdgDispenseFsmState_e {
    VDG_DISPENSE_FSM_MIN,

    /** Initialization. */
    VDG_DISPENSE_FSM_INIT,
    /** The tank is currently dispensing. */
    VDG_DISPENSE_FSM_TANK_DISPENSE,
    /** The source is currently dispensing. */
    VDG_DISPENSE_FSM_SOURCE_DISPENSE,
    /** Dispensing is stopped and the summary is available. */
    VDG_DISPENSE_FSM_STOP,

    VDG_DISPENSE_FSM_MAX,
};

/**
 * @brief Describes a type of target for a dispense process.
 */
enum VdgDispenseProcessTargetType_e {
    /** The goal of the process is a volume output. */
    VDG_DISPENSE_PROCESS_TARGET_VOLUME,
    /** The goal of the process is a duration. */
    VDG_DISPENSE_PROCESS_TARGET_TIME
};

/**
 * @brief Describes the target of a current dispensation process.
 */
struct VdgDispenseProcessTarget_t {
    /** The type of target. */
    VdgDispenseProcessTargetType_e targetType;
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
    /** Time timestamp in miliseconds that the tank yielded. */
    uint32_t tankSwitchoverTime;
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
    VdgDispenseFsmState_e state;
    VdgDispenseProcessTarget_t target;
    VdgDispenseProcessSlice_t slice;
    VdgDispenseProcessSummary_t summary;
};

/**
 * Drain process definitions.
 */

/**
 * @brief Describes possible finite-state-machine states for the drain process.
 */
enum VdgDrainFsmState_e {
    VDG_DRAIN_FSM_MIN,
    
    /** Initialization. */
    VDG_DRAIN_FSM_INIT,
    /** The tank is currently being drained. */
    VDG_DRAIN_FSM_TANK_DRAIN,
    /** Draining is stopped and the summary is available. */
    VDG_DRAIN_FSM_STOP,
    
    VDG_DRAIN_FSM_MAX,
};

/**
 * @brief Describes a type of target fo a drain process.
 */
enum VdgDrainProcessTargetType_e {
    /** The goal of the process is a tank level. */
    VDG_DRAIN_PROCESS_TARGET_LEVEL,
    /** The goal of the process is a duration. */
    VDG_DRAIN_PROCESS_TARGET_TIME
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
    VdgDrainFsmState_e state;
    VdgDrainProcessTarget_t target;
    VdgDrainProcessSlice_t slice;
    VdgDrainProcessSummary_t summary;
};

/**
 * @brief Handles the dispensation and draining process.
 */
class ValveManager {
public:
    /**
     * @brief Constructor.
     */
    ValveManager(ConfigManager &configManager);

    /**
     * @brief Begin the ValveManager.
     * 
     * @return esp_err_t Return code. 
     */
    esp_err_t initialize();

    /**
     * @brief Begins a dispensation process.
     * 
     * @param[in] target Target for the process.
     * @param[out] process Output parameter for the current process after this function.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t beginDispenseProcess(VdgDispenseProcessTarget_t target, VdgValveProcess_e &process);
    
    /**
     * @brief Begins a drain process.
     * 
     * @param[in] target Target for the process.
     * @param[out] process Output parameter for the current process after this function.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t beginDrainProcess(VdgDrainProcessTarget_t &target, VdgValveProcess_e &process);
    
    /**
     * @brief Updates the current process.
     * 
     * @param[out] process Output parameter for the current process after this function.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t updateProcess(VdgValveProcess_e &process);
    
    /**
     * @brief Ends the current process.
     * 
     * @param[out] process Output parameter for the current process after this function.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t endProcess(VdgValveProcess_e &process);

    esp_err_t getCurrentProcess(VdgValveProcess_e &process);

private:
    ConfigManager configManager;
    DataContainer dataContainer;

    VdgOpenValveState_e openValve_;
    VdgValveProcess_e currentProcess_;

    esp_err_t setValveState(Valves_e valve, VdgGpioToggleState_e state);

    /**
     * @brief Closes the currently open valve.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t closeValves();

    /**
     * @brief Open the tank dispense valve.
     * 
     * @return esp_err_t Return code. 
     */
    esp_err_t openTankDispenseValve();

    /**
     * @brief Open the tank drain valve.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t openTankDrainValve();

    /**
     * @brief Open the source dispense valve.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t openSourceDispenseValve();
};

#endif