#ifndef MESSAGE_IDS_H
#define MESSAGE_IDS_H

/**
 * @brief Set of incoming messages.
 */
enum class DripRxMessageId_e {
    DispenseActivate,
    Deactivate,
    Restart,
    ConfigUpdate,
    FlowCalibrate,
    PressureCalibrate,
    DrainActivate,
    PressurePoll,
};

/**
 * @brief Set of outgoing messages.
 */
enum class DripTxMessageId_e {
    DispenseSlice,
    DispenseSummary,
    Log,
    ConfigRead,
    DrainSummary,
    PressureReport
};

#endif