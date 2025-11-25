#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

#include "gpioDriver.h"
#include "pins.h"

static const char* TAG = "GpioDriver";

GpioDriver gpioDriver = GpioDriver();

/**
 * @brief Constructor.
 */
GpioDriver::GpioDriver() {
    i2cBusConfig_ = {};
    i2cBusHandle_ = nullptr;
    i2cDeviceConfigAdc_ = {};
    i2cDeviceHandleAdc_ = nullptr;
}

/**
 * @brief Begin the GpioDriver.
 * 
 * @return esp_err_t Return code. 
 */
esp_err_t GpioDriver::initialize() {
    esp_err_t err = ESP_OK;

    err = configureI2C();
    
    return err;
}

esp_err_t GpioDriver::setRelay1State(DripGpioToggleState_e state) {
    esp_err_t err = ESP_OK;
    uint32_t level = 0U;

    if (state == DRIP_GPIO_TOGGLE_ON) {
        level = DRIP_GPIO_PIN_ENABLE;
    } else if (state == DRIP_GPIO_TOGGLE_OFF) {
        level = DRIP_GPIO_PIN_DISABLE;
    } else {
        // handle
    }

    err = gpio_set_level(DRIP_GPIO_PIN_RELAY1, level);
    if (err != ESP_OK) {
        // handle
    }

    return ESP_OK;
}

esp_err_t GpioDriver::setRelay2State(DripGpioToggleState_e state) {
    return ESP_OK;
}

esp_err_t GpioDriver::setRelay3State(DripGpioToggleState_e state) {
    return ESP_OK;
}

esp_err_t GpioDriver::i2cWriteReadTransaction(DripI2CDevices_e device, uint8_t *writeBuf, size_t writeBufLen, uint8_t *readBuf, size_t readBufLen, uint32_t timeoutMs) {
    i2c_master_dev_handle_t *devicePtr = nullptr;
    esp_err_t err = ESP_OK;

    /** Validate input. */
    if ( (writeBuf == nullptr) || (readBuf == nullptr) ) {
        // handle
    }

    /** Select device. */
    switch (device) {
        case DRIP_I2C_DEVICES_ADC:
            devicePtr = &i2cDeviceHandleAdc_;
        default:
            return ESP_FAIL;
            // catch error
    }

    err = i2c_master_transmit_receive(*devicePtr, writeBuf, writeBufLen, readBuf, readBufLen, timeoutMs);
    if (err != ESP_OK) {
        // handle
    }

    return ESP_OK;
}
esp_err_t GpioDriver::i2cWriteTransaction(DripI2CDevices_e device, uint8_t *writeBuf, size_t writeBufLen, uint32_t timeoutMs) {
    i2c_master_dev_handle_t *devicePtr = nullptr;
    esp_err_t err = ESP_OK;

    /** Validate input. */
    if (writeBuf == nullptr) {
        // handle
    }

    /** Select device. */
    switch (device) {
        case DRIP_I2C_DEVICES_ADC:
            devicePtr = &i2cDeviceHandleAdc_;
        default:
            return ESP_FAIL;
            // catch error
    }

    err = i2c_master_transmit(*devicePtr, writeBuf, writeBufLen, timeoutMs);
    if (err != ESP_OK) {
        // handle
    }

    return ESP_OK;
}

esp_err_t GpioDriver::configureRelayPins() {

    /** Configure pin directions. */
    gpio_set_direction(DRIP_GPIO_PIN_RELAY1, GPIO_MODE_OUTPUT);   
    gpio_set_direction(DRIP_GPIO_PIN_RELAY2, GPIO_MODE_OUTPUT);
    gpio_set_direction(DRIP_GPIO_PIN_RELAY3, GPIO_MODE_OUTPUT);

    return ESP_OK;
}

esp_err_t GpioDriver::configureI2C() {
    esp_err_t err = ESP_OK;

    /** Validate state. */
    if ( (i2cBusHandle_ != nullptr) || (i2cDeviceHandleAdc_ != nullptr) ) {
        //Already initialized.
    }

    /** Configure settings. */
    i2cBusConfig_ = {};
    /** Auto-select. */
    i2cBusConfig_.i2c_port = -1;
    i2cBusConfig_.clk_source = I2C_CLK_SRC_DEFAULT;
    i2cBusConfig_.scl_io_num = DRIP_GPIO_PIN_I2C_SCL;
    i2cBusConfig_.sda_io_num = DRIP_GPIO_PIN_I2C_SDA;
    i2cBusConfig_.glitch_ignore_cnt = 7;
    i2cBusConfig_.flags.enable_internal_pullup = false;

    err = i2c_new_master_bus(&i2cBusConfig_, &i2cBusHandle_);
    if ( (err != ESP_OK) || (i2cBusHandle_ == nullptr) ) {
        // handle error
    }

    /** Configure I2C Devices. */
    /** Configure ADC. */
    i2cDeviceConfigAdc_ = {};
    i2cDeviceConfigAdc_.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    i2cDeviceConfigAdc_.device_address = ADS111RUGT_I2C_ADDR;
    i2cDeviceConfigAdc_.scl_speed_hz = ADS111RUGT_I2C_CLK_FREQ_HZ;

    err = i2c_master_bus_add_device(i2cBusHandle_, &i2cDeviceConfigAdc_, &i2cDeviceHandleAdc_);
    if ( (err != ESP_OK) || (i2cDeviceHandleAdc_ == nullptr) ) {
        // handle error
    }

    err = i2c_master_probe(i2cBusHandle_, ADS111RUGT_I2C_ADDR, -1);
    if (err != ESP_OK) {
        // handler error.
    }

    return ESP_OK;
}