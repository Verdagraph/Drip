#ifndef FLOW_MANAGER_H
#define FLOW_MANAGER_H

enum VdgGpioToggleState_e {
    VDG_GPIO_TOGGLE_MIN,
    VDG_GPIO_TOGGLE_ON,
    VDG_GPIO_TOGGLE_OFF,
    VDG_GPIO_TOGGLE_MAX,
};

enum VdgI2CDevices_e {
    VDG_I2C_DEVICES_MIN,

    VDG_I2C_DEVICES_ADC,

    VDG_I2C_DEVICES_MAX,
};

extern GpioDriver gpioDriver;

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
    
    esp_err_t setRelay1State(VdgGpioToggleState_e state);
    esp_err_t setRelay2State(VdgGpioToggleState_e state);
    esp_err_t setRelay3State(VdgGpioToggleState_e state);

    esp_err_t i2cWriteReadTransaction(VdgI2CDevices_e device, uint8_t *writeBuf, size_t writeBufLen, uint8_t *readBuf, size_t readBufLen, uint32_t timeoutMs);
    esp_err_t i2cWriteTransaction(VdgI2CDevices_e device, uint8_t *writeBuf, size_t writeBufLen, uint32_t timeoutMs);
    
    
    
private:
    i2c_master_bus_config_t i2cBusConfig_;
    i2c_master_bus_handle_t i2cBusHandle_;

    i2c_device_config_t i2cDeviceConfigAdc_;
    i2c_master_dev_handle_t i2cDeviceHandleAdc_;
    
    esp_err_t configureRelayPins();
    esp_err_t configureI2C();

    esp_err_t configureFlowSensorInterrupt();
};

#endif