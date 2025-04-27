#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "states.h"
#include "configManager.h"
#include "mqttManager.h"
#include "connectionManager.h"

/**
 * @brief Defines main application routines and transistions between states.
 */
class StateManager {
public:
    /**
     * @brief Constructor.
     */
    StateManager(ConfigManager *configManager, MqttManager *mqttManager, ConnectionManager *connectionManager);

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
     * @brief Handler for state STATE_SLEEP.
     */
    void sleep();

    /**
     * @brief Handler for state STATE_CONFIG.
     */
    void config();

    /**
     * @brief Handler for state STATE_DISPENSE_START.
     */
    void dispense_start();

    /**
     * @brief Handler for state STATE_DISPENSE_SOURCE.
     */
    void dispense_source();

    /**
     * @brief Handler for state STATE_DISPENSE_TANK.
     */
    void dispense_tank();

    /**
     * @brief Handler for state STATE_FLOW_CALIBRATE_DISPENSE.
     */
    void flow_calibrate_dispense();

    /**
     * @brief Handler for state STATE_FLOW_CALIBRATE_MEASURE.
     */
    void flow_calibrate_measure();

    /**
     * @brief Handler for state STATE_PRESSURE_CALIBRATE.
     */
    void pressure_calibrate();

    /**
     * @brief Handler for state STATE_PRESSURE_POLL.
     */
    void pressure_poll();

    /**
     * @brief Handler for state STATE_DRAIN.
     */
    void drain();
};

#endif