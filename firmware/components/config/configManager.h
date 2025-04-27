#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "esp_err.h"
#include "config.h"

/**
 * @brief Handles reading from and writing to the persistent config.
 */
class ConfigManager {
public:
    /**
     * @brief Constructor.
     */
    ConfigManager();
 
    /**
     * @brief Begins the ConfigManager.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t initialize();

    /**
     * @brief Retrieve the application configuration.
     * 
     * @param config Overwritten with the configuration.
     * @return esp_err_t Return code.
     */
    esp_err_t getConfig(Config_t &config);

    /**
     * @brief Updates the in-memory config object on the class instance.
     * 
     * @param config New config.
     * @return esp_err_t Return code.
     */
    esp_err_t setConfig(Config_t &config);

    /**
     * @brief Persists the current config to non-volatile memory.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t persist();

    /**
     * @brief Refreshes the in-memory config object to the non-volatile values. 
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t refresh();

private:
    Config_t config;
    PressureSensorCalibrationPoint_t pressureCalibration[MAX_PRESSURE_CALIBRATION_POINTS];
};

#endif