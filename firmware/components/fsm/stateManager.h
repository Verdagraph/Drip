#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "states.h"
#include "configManager.h"
#include "mqttManager.h"
#include "connectionManager.h"
#include "valveManager.h"

/**
 * @brief Defines main application routines and transistions between states.
 */
class StateManager {
public:
    /**
     * @brief Constructor.
     */
    StateManager(
        ConfigManager *configManager, 
        MqttManager *mqttManager, 
        ConnectionManager *connectionManager, 
        ValveManager *valveManager
    );

    /**
     * @brief Begins the finite state machine.
     */
    void initialize();

    /**
     * @brief Executes the current state.
     */
    void handle_current_state();
    
private:
    /** Current state. */
    FsmStates_e state;

    /** Managers. */
    ConfigManager *configManager;
    MqttManager *mqttManager;
    ConnectionManager *connectionManager;
    ValveManager *valveManager;

    /** State handlers. */

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

    /** Received MQTT message handlers. */

    /**
     * @brief Handles state change for a dispense request.
     * 
     * @param message MQTT received message.
     * @return esp_err_t Return code.
     */
    esp_err_t handleDispenseRequest(MqttRxMessage_t *message);

    /**
     * @brief Handles state change for a config change request.
     * 
     * @param message MQTT received message.
     * @return esp_err_t Return code.
     */
    esp_err_t handleConfigChangeRequest(MqttRxMessage_t *message);

    /**
     * @brief Handles state change for a flow calibrate request.
     * 
     * @param message MQTT received message.
     * @return esp_err_t Return code.
     */
    esp_err_t handleFlowCalibrateRequest(MqttRxMessage_t *message);

    /**
     * @brief Handles state change for a pressure calibrate request.
     * 
     * @param message MQTT received message.
     * @return esp_err_t Return code.
     */
    esp_err_t handlePressureCalibrateRequest(MqttRxMessage_t *message);

    /**
     * @brief Handles state change for a drain request.
     * 
     * @param message MQTT received message.
     * @return esp_err_t Return code.
     */
    esp_err_t handleDrainRequest(MqttRxMessage_t *message);

    /**
     * @brief Handles state change for a pressure poll request.
     * 
     * @param message MQTT received message.
     * @return esp_err_t Return code.
     */
    esp_err_t handlePressurePollRequest(MqttRxMessage_t *message);
};

#endif