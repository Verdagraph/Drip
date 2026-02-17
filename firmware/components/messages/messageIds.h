#ifndef MESSAGE_IDS_H
#define MESSAGE_IDS_H

/**
 * @brief Set of incoming messages.
 */
enum class DripRxMessageId_e {
    Unknown = 0U,
    DispenseActivate,
    Deactivate,
    Restart,
    ConfigUpdate,
    FlowCalibrateBegin,
    FlowCalibrateDispense,
    FlowCalibrateMeasure,
    FlowCalibrateEnd,
    DrainActivate,
    PressureCalibrateUpdate,
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