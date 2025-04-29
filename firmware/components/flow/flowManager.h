#ifndef FLOW_MANAGER_H
#define FLOW_MANAGER_H

/**
 * @brief Describes the possible states of the flow sensor.
 */
typedef enum FlowSensorStates_e {
    /** Default state. */
    FLOW_SENSOR_UNKNOWN,
    FLOW_SENSOR_IDLE,
    FLOW_SENSOR_DISPENSING,
    FLOW_SENSOR_CALIBRATION_DISPENSING,
    FLOW_SENSOR_CALIBRATION_WAITING_FOR_MEASUREMENT
} FlowSensorStates_e;

/**
 * @brief Describes the target of a current flow sensor calibration process.
 * 
 */
typedef struct FlowCalibrateTarget_t {
    float targetVolume = 0;
    uint32_t timeout = 0;
} FlowCalibrateTarget_t;

/**
 * @brief Describes the input by the user for a flow sensor calibration process.,
 * 
 */
typedef struct FlowCalibrateMeasurement_t {
    float measuredVolume = 0;
    bool conclude = 0;
};

/**
 * @brief Describes the realtime variables of a flow sensor calibration process.
 */
typedef struct FlowCalibrateProcess_t {
    uint32_t time = 0;
    uint32_t pulses = 0;
    float tankLevel = 0;
} FlowCalibrateProcess_t;

/**
 * @brief Describes a summary of the process variables for a whole flow sensor calibration process.
 */
typedef struct FlowCalibrateSummary_t {
    uint32_t pulsesPerLiter = 0;
    uint16_t calibrationPointsCount = 0;
} FlowCalibrateSummary_t;

/**
 * @brief Handles the dispensation and draining process.
 */
class FlowManager {
public:
    /**
     * @brief Constructor.
     */
    FlowManager();

    /**
     * @brief Begin the FlowManager.
     * 
     * @return esp_err_t Return code. 
     */
    esp_err_t initialize();

    /**
     * @brief Begins a calibration process.
     * 
     * @param target Target for the process.
     * @param state Overwritten with the initial state of the process.
     * @param process Overwritten with the initial process variables.
     * @return esp_err_t Return code.
     */
    esp_err_t beginCalibration(FlowCalibrateTarget_t &target, FlowSensorStates_e &state, FlowCalibrateProcess_t &process);

    /**
     * @brief Updates the calibration process.
     * 
     * @param state Overwritten with the final state.
     * @param process Overwritten with the final process variables.
     * @param summary Overwritten with the final process summary.
     * @return esp_err_t Return code.
     */
    esp_err_t loopCalibration(FlowSensorStates_e &state, FlowCalibrateProcess_t &process, FlowCalibrateSummary_t &summary);

    /**
     * @brief Accepts a measurement into the calibration process.
     * 
     * @param state Overwritten with the final state.
     * @param measurement The new measurement.
     * @param target The new target.
     * @param process Overwritten with the final process variables.
     * @return esp_err_t Return code.
     */
    esp_err_t inputCalibration(FlowSensorStates_e &state, FlowCalibrateMeasurement_t &measurement, FlowCalibrateTarget_t &target, FlowCalibrateProcess_t &process);

    /**
     * @brief Ends the calibration process.
     * 
     * @param state Overwritten with the state.
     * @param process Overwritten with the final process variables.
     * @param summary Overwritten with the final process summary.
     * @return esp_err_t Return code.
     */
    esp_err_t endCalibration(FlowSensorStates_e &state, FlowCalibrateProcess_t &process, FlowCalibrateSummary_t &summary);


private:
    FlowSensorStates_e state;
    FlowCalibrateTarget_t calibrationTarget;
    FlowCalibrateProcess_t calibrationProcess;
    FlowCalibrateSummary_t calibrationSummary;
};

#endif