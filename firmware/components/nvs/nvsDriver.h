#ifndef NVS_DRIVER_H
#define NVS_DRIVER_H



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
