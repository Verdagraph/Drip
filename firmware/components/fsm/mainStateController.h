#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

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
    MainStateController();

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
     * @param[in] message MQTT received message.
     * @return esp_err_t Return code.
     */
    void handleDispenseRequestStateListen(DispenseActivateRxMessage &message);

    /**
     * @brief Handles state change for a deactivation request in the listen state.
     *
     * @param[in] message MQTT received message.
     * @return esp_err_t Return code.
     */
    void handleDeactivateRequestStateListen(DeactivateRxMessage message);

    /**
     * @brief Handles state change for a restart request in the listen state.
     *
     * @param[in] message MQTT received message.
     */
    void handleRestartRequestStateListen(RestartRxMessage message);

    /**
     * @brief Handles state change for a config change request in the listen state.
     *
     * @param[in] message MQTT received message.
     */
    void handleConfigChangeRequestStateListen(ConfigUpdateRxMessage message);

    /**
     * @brief Handles state change for a flow calibrate request when in the listen state.
     *
     * @param[in] message MQTT received message.
     */
    void handleFlowCalibrateRequestStateListen(FlowCalibrationUpdateRxMessage message);

    /**
     * @brief Handles state change for a flow calibrate request when 
     * in the flow sensor calibrate state.
     *
     * @param[in] message MQTT received message.
     */
    void handleFlowCalibrateRequestStateFlowCalibration(FlowCalibrationUpdateRxMessage message);

    /**
     * @brief Handles state change for a pressure calibrate request when in the listen state.
     *
     * @param[in] message MQTT received message.
     */
    void handlePressureCalibrateRequestStateListen(PressureCalibrateRxMessage message);

    /**
     * @brief Handles state change for a drain request when in the listen state.
     *
     * @param[in] message MQTT received message.
     */
    void handleDrainRequestStateListen(DrainActivateRxMessage message);

    /**
     * @brief Handles state change for a pressure poll request when in the listen state.
     *
     * @param[in] message MQTT received message.
     */
    void handlePressurePollRequestStateListen(PressurePollRxMessage message);

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