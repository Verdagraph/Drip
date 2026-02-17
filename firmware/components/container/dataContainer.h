#ifndef DATA_CONTAINER_H
#define DATA_CONTAINER_H

#include "esp_err.h"

#include "config.h"
//#include "valveData.h"
//#include "flowManager.h"

/** The maximum number of characters in each log message. */
constexpr size_t DRIP_LOG_MESSAGE_BUFFER_BYTES = 512U;

/** The number of info, warning, and error logs to buffer before upload. */
constexpr size_t DRIP_LOG_MAX_INFO_LOGS = 10U;
constexpr size_t DRIP_LOG_MAX_WARNING_LOGS = 10U;
constexpr size_t DRIP_LOG_MAX_ERROR_LOGS = 10U;

/**
 * @brief Describes a type of log message.
 */
enum class DripLogType_e {
    /** @brief For routine informational messages.  */
    Info,
    /** @brief For potential errors that do not distrupt a routine. */
    Warning,
    /** @brief For failures. */
    Error,
};

/**
 * @brief Describes a log message.
 */
struct DripLog_t {
    /** @brief The type of message. */
    DripLogType_e type;
    /** @brief The return code, if any, associated with the log. */
    esp_err_t returnCode;
    /** @brief The message associated with the log. */
    char message[DRIP_LOG_MESSAGE_BUFFER_BYTES];
    /** @brief Unix timestamp the message was logged at. */
    uint32_t timestamp;
};

/**
 * @brief Indicates whether features are enabled and are functioning.
 */
struct DripDriverStatus_t {
    bool sourceDispenseValveOnline;
    bool tankDispenseValveOnline;
    bool tankDrainValveOnline;
    bool flowSensorOnline;
    bool pressureSensorOnline;
};

/**
 * @brief Measured data of the device.
 */
struct DripMeasurementData_t {
    /** @brief Current pulse count of the flow sensor. */
    uint32_t flowSensorPulses = 0U;
    /** @brief Current voltage of the pressure sensor. */
    float pressureSensorVoltage = 0.0f;
};

/**
 * @brief Data derived from the measurement data.
 */
struct DripDerivedData_t {
    /** @brief The current volume output given the flow sensor pulse count. */
    float volumeOutputLiters = 0.0f;
    /** @brief The current tank level given the tank pressure and geometry. */
    float tankLevel = 0.0f;
    /** @brief The current tank volume given the tank pressure and geometry or calibration. */
    float tankVolume = 0.0f;
    /** @brief The current tank pressure given the pressure sensor voltage. */
    float tankPressure = 0.0f;
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

    esp_err_t getConfig(DripConfig_t &config);
    esp_err_t setConfig(const DripConfig_t &config);

    esp_err_t getDriverStatus(DripDriverStatus_t &status);
    esp_err_t setDriverStatus(const DripDriverStatus_t &status);

    /**
     * Measurement data.
     */
    esp_err_t getMeasurementData(DripMeasurementData_t &data);
    esp_err_t setMeasurementData(const DripMeasurementData_t &data);
    esp_err_t getDerivedData(DripDerivedData_t &data);

    /**
     * Process data.
     */
    //esp_err_t getDispenseProcessData(VdgDispenseProcessData_t &data);
    //esp_err_t setDispenseProcessData(VdgDispenseProcessData_t data);
    //esp_err_t getDrainProcessData(VdgDrainProcessData_t &data);
    //esp_err_t setDrainProcessData(VdgDrainProcessData_t data);
    //esp_err_t getFlowCalibrationProcessData(DripFlowCalibrationProcessData_t &data);
    //esp_err_t setFlowCalibrationProcessData(DripFlowCalibrationProcessData_t data);

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
    esp_err_t logInfo(esp_err_t returnCode, const char* tag, const char *message, ...) __attribute__ ((format (printf, 4, 5)));

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
    esp_err_t logWarning(esp_err_t returnCode, const char* tag, const char *message, ...) __attribute__ ((format (printf, 4, 5)));

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
    esp_err_t logError(esp_err_t returnCode, const char* tag, const char *message, ...) __attribute__ ((format (printf, 4, 5)));

    /**
     * @brief Gets the next available log. The log is popped from the buffer 
     * and erased, so it must be uploaded after this.
     * 
     * @param[out] message Output parameter for the retrieved log.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t getNextLog(DripLog_t &message);

private:
    /** Config. */
    DripConfig_t config_;

    /** Measurement data. */
    DripMeasurementData_t measurementData_;
    DripDerivedData_t derivedData_;

    /** Process data. */
    //VdgDispenseProcessData_t dispenseProcess_;
    //VdgDrainProcessData_t drainProcess_;
    //DripFlowCalibrationProcessData_t flowCalibrationProcess_;

    /** Logging. */
    size_t numInfoLogs;
    size_t numWarningLogs;
    size_t numErrorLogs;
    DripLog_t infoLogs_[DRIP_LOG_MAX_INFO_LOGS];
    DripLog_t warningLogs_[DRIP_LOG_MAX_WARNING_LOGS];
    DripLog_t errorLogs_[DRIP_LOG_MAX_ERROR_LOGS];

    esp_err_t log(esp_err_t returnCode, const char* tag, const char *message, ...) __attribute__ ((format (printf, 4, 5)));
};

#endif