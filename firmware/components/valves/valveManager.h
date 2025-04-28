#ifndef VALVE_MANAGER_H
#define VALVE_MANAGER_H

/**
 * @brief Describes the possible states of the valves.
 */
typedef enum ValveStates_e {
    /** Default state. */
    VALVES_UNKNOWN,
    /** No valves are open. */
    VALVES_IDLE,
    /** The tank valve is open for dispensing. */
    VALVES_TANK_DISPENSE,
    /** The source valve is open for dispensing. */
    VALVES_SOURCE_DISPENSE,
    /** The tank drain valve is open for draining. */
    VALVES_TANK_DRAIN
} ValveStates_e;

/**
 * @brief Describes the target of a current dispensation process.
 * 
 */
typedef struct DispenseTarget_t {
    /** Target volume for the process in liters. */
    float targetVolume = 0;
    /**
     * Target duration for the process in miliseconds. If defined,
     * and targetVolume is defined, the targetTime will apply instead
     * only if the tank flow rate drops while the pressure reads high.
     */
    uint32_t targetTime = 0;
    /** Maximum duration of the process in miliseconds. */
    uint32_t timeout = 0;
} DispenseTarget_t;

/**
 * @brief Describes the realtime variables of a dispensation process;
 */
typedef struct DispenseProcess_t {
    uint32_t time = 0;
    float outputVolume = 0;
    float flowRate = 0;
    float tankLevel = 0;
} DispenseProcess_t;

/**
 * @brief Describes a summary of the process variables for a whole dispensation process.
 */
typedef struct DispenseSummary_t {
    uint32_t duration = 0;
    float outputVolume = 0;
    float outputTankVolume = 0;
    uint32_t tankSwitchoverTime = 0;
    float initialTankLevel = 0;
    float finalTankLevel = 0;
} DispenseSummary_t;

/** 
 * @brief Describes the target of a drain process.
 */
typedef struct DrainTarget_t {
    uint32_t targetTime = 0;
    uint32_t timeout = 0;
} DrainTarget_t;

/** 
 * @brief Describes the realtime variables of a dispensation process.
 */
typedef struct DrainProcess_t {
    uint32_t time = 0;
    float tankLevel = 0;
} DrainProcess_t;

/**
 * @brief Describes a summary of the process variables for a whole drain process.
 */
typedef struct DrainSummary_t {
    uint32_t duration = 0;
    float initialTankLevel = 0;
    float finalTankLevel = 0;
} DrainSummary_t;

/**
 * @brief Handles the dispensation and draining process.
 */
class ValveManager {
public:
    /**
     * @brief Constructor.
     */
    ValveManager();

    /**
     * @brief Begin the ValveManager.
     * 
     * @return esp_err_t Return code. 
     */
    esp_err_t initialize();

    /**
     * @brief Begins a dispensation process.
     * 
     * @param target Target for the process.
     * @param state Overwritten with the initial state of the dispensation process.
     * @param process Overwritten with the initial process variables.
     * @return esp_err_t Return code.
     */
    esp_err_t beginDispenstation(DispenseTarget_t &target, ValveStates_e &state, DispenseProcess_t &process);

    /**
     * @brief Updates the dispense process.
     * 
     * @param state Overwritten with the current state.
     * @param process Overwritten with the current process variables.
     * @param summary Overwritten with the current process summary.
     * @return esp_err_t Return code.
     */
    esp_err_t loopDispense(ValveStates_e &state, DispenseProcess_t &process, DispenseSummary_t &summary);

    /**
     * @brief Ends the dispense process.
     * 
     * @param state Overwritten with the state.
     * @param process Overwritten with the current process variables.
     * @param summary Overwritten with the current process summary.
     * @return esp_err_t Return code.
     */
    esp_err_t endDispense(ValveStates_e &state, DispenseProcess_t &process, DispenseSummary_t &summary);

    /**
     * @brief Begins a drain process.
     * 
     * @param target Target for the process.
     * @param state Overwritten with the initial state of the drain process.
     * @param process Overwritten with the initial process variables.
     * @return esp_err_t Return code.
     */
    esp_err_t beginDrain(DrainTarget_t &target, ValveStates_e &state, DrainProcess_t &process);

    /**
     * @brief Updates the drain process.
     * 
     * @param state Overwritten with the current state.
     * @param process Overwritten with the current process variables.
     * @param summary Overwritten with the current process summary.
     * @return esp_err_t Return code.
     */
    esp_err_t loopDrain(ValveStates_e &state, DrainProcess_t &process, DrainSummary_t &summary);

    /**
     * @brief Ends the drain process.
     * 
     * @param state Overwritten with the state.
     * @param process Overwritten with the current process variables.
     * @param summary Overwritten with the current process summary.
     * @return esp_err_t Return code.
     */
    esp_err_t endDrain(ValveStates_e &state, DrainProcess_t &process, DrainSummary_t &summary);


private:
    ValveStates_e state;
    DispenseTarget_t dispenseTarget;
    DispenseProcess_t dispenseProcess;
    DispenseSummary_t dispenseSummary;
    DrainTarget_t drainTarget;
    DrainProcess_t drainProcess;
    DrainSummary_t drainSummary;
};

#endif