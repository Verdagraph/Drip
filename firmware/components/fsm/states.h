#ifndef STATES_H
#define STATES_H

/**
 * @brief Describes possible finite-state-machine states for the whole application.
*/
enum class DripMainFsmState_e {
    /** @brief Pre-initialization. */
    Uninitialized,
    /** @brief Initialization. */
    Boot,
    /** @brief Unrecoverable error. */
    FatalError,
    /** @brief Establishing WiFi connection. */
    ConnectWifi,
    /** @brief Establishing MQTT connection. */
    ConnectMqtt,
    /** @brief WiFi provisioning. */
    Provisioning,
    /** @brief Restarts the device. */
    Restart,
    /** @brief Main idle state. */
    Listen,
    /** @brief Dispensation process. */
    Dispense,
    /** @brief Flow calibration process. */
    FlowSensorCalibrate,
    /** @brief Tank drain process. */
    Drain,
};

#endif