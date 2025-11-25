#ifndef RX_MESSAGES_H
#define RX_MESSAGES_H

#include "message.h"
#include "config.h"
#include "dataContainer.h"
#include "valveManager.h"


/** @brief Default; empty message payload; */
struct DripDefaultRxMessage_t {};

/** 
 * @brief Dispense activate command.
 */
using DispenseActivateRxMessage_t = VdgDispenseProcessTarget_t;
class DispenseActivateRxMessageContainer : public DripRxMessageContainer<DispenseActivateRxMessage_t> {
public:
    DispenseActivateRxMessageContainer() : DripRxMessageContainer<DispenseActivateRxMessage_t>() {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }
};

/**
 * @brief Process deactivation command.
 */
using DeactivateRxMessage_t = DripDefaultRxMessage_t;
class DeactivateRxMessageContainer : public DripRxMessageContainer<DeactivateRxMessage_t> {
public:
    DeactivateRxMessageContainer() : DripRxMessageContainer<DeactivateRxMessage_t>() {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }
};

/**
 * @brief Process restart command.
 */
using RestartRxMessage_t = DripDefaultRxMessage_t;
class RestartRxMessageContainer : public DripRxMessageContainer<RestartRxMessage_t> {
public:
    RestartRxMessageContainer() : DripRxMessageContainer<RestartRxMessage_t>() {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }
};

/**
 * @brief Config change command.
 */
using ConfigUpdateRxMessage_t = DripConfig_t;
class ConfigUpdateRxMessageContainer : public DripRxMessageContainer<ConfigUpdateRxMessage_t> {
public:
    ConfigUpdateRxMessageContainer() : DripRxMessageContainer<ConfigUpdateRxMessage_t>() {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }
};

/**
 * @brief Flow calibration dispense and measure command.
 */
struct FlowCalibrationUpdateRxMessage_t {
    /** 
     * @brief The target volume of this calibration step in liters.
     * Ignored if conclude is true.
     */
    float targetVolume;
    /** 
     * @brief The timeout of this calibration step in miliseconds. 
     * Ignored if conclude is true.
     */
    uint32_t timeoutMs;
    /** @brief The measured volume from the last calibration step if applicable. */
    float measuredVolume;
    /** @brief If true the calibration process is concluded. */
    bool conclude = false;
};
class FlowCalibrationUpdateRxMessageContainer : public DripRxMessageContainer<FlowCalibrationUpdateRxMessage_t> {
public:
    FlowCalibrationUpdateRxMessageContainer() : DripRxMessageContainer<FlowCalibrationUpdateRxMessage_t>() {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }
};

/**
 * @brief Pressure calibration command. 
 */
using PressureCalibrateRxMessage_t = DripDefaultRxMessage_t;
class PressureCalibrateRxMessageContainer : public DripRxMessageContainer<PressureCalibrateRxMessage_t> {
public:
    PressureCalibrateRxMessageContainer() : DripRxMessageContainer<PressureCalibrateRxMessage_t>() {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }
};

/** 
 * @brief Drain activate command.
 */
using DrainActivateRxMessage_t = VdgDrainProcessTarget_t;
class DrainActivateRxMessageContainer : public DripRxMessageContainer<DrainActivateRxMessage_t> {
public:
    DrainActivateRxMessageContainer() : DripRxMessageContainer<DrainActivateRxMessage_t>() {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }
};

/**
 * @brief Pressure poll command.
 */
using PressurePollRxMessage_t = DripDefaultRxMessage_t;
class DispenseActivateRxMessageContainer : public DripRxMessageContainer<PressurePollRxMessage_t> {
public:
    DispenseActivateRxMessageContainer() : DripRxMessageContainer<PressurePollRxMessage_t>() {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }
};

#endif