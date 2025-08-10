#ifndef DATA_CONTAINER_H
#define DATA_CONTAINER_H

#include "esp_err.h"
#include "valveManager.h"
#include "flowManager.h"

/** The maximum number of characters in each log message. */
#define VDG_LOG_MESSAGE_BUFFER_BYTES 128U

/** The number of info, warning, and error logs to buffer before upload. */
#define VDG_LOG_MAX_INFO_LOGS 5U
#define VDG_LOG_MAX_WARNING_LOGS 5U
#define VDG_LOG_MAX_ERROR_LOGS 5U

/**
 * @brief Describes a type of log message.
 */
enum VdgLogMessageType_e {
    /** For routine informational messages.  */
    VDG_LOG_INFO,
    /** For potential errors that do not distrupt a routine. */
    VDG_LOG_WARNING,
    /** For failures. */
    VDG_LOG_ERROR,
};

/**
 * @brief Describes a log message.
 */
struct VdgLogMessage_t {
    /** The type of message. */
    VdgLogMessageType_e type;
    /** The return code, if any, associated with the log. */
    esp_err_t returnCode;
    /** The message associated with the log. */
    char message[VDG_LOG_MESSAGE_BUFFER_BYTES];
    /** Unix timestamp the message was logged at. */
    uint32_t timestamp;
};

struct VdgMeasurementData_t {
    /** Raw measurement data. */
    float flowSensorPulses;
    float pressureSensorVoltage;

    /** Derived data. */
    float volumeOutput;
    float tankPressure;
    float tankLevel;
    float tankVolume;
};

/**
 * @brief Stores state common to multiple managers.
 */
class DataContainer {
public:
    /**
     * @brief Constructor.
     */
    DataContainer();

    /**
     * @brief Begin the DataContainer.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t initialize();

    /**
     * Measurement data.
     */
    esp_err_t getMeasurementData(VdgMeasurementData_t &data);
    esp_err_t getDerivedMeasurementData(VdgMeasurementData_t &data);
    esp_err_t setPressureSensorVoltage(float voltage);
    esp_err_t incrementFlowSensorPulses();
    esp_err_t clearFlowSensorPulses();

    /**
     * Process data.
     */
    esp_err_t getDispenseProcessData(VdgDispenseProcessData_t &data);
    esp_err_t setDispenseProcessData(VdgDispenseProcessData_t data);
    esp_err_t getDrainProcessData(VdgDispenseProcessData_t &data);
    esp_err_t setDrainProcessData(VdgDispenseProcessData_t data);
    esp_err_t getFlowCalibrationProcessData(VdgFlowCalibrationProcessData_t &data);
    esp_err_t setFlowCalibrationProcessData(VdgFlowCalibrationProcessData_t data);

    /**
     * Logging.
     */

    /**
     * @brief Retrieves the number of stored logs.
     * 
     * @return size_t The number of stored logs. 
     */
    size_t getNumLogs();

    /**
     * @brief Logs an informational log.
     * 
     * @param[in] returnCode The return code, if any, associated with the log.
     * Defaults to ESP_OK;
     * @param[in] tag The ESP tag used for debug logging.
     * @param[in] message The log message.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t logInfo(esp_err_t returnCode, const char* tag, const char *message);

    /**
     * @brief Logs a warning log.
     * 
     * @param[in] returnCode The return code, if any, associated with the log.
     * Defaults to ESP_OK;
     * @param[in] tag The ESP tag used for debug logging.
     * @param[in] message The log message.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t logWarning(esp_err_t returnCode, const char* tag, const char *message);

    /**
     * @brief Logs an error log.
     * 
     * @param[in] returnCode The return code, if any, associated with the log.
     * Defaults to ESP_FAIL;
     * @param[in] tag The ESP tag used for debug logging.
     * @param[in] message The log message.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t logError(esp_err_t returnCode, const char* tag, const char *message);

    /**
     * @brief Gets the next available log. The log is popped from the buffer 
     * and erased, so it must be uploaded after this.
     * 
     * @param[out] message Output parameter for the retrieved log.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t getNextLog(VdgLogMessage_t &message);




private:

    /** Measurement data. */
    VdgMeasurementData_t measurementData_;

    /** Process data. */
    VdgDispenseProcessData_t dispenseProcess_;
    VdgDrainProcessData_t drainProcess_;
    VdgFlowCalibrationProcessData_t flowCalibrationProcess_;

    /** Logging. */
    size_t numInfoLogs;
    size_t numWarningLogs;
    size_t numErrorLogs;
    VdgLogMessage_t infoLogs_[VDG_LOG_MAX_INFO_LOGS];
    VdgLogMessage_t warningLogs_[VDG_LOG_MAX_WARNING_LOGS];
    VdgLogMessage_t errorLogs_[VDG_LOG_MAX_ERROR_LOGS];
};

#endif