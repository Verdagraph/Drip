#ifndef RX_MESSAGES_H
#define RX_MESSAGES_H

#include "json-maker/json-maker.h"

#include "message.h"
#include "dataContainer.h"
#include "valveManager.h"


/** @brief Default; empty message payload; */
struct DripDefaultRxMessage_t {};

/**
 * @brief Dispense slice report.
 */
using DispenseSliceTxMessage_t = VdgDispenseProcessSlice_t;
class DispenseSliceTxMessageContainer : public DripTxMessageContainer<DripTxMessageId_e, DispenseSliceTxMessage_t> {
public:
    DispenseSliceTxMessageContainer() : DripTxMessageContainer<DripTxMessageId_e, DispenseSliceTxMessage_t>(DripTxMessageId_e::DispenseSlice) {}

    esp_err_t toJson(const DispenseSliceTxMessage_t data) override {
        return ESP_OK;
    }
};


/**
 * @brief Dispense summary report.
 */
using DispenseSummaryTxMessage_t = VdgDispenseProcessSummary_t;
class DispenseSummaryMessageContainer : public DripTxMessageContainer<DripTxMessageId_e, DispenseSummaryTxMessage_t> {
public:
    DispenseSummaryMessageContainer() : DripTxMessageContainer<DripTxMessageId_e, DispenseSummaryTxMessage_t>(DripTxMessageId_e::DispenseSummary) {}

    esp_err_t toJson(const DispenseSummaryTxMessage_t data) override {
        return ESP_OK;
    }
};

/**
 * @brief Info, warning, and error logs.
 */
using LogTxMessage_t = DripLog_t;
class LogTxMessageContainer : public DripTxMessageContainer<DripTxMessageId_e, LogTxMessage_t> {
public:
    LogTxMessageContainer() : DripTxMessageContainer<DripTxMessageId_e, LogTxMessage_t>(DripTxMessageId_e::Log) {}

    esp_err_t toJson(const LogTxMessage_t data) override {
        return ESP_OK;
    }
};

/**
 * @brief Config read command.
 */
using ConfigTxMessage_t = DripConfig_t;
class ConfigTxMessageContainer : public DripTxMessageContainer<DripTxMessageId_e, ConfigTxMessage_t> {
public:
    ConfigTxMessageContainer() : DripTxMessageContainer<DripTxMessageId_e, ConfigTxMessage_t>(DripTxMessageId_e::ConfigRead) {}

    esp_err_t toJson(const ConfigTxMessage_t data) override {
        return ESP_OK;
    }
};

/**
 * @brief Drain summary report.
 */
using DrainSummaryTxMessage_t = VdgDrainProcessSummary_t;
class DrainSummaryTxMessageContainer : public DripTxMessageContainer<DripTxMessageId_e, DrainSummaryTxMessage_t> {
public:
    DrainSummaryTxMessageContainer() : DripTxMessageContainer<DripTxMessageId_e, DrainSummaryTxMessage_t>(DripTxMessageId_e::DrainSummary) {}

    esp_err_t toJson(const DrainSummaryTxMessage_t data) override {
        return ESP_OK;
    }
};

/**
 * @brief Pressure report.
 */
using PressureReportTxMessage_t = VdgDrainProcessSummary_t;
class PressureReportTxMessageContainer : public DripTxMessageContainer<DripTxMessageId_e, PressureReportTxMessage_t> {
public:
    PressureReportTxMessageContainer() : DripTxMessageContainer<DripTxMessageId_e, PressureReportTxMessage_t>(DripTxMessageId_e::PressureReport) {}

    esp_err_t toJson(const PressureReportTxMessage_t data) override {
        return ESP_OK;
    }
};

#endif