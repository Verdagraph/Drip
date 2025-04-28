
#ifndef MQTT_MESSAGES_H
#define MQTT_MESSAGES_H

#include "config.h"
#include "valveManager.h"

#define MAX_LOG_MESSAGE_BYTES 256

/**
 * @brief Defines the possible messages to be received.
 */
typedef enum MqttRxMessages_e {
    MQTT_RX_MIN,

    MQTT_RX_DISPENSE_ACTIVATE,
    MQTT_RX_DEACTIVATE,
    MQTT_RX_RESTART,
    MQTT_RX_CHANGE_CONFIG,
    MQTT_RX_FLOW_CALIBRATE,
    MQTT_RX_PRESSURE_CALIBRATE,
    MQTT_RX_DRAIN,
    MQTT_RX_PRESSURE_POLL,

    MQTT_RX_MAX
} MqttRxMessages_e;

/**
 * @brief Defines the possible messages to be transmitted.
 */
typedef enum MqttTxMessages_e {
    MQTT_TX_MIN,

    MQTT_TX_DISPENSE_SLICE,
    MQTT_TX_DISPENSE_SUMMARY,
    MQTT_TX_INFO_LOG,
    MQTT_TX_WARNING_LOG,
    MQTT_TX_ERROR_LOG,
    MQTT_TX_READ_CONFIG,
    MQTT_TX_DRAIN_SUMMARY,
    MQTT_TX_PRESSURE,
    
    MQTT_TX_MAX
} MqttTxMessages_e;

/** Incoming messages. */

/**
 * @brief Describes a message that has been received.
 */
typedef struct MqttRxMessage_t {
    MqttRxMessages_e messageCode;
    char* payload;
} MqttRxMessage_t;

/**
 * @brief Config change command.
 */
typedef struct MqttRxConfigMessage_t {
    Config_t config;
} MqttRxConfigMessage_t;

/** 
 * @brief Dispense activate command.
 */
typedef DispenseTarget_t MqttRxDispenseActivateMessage_t;

/** Outgoing messages. */

/**
 * @brief Dispense slice report.
 */
typedef struct MqttTxDispenseSliceMessage_t {
    uint32_t time;
    float volume;
    float flowRate;
    float waterLevel;
    float waterVolume; 
} MqttTxDispenseSliceMessage_t;

/**
 * @brief Dispense summary report.
 */
typedef struct MqttTxDispenseSummaryMessage_t {
    uint32_t duration;
    float volume;
    float tankVolume;
    uint32_t tankSwitchoverTime;
} MqttTxDispenseSummaryMessage_t;

/**
 * @brief Info, warning, and error logs.
 */
typedef struct MqttTxLogMessage_t {
    char message[MAX_LOG_MESSAGE_BYTES];
} MqttTxLogMessage_t;

/**
 * @brief Config report.
 */
typedef struct MqttTxConfigMessage_t {
    Config_t config;
} MqttTxConfigMessage_t;

#endif