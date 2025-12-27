#ifndef FLOW_MANAGER_H
#define FLOW_MANAGER_H

/**
 * @brief Describes the current process of the flow sensor calibration.
 */
enum DripFlowSensorCalibrationState_e {
    DRIP_FLOW_SENSOR_CALIBRATION_MIN,

    /** @brief No calibration is ongoing. */
    DRIP_FLOW_SENSOR_CALIBRATION_IDLE,
    /** @brief The flow sensor is currently measuring flow. */
    DRIP_FLOW_SENSOR_CALIBRATION_MEASURING,
    /** @brief The flow sensor is currently waiting for a calibration feedback. */
    DRIP_FLOW_SENSOR_CALIBRATION_WAITING_FOR_FEEDBACK,
    DRIP_FLOW_SENSOR_CALIBRATION_TIMEOUT,

    DRIP_FLOW_SENSOR_CALIBRATION_MAX
};

/**
 * @brief Describes a target for one calibration step.
 */
struct DripFlowCalibrationTarget_t {
    /** @brief The target volume of this calibration step in liters. */
    float targetVolume;
    /** @brief The timeout of this calibration step in miliseconds. */
    uint32_t timeoutMs;
};

/**
 * @brief Describes the input by the user for a flow sensor calibration step.
 * 
 */
struct DripFlowCalibrationMeasurement_t {
    /** @brief The volume measured by the user. */
    float measuredVolume = 0.0f;
    /** @brief If true, the calibration will conclude. */
    bool conclude = true;
};

/**
 * @brief Describes the realtime variables of a flow sensor calibration process.
 */
struct DripFlowCalibrationProcessSlice_t {
    /** @brief The current time of the process in miliseconds. */
    uint32_t timeTicks = 0U;
    /** @brief The number of pulses recorded by the sensor. */
    uint32_t pulses = 0U;
    /** @brief The tick count in the process where the last dispense process ended. */
    uint32_t timeTicksLastDispenseEnded = 0U;
};

/**
 * @brief Describes a summary of the process variables for a whole flow sensor calibration process.
 */
struct DripFlowCalibrationSummary_t {
    /** @brief The new value for the number of pulses the flow sensor returns. */
    uint32_t pulsesPerLiter = 0U;
    /** @brief The number of points used in the calibration process. */
    uint16_t calibrationPointsCount = 0U;
};

/**
 * @brief Holds all calibration process related data.
 */
struct DripFlowCalibrationProcessData_t {
    DripFlowCalibrationMeasurement_t measurement;
    DripFlowCalibrationProcessSlice_t slice;
    DripFlowCalibrationSummary_t summary;
};

/**
 * @brief Handles the calibration of the flow sensor.
 */
class FlowSensorManager {
public:
    /**
     * @brief Constructor.
     */
    FlowSensorManager();

    /**
     * @brief Begin the FlowSensorManager.
     * 
     * @return esp_err_t Return code. 
     */
    esp_err_t initialize();

    /**
     * @brief Begins a calibration process.
     * 
     * @param[out] state Output parameter for the current state after this function.
     *
     * @return esp_err_t Return code.
     */
    esp_err_t beginCalibration(DripFlowSensorCalibrationState_e &state);

    /**
     * @brief Accepts a measurement into the calibration process.
     * 
     * @param[out] state Output parameter for the current state after this function.
     * @param[in] measurement The new measurement.
     * @param process Overwritten with the final process variables.
     * @return esp_err_t Return code.
     */
    esp_err_t inputCalibration(DripFlowSensorCalibrationState_e &state, DripFlowCalibrationMeasurement_t measurement);

    /**
     * @brief Ends the current process.
     * 
     * @param[out] state Output parameter for the current state after this function.
     *
     * @return esp_err_t Return code.
     */
    esp_err_t endProcess(DripFlowSensorCalibrationState_e &state);

private:
    DripFlowSensorCalibrationState_e state;
};

#endif