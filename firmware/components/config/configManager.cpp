#include "esp_err.h"

#include "configManager.h"

static const char* TAG = "ConfigManager";

/**
 * @brief Constructor.
 */
ConfigManager::ConfigManager(DataContainer &dataContainer) {
    return;
}

/**
 * @brief Begins the ConfigManager.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ConfigManager::initialize() {
    return ESP_OK;
}

/**
 * @brief Retrieve the application configuration.
 * 
 * @param config Overwritten with the configuration.
 * @return esp_err_t Return code.
 */
esp_err_t ConfigManager::getConfig(DripConfig_t &config) {
    return ESP_OK;
}

/**
 * @brief Updates the in-memory config object on the class instance.
 * 
 * @param config New config.
 * @return esp_err_t Return code.
 */
esp_err_t ConfigManager::setConfig(DripConfig_t &config) {
    return ESP_OK;
}

/**
 * @brief Persists the current config to non-volatile memory.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ConfigManager::persist() {
    return ESP_OK;
}
