#include "esp_err.h"

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
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS formatting required...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    } else if (ret != ESP_OK) {
        return ret;
    }

    ESP_LOGI(TAG, "NVS successfully initialized.");
    return ESP_OK;
}