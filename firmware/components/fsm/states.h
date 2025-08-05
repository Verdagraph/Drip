#ifndef STATES_H
#define STATES_H

/**
 * @brief Describes possible finite-state-machine states for the whole application.
*/
enum VdgMainFsmState_e {
    VDG_MAIN_FSM_MIN,

    /** Initialization. */
    VDG_MAIN_FSM_BOOT,
    /** Unrecoverable error. */
    VDG_MAIN_FSM_FATAL_ERROR,
    /** Establishing WiFi connection. */
    VDG_MAIN_FSM_CONNECT,
    /** WiFi provisioning. */
    VDG_MAIN_FSM_PROVISIONING,
    /** Restarts the device. */
    VDG_MAIN_FSM_RESTART,
    /** Main idle state. */
    VDG_MAIN_FSM_LISTEN,
    /** Dispensation process. */
    VDG_MAIN_FSM_DISPENSE,
    /** Flow calibration process. */
    VDG_MAIN_FSM_FLOW_CALIBRATE,
    /** Pressure calibration process. */
    VDG_MAIN_FSM_PRESSURE_CALIBRATE,
    /** Tank drain process. */
    VDG_MAIN_FSM_DRAIN,

    VDG_MAIN_FSM_MAX
};

#endif