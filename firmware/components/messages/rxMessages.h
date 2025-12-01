#ifndef RX_MESSAGES_H
#define RX_MESSAGES_H

#include "tiny-json.h"

#include "message.h"
#include "config.h"
#include "dataContainer.h"
#include "valveManager.h"

/** 
 * @brief Dispense activate command.
 */
using DispenseActivateRxMessagePayload_t = VdgDispenseProcessTarget_t;
class DispenseActivateRxMessage : public DripRxMessage {
public:
    DispenseActivateRxMessage() : DripRxMessage(DripRxMessageId_e::DispenseActivate) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }

    DispenseActivateRxMessagePayload_t data() {
        return messageData;
    }

private:
    DispenseActivateRxMessagePayload_t messageData;
};

/**
 * @brief Process deactivation command.
 */
class DeactivateRxMessage : public DripRxMessage {
public:
    DeactivateRxMessage() : DripRxMessage(DripRxMessageId_e::Deactivate) {}

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
class RestartRxMessage : public DripRxMessage {
public:
    RestartRxMessage() : DripRxMessage(DripRxMessageId_e::Restart) {}

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
using ConfigUpdateRxMessagePayload_t = DripConfig_t;
class ConfigUpdateRxMessage : public DripRxMessage {
public:
    ConfigUpdateRxMessage() : DripRxMessage(DripRxMessageId_e::ConfigUpdate) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }

    ConfigUpdateRxMessagePayload_t data() {
        return messageData;
    }

private:
    ConfigUpdateRxMessagePayload_t messageData;
};

/**
 * @brief Flow calibration dispense and measure command.
 */
struct FlowCalibrationUpdateRxMessagePayload_t {
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
class FlowCalibrationUpdateRxMessage : public DripRxMessage {
public:
    FlowCalibrationUpdateRxMessage() : DripRxMessage(DripRxMessageId_e::FlowCalibrate) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }

    FlowCalibrationUpdateRxMessagePayload_t data() {
        return messageData;
    }

private:
    FlowCalibrationUpdateRxMessagePayload_t messageData;
};

/**
 * @brief Pressure calibration command. 
 */
struct PressureCalibrateRxMessagePayload_t {

};
class PressureCalibrateRxMessage : public DripRxMessage {
public:
    PressureCalibrateRxMessage() : DripRxMessage(DripRxMessageId_e::PressureCalibrate) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }

    PressureCalibrateRxMessagePayload_t data() {
        return messageData;
    }

private:
    PressureCalibrateRxMessagePayload_t messageData;
};

/** 
 * @brief Drain activate command.
 */
using DrainActivateRxMessagePayload_t = VdgDrainProcessTarget_t;
class DrainActivateRxMessage : public DripRxMessage {
public:
    DrainActivateRxMessage() : DripRxMessage(DripRxMessageId_e::DrainActivate) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }

    DrainActivateRxMessagePayload_t data() {
        return messageData;
    }

private:
    DrainActivateRxMessagePayload_t messageData;
};

/**
 * @brief Pressure poll command.
 */
class PressurePollRxMessage : public DripRxMessage {
public:
    PressurePollRxMessage() : DripRxMessage(DripRxMessageId_e::PressurePoll) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container, const DripConfig_t &config) {
        return ESP_OK;
    }
};

#endif