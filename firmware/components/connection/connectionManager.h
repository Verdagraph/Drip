#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "esp_err.h"

/**
 * @brief Handles connecting and provisioning WiFi and MQTT.
 */
class ConnectionManager {
public:
    /**
     * @brief Constructor.
     */
    ConnectionManager();

    /**
     * @brief Begin the ConnectionManager.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t initialize();

    /**
     * @brief If true, the ConnectionManager is currently provisioning.
     * 
     * @returns The isProvisioning flag. 
     */
    bool isProvisioning();

    /**
     * @brief If true, the ConnectionManager has successful WiFi
     * and MQTT connection.
     * 
     * @returns the isConnected flag.
     */
    bool isConnected();

    /**
     * @brief Attempts to initiate a WiFi and MQTT connection.
     * Blocks until the connection is resolved or failed.
     * 
     * @param connected If set to true, the connection succeeded.
     * @return esp_err_t Return code.
     */
    esp_err_t connect(bool &connected);

    /**
     * @brief Begins the provisioning process.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t beginProvisioning();

private:
    bool _isProvisioning;
    bool _isConnected;
};

#endif