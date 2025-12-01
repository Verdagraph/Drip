#include "esp_err.h"

#include "dataContainer.h"

static const char* TAG = "Datacontainer";

/**
 * @brief Constructor.
 */
DataContainer::DataContainer() {}

/**
 * @brief Begin the DataContainer.
 * 
 * @return esp_err_t Return code. 
 */
esp_err_t DataContainer::initialize() {
    esp_err_t ret = ESP_OK;
    return ret;
}

/**
 * Measurement data.
 */
esp_err_t DataContainer::getMeasurementData(DripMeasurementData_t &data) {
    return ESP_OK;
}
esp_err_t DataContainer::setMeasurementData(DripMeasurementData_t data) {
    return ESP_OK;
}
esp_err_t DataContainer::getDerivedData(DripDerivedData_t &data) {
    return ESP_OK;
}
esp_err_t DataContainer::setDerivedData(DripDerivedData_t data) {
    return ESP_OK;
}


/**
 * Process data.
 */
esp_err_t DataContainer::getDispenseProcessData(VdgDispenseProcessData_t &data) {
    return ESP_OK;
}
esp_err_t DataContainer::setDispenseProcessData(VdgDispenseProcessData_t data) {
    return ESP_OK;
}
esp_err_t DataContainer::getDrainProcessData(VdgDrainProcessData_t &data) {
    return ESP_OK;
}
esp_err_t DataContainer::setDrainProcessData(VdgDrainProcessData_t data) {
    return ESP_OK;
}
esp_err_t DataContainer::getFlowCalibrationProcessData(DripFlowCalibrationProcessData_t &data) {
    return ESP_OK;
}
esp_err_t DataContainer::setFlowCalibrationProcessData(DripFlowCalibrationProcessData_t data) {
    return ESP_OK;
}

/**
 * Logging.
 */

/**
 * @brief Retrieves the number of stored logs.
 * 
 * @return size_t The number of stored logs. 
 */
size_t DataContainer::getNumLogs() {
    return 0U;
}

/**
 * @brief Logs an informational log.
 * 
 * @param[in] returnCode The return code, if any, associated with the log.
 * Defaults to ESP_OK;
 * @param[in] tag The ESP tag used for debug logging.
 * @param[in] message The log message.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t DataContainer::logInfo(esp_err_t returnCode, const char* tag, const char *message) {
    return ESP_OK;
}

/**
 * @brief Logs a warning log.
 * 
 * @param[in] returnCode The return code, if any, associated with the log.
 * Defaults to ESP_OK;
 * @param[in] tag The ESP tag used for debug logging.
 * @param[in] message The log message.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t DataContainer::logWarning(esp_err_t returnCode, const char* tag, const char *message) {
    return ESP_OK;
}

/**
 * @brief Logs an error log.
 * 
 * @param[in] returnCode The return code, if any, associated with the log.
 * Defaults to ESP_FAIL;
 * @param[in] tag The ESP tag used for debug logging.
 * @param[in] message The log message.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t DataContainer::logError(esp_err_t returnCode, const char* tag, const char *message) {
    return ESP_OK;
}

/**
 * @brief Gets the next available log. The log is popped from the buffer 
 * and erased, so it must be uploaded after this.
 * 
 * @param[out] message Output parameter for the retrieved log.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t DataContainer::getNextLog(DripLog_t &message) {
    return ESP_OK;
}

esp_err_t DataContainer::log(DripLog_t &log) {
    return ESP_OK;
}