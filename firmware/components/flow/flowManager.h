#ifndef FLOW_MANAGER_H
#define FLOW_MANAGER_H

/**
 * @brief Describes the current process of the flow sensor calibration.
 */
enum VdgFlowSensorCalibrationState_e {
    VDG_FLOW_SENSOR_CALIBRATION_MIN,

    /** No calibration is ongoing. */
    VDG_FLOW_SENSOR_CALIBRATION_IDLE,
    /** The flow sensor is currently measuring flow. */
    VDG_FLOW_SENSOR_CALIBRATION_MEASURING,
    /** The flow sensor is currently waiting for a calibration feedback. */
    VDG_FLOW_SENSOR_CALIBRATION_WAITING_FOR_FEEDBACK,

    VDG_FLOW_SENSOR_CALIBRATION_MAX
};

/**
 * @brief Describes the target of a current flow sensor calibration step.
 */
struct VdgFlowCalibrationTarget_t {
    /** The target output volume in liters */
    float targetVolume = 0.0f;
    /** The maximum duration of the calibration step in miliseconds. */
    uint32_t timeout = 0U;
};

/**
 * @brief Describes the input by the user for a flow sensor calibration step.
 * 
 */
struct VdgFlowCalibrationMeasurement_t {
    /** The volume measured by the user. */
    float measuredVolume = 0.0f;
    /** If true, the calibration will conclude. */
    bool conclude = true;
};

/**
 * @brief Describes the realtime variables of a flow sensor calibration process.
 */
struct VdgFlowCalibrationProcessSlice_t {
    /** The current time of the process in miliseconds. */
    uint32_t timeTicks = 0U;
    /** The number of pulses recorded by the sensor. */
    uint32_t pulses = 0U;
    /** The tick count in the process where the last dispense process ended. */
    uint32_t timeTicksLastDispenseEnded = 0U;
};

/**
 * @brief Describes a summary of the process variables for a whole flow sensor calibration process.
 */
struct FlowCalibrationSummary_t {
    /** The new value for the number of pulses the flow sensor returns. */
    uint32_t pulsesPerLiter = 0U;
    /** The number of points used in the calibration process. */
    uint16_t calibrationPointsCount = 0U;
};

/**
 * @brief Holds all calibration process related data.
 */
struct VdgFlowCalibrationProcessData_t {
    VdgFlowCalibrationTarget_t target;
    VdgFlowCalibrationMeasurement_t measurement;
    VdgFlowCalibrationProcessSlice_t slice;
    FlowCalibrationSummary_t summary;
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
     * @param[in] target Target for the process.
     * @param[out] state Output parameter for the current state after this function.
     *
     * @return esp_err_t Return code.
     */
    esp_err_t beginCalibration(VdgFlowCalibrationTarget_t target, VdgFlowSensorCalibrationState_e &state);

    /**
     * @brief Accepts a measurement into the calibration process.
     * 
     * @param[out] state Output parameter for the current state after this function.
     * @param[in] measurement The new measurement.
     * @param process Overwritten with the final process variables.
     * @return esp_err_t Return code.
     */
    esp_err_t inputCalibration(VdgFlowSensorCalibrationState_e &state, VdgFlowCalibrationMeasurement_t measurement);

    /**
     * @brief Ends the current process.
     * 
     * @param[out] state Output parameter for the current state after this function.
     *
     * @return esp_err_t Return code.
     */
    esp_err_t endProcess(VdgFlowSensorCalibrationState_e &state);

private:
    VdgFlowSensorCalibrationState_e state;
};

#endif