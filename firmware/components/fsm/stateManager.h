#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "states.h"
#include "config.h"
#include "configManager.h"
#include "messages.h"
#include "configManager.h"
#include "dataContainer.h"
#include "mqttManager.h"
#include "connectionManager.h"
#include "valveManager.h"
#include "flowManager.h"

/**
 * Defines the default time in miliseconds in between uploading slices of a process.
 */
#define PROCESS_SLICE_UPLOAD_MS_DEFAULT 100U

/**
 * Forward declarations.
 */
class StateManager;

/**
 * @brief Signature common to all incoming message handler functions.
 * 
 * @param message The message to handle
 */
typedef void (StateManager::*VdgMessageHandleFunc_t)(VdgMessage_t message);

/**
 * @brief Holds timing information.
 */
struct VdgMainFsmEventTimers_t {
    /** The timestamp of the last time a process slice was uploaded. */
    uint32_t lastProcessSliceUploadTicks;
};

/**
 * @brief Describes an entry in a lookup table 
 * mapping message IDs to handler functions
 */
struct VdgMessageHandleFuncTable_t {
    VdgMessageId_t id;
    VdgMessageHandleFunc_t handlerFunction;
};

/**
 * @brief Defines main application routines and transistions between states.
 */
class StateManager {
public:
    /**
     * @brief Constructor.
     */
    StateManager();

    /**
     * @brief Begins the finite state machine.
     */
    void initialize();

    /**
     * @brief Updates the FSM.
     */
    void update();

    /** 
     * State handlers. 
     */
    
    /**
     * @brief Handler for state STATE_BOOT.
     */
    void boot();
    
    /**
     * @brief Handler for state STATE_FATAL_ERROR.
     */
    void fatalError();
    
    /**
     * @brief Handler for state STATE_CONNECT.
     */
    void connect();
    
    /**
     * @brief Handler for state STATE_PROVISIONING.
     */
    void accessPoint();
    
    /**
     * @brief Handler for state STATE_RESTART.
     */
    void restart();
    
    /**
     * @brief Handler for state STATE_LISTEN.
     */
    void listen();
    
    /**
     * @brief Handler for state STATE_DISPENSE.
     */
    void dispense();
    
    /**
     * @brief Handler for state STATE_FLOW_CALIBRATE.
     */
    void flowCalibrate();
    
    /**
     * @brief Handler for state STATE_PRESSURE_CALIBRATE.
     */
    void pressureCalibrate();
    
    /**
     * @brief Handler for state STATE_DRAIN.
     */
    void drain();
    
    /** 
     * Received MQTT message handlers. 
     */
    
    /**
     * @brief Handles state change for a dispense request.
     * 
     * @param[in] message MQTT received message.
     * @return esp_err_t Return code.
     */
    void handleDispenseRequest(VdgMessage_t message);
    
    /**
     * @brief Handles state change for a deactivation request.
     * 
     * @param[in] message MQTT received message.
     * @return esp_err_t Return code.
     */
    void handleDeactivateRequest(VdgMessage_t message);
    
    /**
     * @brief Handles state change for a restart request.
     * 
     * @param[in] message MQTT received message.
     */
    void handleRestartRequest(VdgMessage_t message);
    
    /**
     * @brief Handles state change for a flow calibrate request
     * when in the state VDG_MAIN_FSM_LISTEN.
     * 
     * @param[in] message MQTT received message.
     */
    void handleFlowCalibrateRequestListen(VdgMessage_t message);
    
    /**
     * @brief Handles state change for a flow calibrate request
     * when in the state VDG_MAIN_FSM_FLOW_CALIBRATE.
     * 
     * @param[in] message MQTT received message.
     */
    void handleFlowCalibrateRequestFlowCalibration(VdgMessage_t message);
    
    /**
     * @brief Handles state change for a pressure calibrate request.
     * 
     * @param[in] message MQTT received message.
     */
    void handlePressureCalibrateRequest(VdgMessage_t message);
    
    /**
     * @brief Handles XC state change for a drain request.
     * 
     * @param[in] message MQTT received message.
     */
    void handleDrainRequest(VdgMessage_t message);
    
    /**
     * @brief Handles state change for a pressure poll request.
     * 
     * @param[in] message MQTT received message.
     */
    void handlePressurePollRequest(VdgMessage_t message);
    
    /**
     * @brief Handles state change for a config change request.
     * 
     * @param[in] message MQTT received message.
     */
    void handleConfigChangeRequest(VdgMessage_t message);

private:
    /** Current state. */
    VdgMainFsmState_e state_;
    VdgMainFsmEventTimers_t eventTimers_;

    /** Managers. */
    DataContainer dataContainer_;
    ConfigManager configManager_;
    MqttManager mqttManager_;
    ConnectionManager connectionManager_;
    ValveManager valveManager_;
    FlowSensorManager flowSensorManager_;

    const static VdgMessageHandleFuncTable_t vdgMessageHandleFuncTableListenState[];
    const static VdgMessageHandleFuncTable_t vdgMessageHandleFuncTableDispenseState[];
    const static VdgMessageHandleFuncTable_t vdgMessageHandleFuncTableFlowCalibrateState[];
    const static VdgMessageHandleFuncTable_t vdgMessageHandleFuncTablePressureCalibrateState[];
    const static VdgMessageHandleFuncTable_t vdgMessageHandleFuncTableDrainState[];

    esp_err_t getHandlerFunctionFromMessageId(const VdgMessageHandleFuncTable_t *table, VdgMessageId_t id, VdgMessageHandleFunc_t &handlerFunction);

    /**
     * @brief Executes the current state.
     */
    void handleCurrentState();

    /**
     * @brief Handles all received messages according to the handler
     * functions defined in a lookup table.
     * 
     * @details If a message is received that does not match an associated
     * handler function, it is ignored.
     * 
     * @param[in] table The lookup table to use to find the handler functions.
     * 
     * @retval ESP_INVALID_INPUT Returned if the lookup table is invalid.
     * @retval ESP_FAILURE Returned if an error occurred retrieving the message or handler function
     * @retval ESP_OK Returned if all messages are handled or ignored.
     */
    esp_err_t handleReceivedMessages(const VdgMessageHandleFuncTable_t *table);

    esp_err_t initializeFilesystem();

};

#endif