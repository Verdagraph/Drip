#ifndef RX_MESSAGES_H
#define RX_MESSAGES_H

#include "tiny-json.h"

#include "message.h"
#include "messageIds.h"
#include "config.h"
#include "dataContainer.h"
#include "valveManager.h"

/** 
 * @brief Dispense activate command.
 */
using DispenseActivateRxMessagePayload_t = VdgDispenseProcessTarget_t;
class DispenseActivateRxMessage : public DripRxMessage<DripRxMessageId_e> {
public:
    DispenseActivateRxMessage() : DripRxMessage<DripRxMessageId_e>(DripRxMessageId_e::DispenseActivate) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen, const DataContainer &container) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container) {
        return ESP_OK;
    }

    DispenseActivateRxMessagePayload_t data() const {
        return messageData;
    }

private:
    DispenseActivateRxMessagePayload_t messageData;
};

/**
 * @brief Process deactivation command.
 */
class DeactivateRxMessage : public DripRxMessage<DripRxMessageId_e> {
public:
    DeactivateRxMessage() : DripRxMessage<DripRxMessageId_e>(DripRxMessageId_e::Deactivate) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen, const DataContainer &container) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container) {
        return ESP_OK;
    }
};

/**
 * @brief Process restart command.
 */
class RestartRxMessage : public DripRxMessage<DripRxMessageId_e> {
public:
    RestartRxMessage() : DripRxMessage<DripRxMessageId_e>(DripRxMessageId_e::Restart) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen, const DataContainer &container) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container) {
        return ESP_OK;
    }
};

/**
 * @brief Config change command.
 */
using ConfigUpdateRxMessagePayload_t = DripConfig_t;
class ConfigUpdateRxMessage : public DripRxMessage<DripRxMessageId_e> {
public:
    ConfigUpdateRxMessage() : DripRxMessage<DripRxMessageId_e>(DripRxMessageId_e::ConfigUpdate) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen, const DataContainer &container) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container) {
        return ESP_OK;
    }

    ConfigUpdateRxMessagePayload_t data() {
        return messageData;
    }

private:
    ConfigUpdateRxMessagePayload_t messageData;
};

/**
 * @brief Flow calibration begin command.
 */
class FlowCalibrateBeginRxMessage : public DripRxMessage<DripRxMessageId_e> {
public:
    FlowCalibrateBeginRxMessage() : DripRxMessage<DripRxMessageId_e>(DripRxMessageId_e::FlowCalibrateBegin) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen, const DataContainer &container) override {
        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container) {
        return ESP_OK;
    }
};

/**
 * @brief Flow calibration dispense command.
 */
using FlowCalibrateDispenseRxMessagePayload_t = DripFlowCalibrationTarget_t;
class FlowCalibrateDispenseRxMessage : public DripRxMessage<DripRxMessageId_e> {
public:
    FlowCalibrateDispenseRxMessage() : DripRxMessage<DripRxMessageId_e>(DripRxMessageId_e::FlowCalibrateDispense) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen, const DataContainer &container) override {
        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container) {
        return ESP_OK;
    }

    FlowCalibrateDispenseRxMessagePayload_t data() {
        return messageData;
    }

private:
    FlowCalibrateDispenseRxMessagePayload_t messageData;
};

/**
 * @brief Flow calibration measure command.
 */
using FlowCalibrateMeasureRxMessagePayload_t = DripFlowCalibrationMeasurement_t;
class FlowCalibrateMeasureRxMessage : public DripRxMessage<DripRxMessageId_e> {
public:
    FlowCalibrateMeasureRxMessage() : DripRxMessage<DripRxMessageId_e>(DripRxMessageId_e::FlowCalibrateMeasure) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen, const DataContainer &container) override {
        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container) {
        return ESP_OK;
    }

    FlowCalibrateMeasureRxMessagePayload_t data() {
        return messageData;
    }

private:
    FlowCalibrateMeasureRxMessagePayload_t messageData;
};

/**
 * @brief Flow calibration end command.
 */
class FlowCalibrateEndRxMessage : public DripRxMessage<DripRxMessageId_e> {
public:
    FlowCalibrateEndRxMessage() : DripRxMessage<DripRxMessageId_e>(DripRxMessageId_e::FlowCalibrateEnd) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen, const DataContainer &container) override {
        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container) {
        return ESP_OK;
    }
};

/**
 * @brief Pressure calibration command. 
 */
struct PressureCalibrateUpdateRxMessagePayload_t {

};
class PressureCalibrateUpdateRxMessage : public DripRxMessage<DripRxMessageId_e> {
public:
    PressureCalibrateUpdateRxMessage() : DripRxMessage<DripRxMessageId_e>(DripRxMessageId_e::PressureCalibrateUpdate) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen, const DataContainer &container) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container) {
        return ESP_OK;
    }

    PressureCalibrateUpdateRxMessagePayload_t data() {
        return messageData;
    }

private:
    PressureCalibrateUpdateRxMessagePayload_t messageData;
};

/** 
 * @brief Drain activate command.
 */
using DrainActivateRxMessagePayload_t = VdgDrainProcessTarget_t;
class DrainActivateRxMessage : public DripRxMessage<DripRxMessageId_e> {
public:
    DrainActivateRxMessage() : DripRxMessage<DripRxMessageId_e>(DripRxMessageId_e::DrainActivate) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen, const DataContainer &container) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container) {
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
class PressurePollRxMessage : public DripRxMessage<DripRxMessageId_e> {
public:
    PressurePollRxMessage() : DripRxMessage<DripRxMessageId_e>(DripRxMessageId_e::PressurePoll) {}

    esp_err_t fromJson(uint8_t *payload, size_t payloadLen, const DataContainer &container) override {

        return ESP_OK;
    }

    esp_err_t validate(const DataContainer &container) {
        return ESP_OK;
    }
};

#endif