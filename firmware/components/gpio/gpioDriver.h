#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "driver/i2c_master.h"

enum DripGpioToggleState_e {
    DRIP_GPIO_TOGGLE_MIN,
    DRIP_GPIO_TOGGLE_ON,
    DRIP_GPIO_TOGGLE_OFF,
    DRIP_GPIO_TOGGLE_MAX,
};

enum DripI2CDevices_e {
    DRIP_I2C_DEVICES_MIN,

    DRIP_I2C_DEVICES_ADC,

    DRIP_I2C_DEVICES_MAX,
};

/**
 * @brief Provides access to all GPIO pins.
 */
class GpioDriver {
public:
    /**
     * @brief Constructor.
     */
    GpioDriver();

    /**
     * @brief Begin the GpioDriver.
     * 
     * @return esp_err_t Return code. 
     */
    esp_err_t initialize();
    
    esp_err_t setRelay1State(DripGpioToggleState_e state);
    esp_err_t setRelay2State(DripGpioToggleState_e state);
    esp_err_t setRelay3State(DripGpioToggleState_e state);

    esp_err_t i2cWriteReadTransaction(DripI2CDevices_e device, uint8_t *writeBuf, size_t writeBufLen, uint8_t *readBuf, size_t readBufLen, uint32_t timeoutMs);
    esp_err_t i2cWriteTransaction(DripI2CDevices_e device, uint8_t *writeBuf, size_t writeBufLen, uint32_t timeoutMs);
    
    
    
private:
    i2c_master_bus_config_t i2cBusConfig_;
    i2c_master_bus_handle_t i2cBusHandle_;

    i2c_device_config_t i2cDeviceConfigAdc_;
    i2c_master_dev_handle_t i2cDeviceHandleAdc_;
    
    esp_err_t configureRelayPins();
    esp_err_t configureI2C();

    esp_err_t configureFlowSensorInterrupt();
};

extern GpioDriver gpioDriver;

#endif