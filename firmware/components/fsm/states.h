#ifndef STATES_H
#define STATES_H

/**
 * @brief Describes possible finite-state-machine states.
*/
typedef enum FsmStates_e {
    STATE_MIN,

    /** Initialization. */
    STATE_BOOT,
    /** Unrecoverable error. */
    STATE_FATAL_ERROR,
    /** Establishing WiFi connection. */
    STATE_CONNECT,
    /** WiFi provisioning. */
    STATE_PROVISIONING,
    /** Restarts the device. */
    STATE_RESTART,
    /** Main idle state. */
    STATE_LISTEN,
    /** Dispensation process. */
    STATE_DISPENSE,
    /** Flow calibration process. */
    STATE_FLOW_CALIBRATE,
    /** Pressure calibration process. */
    STATE_PRESSURE_CALIBRATE,
    /** Tank drain process. */
    STATE_DRAIN,

    STATE_MAX
} FsmStates_e;

#endif