#include "esp_err.h"

#include "flowManager.h"

static const char* TAG = "FlowManager";

/**
 * @brief Constructor.
 */
FlowSensorManager::FlowSensorManager() {}

/**
 * @brief Begin the FlowSensorManager.
 * 
 * @return esp_err_t Return code. 
 */
esp_err_t initialize() {
    return ESP_OK;
}

/**
 * @brief Begins a calibration process.
 * 
 * @param[out] state Output parameter for the current state after this function.
 *
 * @return esp_err_t Return code.
 */
esp_err_t beginCalibration(VdgFlowSensorCalibrationState_e &state) {
    return ESP_OK;
}

/**
 * @brief Accepts a measurement into the calibration process.
 * 
 * @param[out] state Output parameter for the current state after this function.
 * @param[in] measurement The new measurement.
 * @param process Overwritten with the final process variables.
 * @return esp_err_t Return code.
 */
esp_err_t inputCalibration(VdgFlowSensorCalibrationState_e &state, VdgFlowCalibrationMeasurement_t measurement) {
    return ESP_OK;
}

/**
 * @brief Ends the current process.
 * 
 * @param[out] state Output parameter for the current state after this function.
 *
 * @return esp_err_t Return code.
 */
esp_err_t endProcess(VdgFlowSensorCalibrationState_e &state) {
    return ESP_OK;
}