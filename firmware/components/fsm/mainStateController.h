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

/**
 * @brief Holds timing information.
 */
struct VdgMainFsmEventTimers_t
{
    /** The timestamp of the last time a process slice was uploaded. */
    uint32_t lastProcessSliceUploadTicks;
};

/**
 * @brief Defines main application routines.
 */

class MainStateController : public StateController<DripMainFsmState_e, DripRxMessageId_e, DripRxMessage, MainStateController> {
public:
    /**
     * @brief Constructor.
     */
    MainStateController(const DataContainer &dataContainer);

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
     * @brief Handlers for state connect.
     */
    void connectEntry();
    void connectUpdate();
    void connectExit();

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
     * @param[in] message MQTT received message of type DispenseActivateRxMessage.
     */
    void handleDispenseRequestStateListen(DripRxMessageId_e id, const DripRxMessage &message);

    /**
     * @brief Handles state change for a deactivation request in the listen state.
     *
     * @param[in] message MQTT received message of type DeactivateRxMessage.
     */
    void handleDeactivateRequestStateListen(DripRxMessageId_e id, const DripRxMessage &message);

    /**
     * @brief Handles state change for a restart request in the listen state.
     *
     * @param[in] message MQTT received message of type RestartRxMessage.
     */
    void handleRestartRequestStateListen(DripRxMessageId_e id, const DripRxMessage &message);

    /**
     * @brief Handles state change for a config change request in the listen state.
     *
     * @param[in] message MQTT received message of type ConfigUpdateRxMessage.
     */
    void handleConfigChangeRequestStateListen(DripRxMessageId_e id, const DripRxMessage &message);

    /**
     * @brief Handles state change for a flow calibrate request when in the listen state.
     *
     * @param[in] message MQTT received message of type FlowCalibrationUpdateRxMessage.
     */
    void handleFlowCalibrateRequestStateListen(DripRxMessageId_e id, const DripRxMessage &message);

    /**
     * @brief Handles state change for a flow calibrate request when 
     * in the flow sensor calibrate state.
     *
     * @param[in] message MQTT received message of type FlowCalibrationUpdateRxMessage.
     */
    void handleFlowCalibrateRequestStateFlowCalibration(DripRxMessageId_e id, const DripRxMessage &message);

    /**
     * @brief Handles state change for a pressure calibrate request when in the listen state.
     *
     * @param[in] message MQTT received message of type PressureCalibrateRxMessage.
     */
    void handlePressureCalibrateRequestStateListen(DripRxMessageId_e id, const DripRxMessage &message);

    /**
     * @brief Handles state change for a drain request when in the listen state.
     *
     * @param[in] message MQTT received message of type DrainActivateRxMessage.
     */
    void handleDrainRequestStateListen(DripRxMessageId_e id, const DripRxMessage &message);

    /**
     * @brief Handles state change for a pressure poll request when in the listen state.
     *
     * @param[in] message MQTT received message of type PressurePollRxMessage.
     */
    void handlePressurePollRequestStateListen(DripRxMessageId_e id, const DripRxMessage &message);

    /** @} */

    
private:
    VdgMainFsmEventTimers_t eventTimers_;
    
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
    
    esp_err_t initializeFilesystem();
};

#endif