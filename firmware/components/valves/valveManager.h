#ifndef VALVE_MANAGER_H
#define VALVE_MANAGER_H

typedef enum DispensationStage_e {
    NOT_DISPENSING,
    DISPENSING_TANK,
    DISPENSING_SOURCE,
    CONCLUDED
} DispensationStage_e;

typedef struct DispensationState_t {
    /** Target parameters. */
    float targetVolume = 0;
    uint32_t targetTime = 0;
    uint32_t timeout = 0;

    /** Process variables. */
    DispensationStage_e stage = NOT_DISPENSING;
    uint32_t time = 0;
    float volume = 0;
    float flowRate = 0;
    float waterLevel = 0;
    float waterVolume = 0;
} DispensationState_t;

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
     * @param targetVolume Target volume for the process in liters.
     * @param targetTime Target duration for the process in miliseconds. If defined,
     * and targetVolume is defined, the targetTime will apply instead
     * only if the tank flow rate drops while the pressure reads high.
     * @param timeout Maximum duration of the process in miliseconds.
     * @param stage Overwritten with the initial stage of the dispensation process.
     * @return esp_err_t Return code.
     */
    esp_err_t beginDispenstation(float targetVolume, uint32_t targetTime, uint32_t timeout, DispensationStage_e &stage);

    /**
     * @brief Updates the dispensation process.
     * 
     * @param stage Overwritten with the current stage.
     * @return esp_err_t Return code.
     */
    esp_err_t loopDispensation(DispensationStage_e &stage);

private:
    DispensationState_t dispensationState;
};

#endif