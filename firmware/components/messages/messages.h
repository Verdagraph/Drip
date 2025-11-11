#ifndef MQTT_MESSAGES_H
#define MQTT_MESSAGES_H

#include "config.h"
#include "dataContainer.h"
#include "valveManager.h"

typedef uint8_t VdgMessageId_t;

#define VDG_MSG_MAX_PAYLOAD_BYTES     512U

/** Minimum message ID. */
#define VDG_MSG_MIN                   0U
/** Minimum message ID in the RX range. */
#define VDG_MSG_RX_MIN                1U

#define VDG_MSG_RX_DISPENSE_ACTIVATE  2U
#define VDG_MSG_RX_DEACTIVATE         3U
#define VDG_MSG_RX_RESTART            4U
#define VDG_MSG_RX_CHANGE_CONFIG      5U
#define VDG_MSG_RX_FLOW_CALIBRATE     6U
#define VDG_MSG_RX_PRESSURE_CALIBRATE 7U
#define VDG_MSG_RX_DRAIN              8U
#define VDG_MSG_RX_PRESSURE_POLL      9U

/** Maximum message ID in the RX range. */
#define VDG_MSG_RX_MAX                99U
/** Minimum message ID in the TX range. */
#define VDG_MSG_TX_MIN                100U

#define VDG_MSG_TX_DISPENSE_SLICE     101U
#define VDG_MSG_TX_DISPENSE_SUMMARY   102U
#define VDG_MSG_TX_INFO_LOG           103U
#define VDG_MSG_TX_WARNING_LOG        104U
#define VDG_MSG_TX_ERROR_LOG          105U
#define VDG_MSG_TX_READ_CONFIG        106U
#define VDG_MSG_TX_DRAIN_SUMMARY      107U
#define VDG_MSG_TX_PRESSURE           108U

/** Maximum message ID in the TX range. */
#define VDG_MSG_TX_MAX                199U
/** Maximum message ID. */
#define VDG_MSG_MAX                   200U


/**
 * @brief Describes a message.
 */
struct VdgMessage_t {
    /** ID of the message. */
    VdgMessageId_t id;
    /** A pointer to the raw message payload. */
    uint8_t *payload;
    /** The number of bytes in the payload. */
    size_t payloadLen;
};

/**
 * @brief Config change command.
 */
struct MqttRxConfigMessage_t {
    Config_t config;
};

/** 
 * @brief Dispense activate command.
 */
typedef VdgDispenseProcessTarget_t VdgDispenseActivateCommand_t;

/**
 * @brief Flow calibration dispense and measure command.
 */
struct VdgFlowCalibrationUpdateCommand_t {
    /** 
     * The target volume of this calibration step in liters.
     * Ignored if conclude is true.
     */
    float targetVolume;
    /** 
     * The timeout of this calibration step in miliseconds. 
     * Ignored if conclude is true.
     */
    uint32_t timeoutMs;
    /** The measured volume from the last calibration step if applicable. */
    float measuredVolume;
    /** If true the calibration process is concluded. */
    bool conclude = false;
};

/** Outgoing messages. */

/**
 * @brief Dispense slice report.
 */
struct MqttTxDispenseSliceMessage_t {
    uint32_t time;
    float volume;
    float flowRate;
    float waterLevel;
    float waterVolume; 
};

/**
 * @brief Dispense summary report.
 */
struct MqttTxDispenseSummaryMessage_t {
    uint32_t duration;
    float volume;
    float tankVolume;
    uint32_t tankSwitchoverTime;
};

/**
 * @brief Info, warning, and error logs.
 */
struct MqttTxLogMessage_t {
    char message[VDG_LOG_MESSAGE_BUFFER_BYTES];
};

/**
 * @brief Config report.
 */
struct MqttTxConfigMessage_t {
    Config_t config;
};

#endif