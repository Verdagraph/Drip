#include "esp_err.h"

#include "configManager.h"

static const char* TAG = "ConfigManager";

/**
 * @brief Constructor.
 */
ConfigManager::ConfigManager() {
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
esp_err_t ConfigManager::getConfig(Config_t &config) {
    return ESP_OK;
}

/**
 * @brief Updates the in-memory config object on the class instance.
 * 
 * @param config New config.
 * @return esp_err_t Return code.
 */
esp_err_t ConfigManager::setConfig(Config_t &config) {
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

/**
 * @brief Refreshes the in-memory config object to the non-volatile values. 
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ConfigManager::refresh() {
    return ESP_OK;
}