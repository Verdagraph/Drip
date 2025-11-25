#ifndef MESSAGE_IDS_H
#define MESSAGE_IDS_H

/**
 * @brief Set of incoming messages.
 */
enum class DripRxMessageId_e {
    DispenseActivate,
    Deactivate,
    Restart,
    ChangeConfig,
    FlowCalibrate,
    PressureCalibrate,
    DrainActivate,
    PollPressure,
};

/**
 * @brief Set of outgoing messages.
 */
enum class DripTxMessageId_e {
    DispenseSlice,
    DispenseSummary,
    Log,
    ReadConfig,
    DrainSummary,
    PressureReport
};

#endif