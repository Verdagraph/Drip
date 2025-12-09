/**
 * @file mainStateController.h
 * @author Nathaniel King
 * @brief Defines the main finite state machine.
 * @date 2025-12-03
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef MAIN_STATE_CONTROLLER_H
#define MAIN_STATE_CONTROLLER_H

#include <etl/const_map.h>

#include "stateController.h"

#include "config.h"
#include "configManager.h"
#include "rxMessages.h"
#include "configManager.h"
#include "dataContainer.h"
#include "wifiManager.h"
#include "mqttManager.h"
#include "valveManager.h"
#include "flowManager.h"

#include "states.h"

/** @brief The amount of time in miliseconds to wait after beginning a WiFi connection attempt before entering the provisioning state. */
constexpr uint32_t DRIP_MAIN_FSM_WIFI_CONNECTION_WAIT_MS = 10000U;

/**
 * @brief Holds timing information.
 */
struct DripMainFsmEventTimers_t {
    /** @brief The timestamp that the attempted wifi connection began. */
    uint32_t wifiConnectionBeganTicks;
    /** @brief The timestamp that the attempted mqtt connection began. */
    uint32_t mqttConnectionBeganTicks;
    /** @brief The timestamp of the last time a process slice was uploaded. */
    uint32_t lastProcessSliceUploadTicks;
};

/**
 * @brief Defines main application routines.
 */

class MainStateController : public StateController<DripMainFsmState_e, DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController> {
public:
    /**
     * @brief Constructor.
     */
    MainStateController(
        const ConfigManager &configManager,
        const DataContainer &dataContainer,
        const WifiManager &wifiManager, 
        const MqttManager &mqttManager,
        const ValveManager &valveManager,
        const FlowSensorManager &flowSensorManager
    );

    /**
     * @defgroup StateHandler State handlers.
     */

    /**
     * @brief Handlers for state uninitialized.
     */
    void uninitializedEntry();
    void uninitializedUpdate();
    void uninitializedExit();

    /**
     * @brief Handlers for state boot.
     */
    void bootEntry();
    void bootUpdate();
    void bootExit();

    /**
     * @brief Handlers for state fatal error.
     */
    void fatalErrorEntry();
    void fatalErrorUpdate();
    void fatalErrorExit();

    /**
     * @brief Handlers for state connect wifi.
     */
    void connectWifiEntry();
    void connectWifiUpdate();
    void connectWifiExit();

    /**
     * @brief Handlers for state connect mqtt.
     */
    void connectMqttEntry();
    void connectMqttUpdate();
    void connectMqttExit();

    /**
     * @brief Handlers for state provisioning.
     */
    void provisioningEntry();
    void provisioningUpdate();
    void provisioningExit();

    /**
     * @brief Handlers for state restart.
     */
    void restartEntry();
    void restartUpdate();
    void restartExit();

    /**
     * @brief Handlers for state listen.
     */
    void listenEntry();
    void listenUpdate();
    void listenExit();

    /**
     * @brief Handlers for state dispense.
     */
    void dispenseEntry();
    void dispenseUpdate();
    void dispenseExit();

    /**
     * @brief Handlers for state flow calibrate.
     */
    void flowSensorCalibrateEntry();
    void flowSensorCalibrateUpdate();
    void flowSensorCalibrateExit();

    /**
     * @brief Handlers for state drain.
     */
    void drainEntry();
    void drainUpdate();
    void drainExit();

    /** @} */

    /**
     * @defgroup EventHandlers Event handlers.
     */

    /**
     * @brief Handles state change for a dispense request in the listen state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type DispenseActivateRxMessage.
     */
    void handleDispenseRequestStateListen(DripRxMessageId_e id, const DripRxMessage<DripMainFsmState_e> *message);

    /**
     * @brief Handles state change for a deactivation request in the dispense,
     * drain, or flow sensor calibration state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type DeactivateRxMessage.
     */
    void handleDeactivateRequestStateDispenseDrainOrFlowCalibration(DripRxMessageId_e id, const DripRxMessage<DripMainFsmState_e> *message);

    /**
     * @brief Handles state change for a restart request in the listen state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type RestartRxMessage.
     */
    void handleRestartRequestStateListen(DripRxMessageId_e id, const DripRxMessage<DripMainFsmState_e> *message);

    /**
     * @brief Handles state change for a config update request in the listen state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type ConfigUpdateRxMessage.
     */
    void handleConfigUpdateRequestStateListen(DripRxMessageId_e id, const DripRxMessage<DripMainFsmState_e> *message);

    /**
     * @brief Handles state change for a flow calibrate begin request when in the listen state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type FlowCalibrationBeginRxMessage.
     */
    void handleFlowCalibrateBeginRequestStateListen(DripRxMessageId_e id, const DripRxMessage<DripMainFsmState_e> *message);

    /**
     * @brief Handles state change for a flow calibrate dispense request when 
     * in the flow sensor calibrate state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type FlowCalibrationDispenseRxMessage.
     */
    void handleFlowCalibrateDispenseRequestStateFlowCalibration(DripRxMessageId_e id, const DripRxMessage<DripMainFsmState_e> *message);

    /**
     * @brief Handles state change for a flow calibrate measure request when 
     * in the flow sensor calibrate state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type FlowCalibrationMeasureRxMessage.
     */
    void handleFlowCalibrateMeasureRequestStateFlowCalibration(DripRxMessageId_e id, const DripRxMessage<DripMainFsmState_e> *message);

    /**
     * @brief Handles state change for a flow calibrate end request when 
     * in the flow sensor calibrate state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type FlowCalibrationEndRxMessage.
     */
    void handleFlowCalibrateEndRequestStateFlowCalibration(DripRxMessageId_e id, const DripRxMessage<DripMainFsmState_e> *message);

    /**
     * @brief Handles state change for a pressure calibration update request when in the listen state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type PressureCalibrateUpdateRxMessage.
     */
    void handlePressureCalibrateUpdateRequestStateListen(DripRxMessageId_e id, const DripRxMessage<DripMainFsmState_e> *message);

    /**
     * @brief Handles state change for a drain request when in the listen state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type DrainActivateRxMessage.
     */
    void handleDrainRequestStateListen(DripRxMessageId_e id, const DripRxMessage<DripMainFsmState_e> *message);

    /**
     * @brief Handles state change for a pressure poll request when in the listen state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type PressurePollRxMessage.
     */
    void handlePressurePollRequestStateListen(DripRxMessageId_e id, const DripRxMessage<DripMainFsmState_e> *message);

    /** @} */

    
private:
    DripMainFsmEventTimers_t eventTimers_;
    
    /** Managers. */
    ConfigManager configManager_;
    DataContainer dataContainer_;
    MqttManager mqttManager_;
    WifiManager wifiManager_;
    ValveManager valveManager_;
    FlowSensorManager flowSensorManager_;
    
    void preUpdate() override;
    void postUpdate() override;

    /**
     * @brief Handles all received messages according to the handler
     * functions defined in a lookup table.
     *
     * @details If a message is received that does not match an associated
     * handler function, it is ignored.
     *
     * @param[in] table The lookup table to use to find the handler functions.
     * @param[in] tableLen The length of the table.
     *
     * @retval ESP_INVALID_INPUT Returned if the lookup table is invalid.
     * @retval ESP_FAILURE Returned if an error occurred retrieving the message or handler function
     * @retval ESP_OK Returned if all messages are handled or ignored.
     */



    esp_err_t handleReceivedMessages();

    /**
     * @brief Initializes and mounts the LittleFS filesystem.
     * 
     * @details Taken from https://github.com/Ariif0/ESPIDF-WiFi-Configuration/blob/main/include/Application.h
     *
     * Registers the LittleFS driver with the Virtual File System (VFS) using the
     * base path defined in config.h, enabling file operations on the storage partition.
     */
    esp_err_t initializeFilesystem();
};

#endif