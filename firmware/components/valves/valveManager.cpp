#include "esp_err.h"

#include "valveManager.h"

static const char* TAG = "ValveManager";

/**
 * @brief Constructor.
 */
ValveManager::ValveManager() {
    dispensationState = {};
}

/**
 * @brief Begin the ValveManager.
 * 
 * @return esp_err_t Return code. 
 */
esp_err_t ValveManager::initialize() {
    return ESP_OK;
}

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
esp_err_t ValveManager::beginDispenstation(float targetVolume, uint32_t targetTime, uint32_t timeout, DispensationStage_e &stage) {
    return ESP_OK;
}

/**
 * @brief Updates the dispensation process.
 * 
 * @param stage Overwritten with the current stage.
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::loopDispensation(DispensationStage_e &stage) {
    return ESP_OK;
}