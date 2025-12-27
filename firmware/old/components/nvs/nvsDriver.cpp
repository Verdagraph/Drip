#include "esp_err.h"
#include "esp_log.h"

#include "nvsDriver.h"


static const char* TAG = "NvsDriver";



/**
 * @brief Constructor.
 */
NvsDriver::NvsDriver() {}

/**
 * @brief Begin the NvsDriver.
 * 
 * @return esp_err_t Return code. 
 */
esp_err_t NvsDriver::initialize() {
    esp_err_t err = ESP_OK;

    ESP_LOGI(TAG, "Initializing NVS...");
    
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS formatting required...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    } else if (err != ESP_OK) {
        return err;
    }

    ESP_LOGI(TAG, "NVS successfully initialized.");
    return ESP_OK;
}