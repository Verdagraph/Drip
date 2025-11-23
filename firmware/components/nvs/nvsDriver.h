#ifndef NVS_DRIVER_H
#define NVS_DRIVER_H

#include "nvs_flash.h"

enum VdgNvsBlocks {
    VDG_NVS_BLOCK_MIN,
    VDG_NVS_BLOCK_CONFIG,
    VDG_NVS_BLOCK_MAX
};

/**
 * @brief Provides access to non-volatile storage.
 */
class NvsDriver {
public:
    /**
     * @brief Constructor.
     */
    NvsDriver();

    /**
     * @brief Begin the NvsDriver.
     * 
     * @return esp_err_t Return code. 
     */
    esp_err_t initialize();
    
private:
};

#endif
