/**
 * @file connectionController.h
 * @author Nathaniel King
 * @brief Controls connection to WiFi and MQTT and MQTT traffic.
 * @date 2026-02-21
 */

#ifndef CONNECTION_CONTROLLER_H
#define CONNECTION_CONTROLLER_H

#include <etl/const_map.h>

#include "stateController.h"

#include "connectionStates.h"

/**
 * @brief Defines network connection routines.
 */

class ConnectionController : public StateController<DripConnectionControllerState_e, NULL, NULL> {
public:
    /**
     * @brief Constructor.
     */
    ConnectionController(DataContainer &dataContainer);

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
     * @brief Handlers for state fatal error.
     */
    void fatalErrorEntry();
    void fatalErrorUpdate();
    void fatalErrorExit();

    /**
     * @brief Handlers for state provisioning.
     */
    void provisioningEntry();
    void provisioningUpdate();
    void provisioningExit();

    /**
     * @brief Handlers for state disconnected.
     */
    void disconnectedBeginEntry();
    void disconnectedBeginUpdate();
    void disconnectedBeginExit();

    /**
     * @brief Handlers for state connect begin.
     */
    void connectBeginEntry();
    void connectBeginUpdate();
    void connectBeginExit();

    /**
     * @brief Handlers for state connect Wifi.
     */
    void connectWifiEntry();
    void connectWifiUpdate();
    void connectWifiExit();

    /**
     * @brief Handlers for state connect MQTT.
     */
    void connectMqttEntry();
    void connectMqttUpdate();
    void connectMqttExit();

    /**
     * @brief Handlers for state connected.
     */
    void connectedEntry();
    void connectedUpdate();
    void connectedExit();

    /** @} */
};

#endif
