#ifndef CONNECTION_STATES_H
#define CONNECTION_STATES_H

/**
 * @brief Describes possible finite-state-machine states for the ConnectionController.
*/
enum class DripConnectionControllerState_e {
    Unknown = 0U,
    /** @brief Pre-initialization. */
    Uninitialized,
    /** @brief Unrecoverable error. */
    FatalError,
    /** @brief WiFi and MQTT provisioning. */
    Provisioning,
    /** @brief Disconnected idle. */
    Disconnected,
    /** @brief Begin connection routine. */
    ConnectBegin,
    /** @brief Establishing WiFi connection. */
    ConnectWifi,
    /** @brief Establishing MQTT connection. */
    ConnectMqtt,
    /** @brief */
    Connected
};

#endif