#include "esp_err.h"

#include "connectionManager.h"

static const char* TAG = "ConnectionManager";

/**
 * @brief Constructor.
 */
ConnectionManager::ConnectionManager() {
    return;
}

/**
 * @brief Begin the ConnectionManager.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ConnectionManager::initialize() {
    return ESP_OK;
}

/**
 * @brief If true, the ConnectionManager is currently provisioning.
 * 
 * @returns The isProvisioning flag. 
 */
bool ConnectionManager::isProvisioning() {
    return true;
}

/**
 * @brief If true, the ConnectionManager has successful WiFi
 * and MQTT connection.
 * 
 * @returns the isConnected flag.
 */
bool ConnectionManager::isConnected() {
    return true;
}

/**
 * @brief Attempts to initiate a WiFi and MQTT connection.
 * Blocks until the connection is resolved or failed.
 * 
 * @param connected If set to true, the connection succeeded.
 * @return esp_err_t Return code.
 */
esp_err_t ConnectionManager::connect(bool &connected) {
    return ESP_OK;
}

/**
 * @brief Begins the provisioning process.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ConnectionManager::beginProvisioning() {
    return ESP_OK;
}