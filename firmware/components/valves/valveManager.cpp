#include "esp_err.h"

#include "valveManager.h"

static const char* TAG = "ValveManager";

/**
 * @brief Constructor.
 */
ValveManager::ValveManager() {
    openValve_ = VDG_VALVE_OPEN_NONE;
    currentProcess_ = VDG_VALVES_MIN;
}

/**
 * @brief Begin the ValveManager.
 * 
 * @return esp_err_t Return code. 
 */
esp_err_t ValveManager::initialize() {
    openValve_ = VDG_VALVE_OPEN_NONE;
    currentProcess_ = VDG_VALVES_IDLE;
    return ESP_OK;
}

/**
 * @brief Begins a dispensation process.
 * 
 * @param[in] target Target for the process.
 * @param[out] process Output parameter for the current process after this function.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::beginDispenseProcess(VdgDispenseProcessTarget_t target, VdgValveProcess_e &process) {
    return ESP_OK;
}

/**
 * @brief Begins a drain process.
 * 
 * @param[in] target Target for the process.
 * @param[out] process Output parameter for the current process after this function.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::beginDrainProcess(VdgDrainProcessTarget_t &target, VdgValveProcess_e &process){
    return ESP_OK;
}

/**
 * @brief Updates the current process.
 * 
 * @param[out] process Output parameter for the current process after this function.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::updateProcess(VdgValveProcess_e &process){
    return ESP_OK;
}

/**
 * @brief Ends the current process.
 * 
 * @param[out] process Output parameter for the current process after this function.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::endProcess(VdgValveProcess_e &process){
    return ESP_OK;
}