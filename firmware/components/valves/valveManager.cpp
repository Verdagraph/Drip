#include "esp_err.h"

#include "valveManager.h"

static const char* TAG = "ValveManager";

/**
 * @brief Constructor.
 */
ValveManager::ValveManager() {
    currentProcess_ = VDG_VALVES_MIN;
    dispenseTarget = {};
    dispenseProcess = {};
    dispenseSummary = {};
    drainTarget = {};
    drainProcess = {};
    drainSummary = {};
}

/**
 * @brief Begin the ValveManager.
 * 
 * @return esp_err_t Return code. 
 */
esp_err_t ValveManager::initialize() {
    currentProcess_ = VDG_VALVES_IDLE;
    return ESP_OK;
}

/**
 * @brief Begins a dispensation process.
 * 
 * @param target Target for the process.
 * @param state Overwritten with the initial state of the dispensation process.
 * @param process Overwritten with the initial process variables.
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::beginDispenseProcess(DispenseTarget_t &target, ValveStates_e &state, DispenseProcess_t &process) {
    return ESP_OK;
}

/**
 * @brief Updates the dispensation process.
 * 
 * @param state Overwritten with the current state.
 * @param process Overwritten with the current process variables.
 * @param summary Overwritten with the current process summary.
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::updateDispenseProcess(ValveStates_e &state, DispenseProcess_t &process, DispenseSummary_t &summary) {
    return ESP_OK;
}

/**
 * @brief Ends the dispensation process.
 * 
 * @param state Overwritten with the state.
 * @param process Overwritten with the current process variables.
 * @param summary Overwritten with the current process summary.
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::endDispenseProcess(ValveStates_e &state, DispenseProcess_t &process, DispenseSummary_t &summary) {
    return ESP_OK;
}

/**
 * @brief Begins a drain process.
 * 
 * @param target Target for the process.
 * @param state Overwritten with the initial state of the drain process.
 * @param process Overwritten with the initial process variables.
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::beginDrainProcess(DrainTarget_t &target, ValveStates_e &state, DrainProcess_t &process) {
    return ESP_OK;
}

/**
 * @brief Updates the drain process.
 * 
 * @param state Overwritten with the current state.
 * @param process Overwritten with the current process variables.
 * @param summary Overwritten with the current process summary.
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::updateDrainProcesss(ValveStates_e &state, DrainProcess_t &process, DrainSummary_t &summary) {
    return ESP_OK;
}

/**
 * @brief Ends the drain process.
 * 
 * @param state Overwritten with the state.
 * @param process Overwritten with the current process variables.
 * @param summary Overwritten with the current process summary.
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::endDrainProcess(ValveStates_e &state, DrainProcess_t &process, DrainSummary_t &summary) {
    return ESP_OK;
}