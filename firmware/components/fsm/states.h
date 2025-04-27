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
    /** Puts the device to deep sleep. */
    STATE_SLEEP,
    /** Persists current config changes. */
    STATE_CONFIG,
    /** Initial dispense state. */
    STATE_DISPENSE_START,
    /** Using source for dispensation. */
    STATE_DISPENSE_SOURCE,
    /** Using tank for dispensation. */
    STATE_DISPENSE_TANK,
    /** Flow calibration dispense phase. */
    STATE_FLOW_CALIBRATE_DISPENSE,
    /** Flow calibration feedback phase. */
    STATE_FLOW_CALIBRATE_MEASURE,
    /** Pressure calibration. */
    STATE_PRESSURE_CALIBRATE,
    /** Uploads the pressure. */
    STATE_PRESSURE_POLL,
    /** Tank drain state. */
    STATE_DRAIN,

    STATE_MAX
} FsmStates_e;

#endif