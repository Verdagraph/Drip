#ifndef STATES_H
#define STATES_H

/**
 * @brief Describes possible finite-state-machine states for the whole application.
*/
enum DripMainFsmState_e {
    DRIP_MAIN_FSM_MIN,

    /** @brief Initialization. */
    DRIP_MAIN_FSM_BOOT,
    /** @brief Unrecoverable error. */
    DRIP_MAIN_FSM_FATAL_ERROR,
    /** @brief Establishing WiFi connection. */
    DRIP_MAIN_FSM_CONNECT,
    /** @brief WiFi provisioning. */
    DRIP_MAIN_FSM_PROVISIONING,
    /** @brief Restarts the device. */
    DRIP_MAIN_FSM_RESTART,
    /** @brief Main idle state. */
    DRIP_MAIN_FSM_LISTEN,
    /** @brief Dispensation process. */
    DRIP_MAIN_FSM_DISPENSE,
    /** @brief Flow calibration process. */
    DRIP_MAIN_FSM_FLOW_CALIBRATE,
    /** @brief Pressure calibration process. */
    DRIP_MAIN_FSM_PRESSURE_CALIBRATE,
    /** @brief Tank drain process. */
    DRIP_MAIN_FSM_DRAIN,

    DRIP_MAIN_FSM_MAX
};

#endif