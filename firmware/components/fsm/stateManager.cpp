#include "cstdio"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"

#include "messages.h"
#include "utils.h"
#include "dataContainer.h"
#include "configManager.h"
#include "mqttManager.h"
#include "wifiManager.h"
#include "valveManager.h"
#include "flowManager.h"

#include "stateManager.h"

static const char* TAG = "StateManager";

/**
 * @brief Lookup table mapping received messages to handler functions
 * for messages received in the listen state.
 */
constexpr VdgMessageHandleFuncTable_t StateManager::vdgMessageHandleFuncTableListenState[] = {
    {VDG_MSG_RX_DISPENSE_ACTIVATE, &StateManager::handleDispenseRequest},
    {VDG_MSG_RX_RESTART, &StateManager::handleRestartRequest},
    {VDG_MSG_RX_CHANGE_CONFIG, &StateManager::handleConfigChangeRequest},
    {VDG_MSG_RX_FLOW_CALIBRATE, &StateManager::handleFlowCalibrateRequestListen},
    {VDG_MSG_RX_PRESSURE_CALIBRATE, &StateManager::handlePressureCalibrateRequest},
    {VDG_MSG_RX_DRAIN, &StateManager::handleDrainRequest},
    {VDG_MSG_RX_PRESSURE_POLL, &StateManager::handlePressurePollRequest},
    {VDG_MSG_RX_MAX, nullptr}
};

/**
 * @brief Lookup table mapping received messages to handler functions
 * for messages received in the dispense state.
 */
constexpr VdgMessageHandleFuncTable_t StateManager::vdgMessageHandleFuncTableDispenseState[] = {
    {VDG_MSG_RX_DEACTIVATE, &StateManager::handleDeactivateRequest},
    {VDG_MSG_RX_MAX, nullptr}
};

/**
 * @brief Lookup table mapping received messages to handler functions
 * for messages received in the flow sensor calibration state.
 */
constexpr VdgMessageHandleFuncTable_t StateManager::vdgMessageHandleFuncTableFlowCalibrateState[] = {
    {VDG_MSG_RX_DEACTIVATE, &StateManager::handleDeactivateRequest},
    {VDG_MSG_RX_FLOW_CALIBRATE, &StateManager::handleFlowCalibrateRequestFlowCalibration},
    {VDG_MSG_RX_MAX, nullptr}
};

/**
 * @brief Lookup table mapping received messages to handler functions
 * for messages received in the pressure sensor calibration state.
 */
constexpr VdgMessageHandleFuncTable_t StateManager::vdgMessageHandleFuncTablePressureCalibrateState[] = {
    {VDG_MSG_RX_DEACTIVATE, &StateManager::handleDeactivateRequest},
    {VDG_MSG_RX_PRESSURE_CALIBRATE, &StateManager::handlePressureCalibrateRequest},
    {VDG_MSG_RX_MAX, nullptr}
};

/**
 * @brief Lookup table mapping received messages to handler functions
 * for messages received in the drain state.
 */
constexpr VdgMessageHandleFuncTable_t StateManager::vdgMessageHandleFuncTableDrainState[] = {
    {VDG_MSG_RX_DEACTIVATE, &StateManager::handleDeactivateRequest},
    {VDG_MSG_RX_MAX, nullptr}
};

/**
 * @brief Retrieves the handler function for a message ID.
 * 
 * @param[in] table The lookup table to use to find the handler function.
 * The final entry in the table must have the ID of VDG_MSG_RX_MAX.
 * @param[in] id The message ID.
 * @param[out] handlerFunction Overwritten with a pointer to the handler function.
 * 
 * @retval ESP_INVALID_INPUT Returned if the lookup table is invalid.
 * @retval ESP_OK Returned regardless of if a handler function is found or not.  
 */
esp_err_t StateManager::getHandlerFunctionFromMessageId(
    const VdgMessageHandleFuncTable_t *table, 
    VdgMessageId_t id, 
    VdgMessageHandleFunc_t &handlerFunction) {
    size_t index = 0U;

    /** Validate table. */
    if (table == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }
    
    handlerFunction = nullptr;

    while (table[index].id != VDG_MSG_RX_MAX) {
        if (table[index].id == id) {
            handlerFunction = table[index].handlerFunction;
            return ESP_OK;
        }

        index++;
    }

    return ESP_OK;
}


/**
 * @brief Constructor
 */
StateManager::StateManager() : 
state_(VDG_MAIN_FSM_MIN),
eventTimers_(),
configManager_(),
dataContainer_(),
mqttManager_(),
wifiManager_(),
valveManager_(configManager_, dataContainer_),
flowSensorManager_()
{
}

/**
 * @brief Initializes the finite state machine.
 */
void StateManager::initialize() {
    state_ = VDG_MAIN_FSM_BOOT;
}

/**
 * @brief Updates the FSM.
 */
void StateManager::update() {
    esp_err_t err = ESP_OK;

    handleCurrentState();

    /** Upload all logs to MQTT. */
    err = mqttManager_.uploadLogs();
    if (err != ESP_OK) {
        dataContainer_.logError(err, TAG, "Failed to upload logs.");
    }
}

/**
 * @brief Executes the current state.
 */
void StateManager::handleCurrentState() {
    switch (state_) {
        case VDG_MAIN_FSM_BOOT:
            boot();
            break;
        case VDG_MAIN_FSM_FATAL_ERROR:
            fatalError();
            break;
        case VDG_MAIN_FSM_CONNECT:
            connect();
            break;
        case VDG_MAIN_FSM_PROVISIONING:
            accessPoint();
            break;
        case VDG_MAIN_FSM_RESTART:
            restart();
            break;
        case VDG_MAIN_FSM_LISTEN:
            listen();
            break;
        case VDG_MAIN_FSM_DISPENSE:
            dispense();
            break;
        case VDG_MAIN_FSM_FLOW_CALIBRATE:
            flowCalibrate();
            break;
        case VDG_MAIN_FSM_PRESSURE_CALIBRATE:
            pressureCalibrate();
            break;
        case VDG_MAIN_FSM_DRAIN:
            drain();
            break;
        default:
            dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "State machine set to invalid state.");
            state_ = VDG_MAIN_FSM_FATAL_ERROR;
            break;
    }
}

/**
 * @brief Handles all received messages according to the handler
 * functions defined in a lookup table.
 * 
 * @details If a message is received that does not match an associated
 * handler function, it is ignored.
 * 
 * @param[in] table The lookup table to use to find the handler functions.
 * 
 * @retval ESP_INVALID_INPUT Returned if the lookup table is invalid.
 * @retval ESP_FAILURE Returned if an error occurred retrieving the message or handler function
 * @retval ESP_OK Returned if all messages are handled or ignored.
 */
esp_err_t StateManager::handleReceivedMessages(const VdgMessageHandleFuncTable_t *table) {
    VdgMessage_t message = {};
    VdgMessageHandleFunc_t handlerFunction = nullptr;
    esp_err_t err = ESP_OK;

    /** Validate table input. */
    if (table == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }

    /** Check for new MQTT messages. */
    while(mqttManager_.numMessagesInQueue() > 0) {

        /** Get the next message from the queue. */
        err = mqttManager_.getNextMessage(message);
        if (err != ESP_OK) {
            dataContainer_.logError(err, TAG, "Failed to retrieve MQTT message.");
            return ESP_FAIL;
        }

        /** Retrieve handler function. */
        err = getHandlerFunctionFromMessageId(table, message.id, handlerFunction);
        if (err != ESP_OK) {
            dataContainer_.logError(err, TAG, "Failed to retrieve message handler function.");
            return ESP_FAIL;
        }

        /** Ignore messages without a handler function. */
        if (handlerFunction == nullptr) {
            continue;
        }

        /** Execute handler function */
        (this->*handlerFunction)(message);    
    }

    return ESP_OK;
}

/**
 * @brief Handler for state VDG_MAIN_FSM_BOOT.
 */
void StateManager::boot() {
    esp_err_t err = ESP_OK;

    /** Initialize managers. */
    err = configManager_.initialize();
    if (err != ESP_OK) goto err;

    err = mqttManager_.initialize();
    if (err != ESP_OK) goto err;


    state_ = VDG_MAIN_FSM_CONNECT;
    return;

err:
    state_ = VDG_MAIN_FSM_FATAL_ERROR;
    return;
}

/**
 * @brief Handler for state VDG_MAIN_FSM_FATAL_ERROR.
 */
void StateManager::fatalError() {
    // Placeholder for fatal error state logic
}

/**
 * @brief Handler for state VDG_MAIN_FSM_CONNECT.
 */
void StateManager::connect() {
    esp_err_t err = ESP_OK;
    
    /** 
     * Attempt connection. 
     * The WifiManager handles all wifi provisioning events in the background.
     */
    wifiManager_.start();

    /** Continue if connected or provision WiFi & MQTT if not. */
    if (wifiManager_.isConnected() == true) {
        state_ = VDG_MAIN_FSM_LISTEN;
        return;
    } else {
        state_ = VDG_MAIN_FSM_PROVISIONING;
        return;
    }
}

/**
 * @brief Handler for state VDG_MAIN_FSM_PROVISIONING.
 */
void StateManager::accessPoint() {
    /** Move forward once connected. */
    if (wifiManager_.isConnected() == true) {
        state_ = VDG_MAIN_FSM_LISTEN;
        return;
    }
}

/**
 * @brief Handler for state VDG_MAIN_FSM_RESTART.
 */
void StateManager::restart() {
    esp_err_t err = ESP_OK;
    
    /** Report reset to MQTT. */
    dataContainer_.logInfo(ESP_OK, TAG, "Device reset requested.");
    mqttManager_.uploadLogs();

    /** Reset device. */
    esp_restart();

    /** Should not reach here. */
    state_ = VDG_MAIN_FSM_FATAL_ERROR;
    return;
}

/**
 * @brief Handler for state VDG_MAIN_FSM_LISTEN.
 */
void StateManager::listen() {
    esp_err_t err = ESP_OK;
    
    /** Handle received messages. */
    err = handleReceivedMessages(vdgMessageHandleFuncTableListenState);
    if (err != ESP_OK) {
        return;
    }
}

/**
 * @brief Handler for state VDG_MAIN_FSM_DISPENSE.
 */
void StateManager::dispense() {
    esp_err_t err = ESP_OK;
    TickType_t currentTicks = 0U;
    VdgValveProcess_e valveProcess = VDG_VALVES_MIN;
    VdgFlowSensorCalibrationState_e flowSensorState = VDG_FLOW_SENSOR_CALIBRATION_MIN;

    /** Handle received messages. */
    err = handleReceivedMessages(vdgMessageHandleFuncTableDispenseState);
    if (err != ESP_OK) {
        return;
    }

    /** Get current time. */
    currentTicks = xTaskGetTickCount();

    /** Update process state. */
    err = valveManager_.updateProcess(valveProcess);
    if (err != ESP_OK) {
        dataContainer_.logError(err, TAG, "Failed to update valve dispense process.");
        goto exit;
    }
    
    /** Handle state transition based on dispensation status. */
    switch (valveProcess) {

        /** Error state. */
        default:
        case VDG_VALVES_MIN:
        case VDG_VALVES_MAX:
        case VDG_VALVES_DRAIN:
            dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "ValveManager in an invalid state.");
            goto exit;
            break;
        
        /** Continuing to dispense. */
        case VDG_VALVES_DISPENSE:

            if ( (currentTicks - eventTimers_.lastProcessSliceUploadTicks) >=
                pdMS_TO_TICKS(PROCESS_SLICE_UPLOAD_MS_DEFAULT) ) {
                err = mqttManager_.uploadDispenseSlice();
                if (err != ESP_OK) {
                    dataContainer_.logError(err, TAG, "Failed to upload dispense slice.");
                }

                eventTimers_.lastProcessSliceUploadTicks = currentTicks;
            }
            break;
            
            
        /** Dispense has concluded. */
        case VDG_VALVES_IDLE:
            goto exit;
            break;
    }

exit:
    /** End the process. */
    err = valveManager_.endProcess(valveProcess);
    if ( (err != ESP_OK) || (valveProcess != VDG_VALVES_IDLE) ) {
        dataContainer_.logError(err, TAG, "Failed to deactivate dispensation.");
    }
    
    /** Report the final variables. */
    err = mqttManager_.uploadDispenseSlice();
    if (err != ESP_OK) {
        dataContainer_.logError(err, TAG, "Failed to upload dispense slice.");
    }
    err = mqttManager_.uploadDispenseSummary();
    if (err != ESP_OK) {
        dataContainer_.logError(err, TAG, "Failed to upload dispense summary.");
    }

    dataContainer_.logInfo(ESP_OK, TAG, "Concluded dispense process.");
    state_ = VDG_MAIN_FSM_LISTEN;
    return;
}

/**
 * @brief Handler for state VDG_MAIN_FSM_FLOW_CALIBRATE.
 */
void StateManager::flowCalibrate() {
    esp_err_t err = ESP_OK;
    VdgValveProcess_e valveProcess = VDG_VALVES_MIN;
    VdgFlowSensorCalibrationState_e calibrationState = VDG_FLOW_SENSOR_CALIBRATION_MIN;
    VdgFlowCalibrationProcessData_t processData = {};
    bool saveConfig = false;

    /** Handle received messages. */
    err = handleReceivedMessages(vdgMessageHandleFuncTableDispenseState);
    if (err != ESP_OK) {
        return;
    }

    /** Retrieve process data. */
    err = dataContainer_.getFlowCalibrationProcessData(processData);
    if (err != ESP_OK) {
        dataContainer_.logError(err, TAG, "Failed to retrieve flow calibration process data.");
        goto exit;
    }

    /** Update process state. */
    err = valveManager_.updateProcess(valveProcess);
    if (err != ESP_OK) {
        dataContainer_.logError(err, TAG, "Failed to update flow calibration process - valve dispense.");
        goto exit;
    }

    /** Handle state transition based on valve status. */
    switch (valveProcess) {

        /** Error state. */
        default:
        case VDG_VALVES_MIN:
        case VDG_VALVES_DRAIN:
        case VDG_VALVES_MAX:
            dataContainer_.logError(err, TAG, "ValveManager in an invalid state.");
            goto exit;
            break;
        
        /** Continuing to dispense. */
        case VDG_VALVES_DISPENSE:
            break;

        /** Finished dispensing. */
        case VDG_VALVES_IDLE:
            break;
            
    }

exit:
    /** End the process. */
    err = valveManager_.endProcess(valveProcess);
    if ( (err != ESP_OK) || (valveProcess != VDG_VALVES_IDLE) ) {
        dataContainer_.logError(err, TAG, "Failed to deactivate dispensation.");
    }

    if (saveConfig) {
        dataContainer_.logError(err, TAG, "Saved flow sensor calibration data:...");

        /** TODO: Set and persist the config. */
    }

    dataContainer_.logInfo(err, TAG, "Concluded calibration process.");
    state_ = VDG_MAIN_FSM_LISTEN;
    return;
}

/**
 * @brief Handler for state VDG_MAIN_FSM_PRESSURE_CALIBRATE.
 */
void StateManager::pressureCalibrate() {
    // Placeholder for pressure calibrate state logic
}

/**
 * @brief Handler for state VDG_MAIN_FSM_DRAIN.
 */
void StateManager::drain() {
    // Placeholder for drain state logic
}

/**
 * @brief Handles state change for a dispense request.
 * 
 * @param[in] message MQTT received message.
 */
void StateManager::handleDispenseRequest(VdgMessage_t message) {
    esp_err_t err = ESP_OK;
    char log[VDG_LOG_MESSAGE_BUFFER_BYTES];
    VdgDispenseActivateCommand_t *command = nullptr;
    VdgFlowSensorCalibrationState_e flowSensorState = VDG_FLOW_SENSOR_CALIBRATION_MIN;
    VdgValveProcess_e valveProcess = VDG_VALVES_MIN;

    /** Validate state. */
    if (state_ != VDG_MAIN_FSM_LISTEN) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Dispense handler called in invalid state.");
        return;
    }

    /** Validate inputs. */
    if (message.payload == nullptr) {
        dataContainer_.logError(ESP_ERR_INVALID_ARG, TAG, "Dispense handler called with null message payload.");
        return;
    }
    
    /** Typecast the payload. */
    command = reinterpret_cast<VdgDispenseActivateCommand_t*>(message.payload);

    /** Begin the dispensation process. */
    err = valveManager_.beginDispenseProcess(*command, valveProcess);
    if ( (err != ESP_OK) || (valveProcess != VDG_VALVES_DISPENSE) ) {
        dataContainer_.logError(err, TAG, "Failed to begin dispense process.");
        goto err;
    }

    /** Log info. */
    switch (command->targetType) {
        case VDG_DISPENSE_PROCESS_TARGET_TIME:
            snprintf(log, 
                sizeof(log), 
                "Beginning dispense process with a target volume: %.2f liters, timeout: %ld miliseconds", 
                command->target, 
                command->timeoutMs
            );
            break;

        case VDG_DISPENSE_PROCESS_TARGET_VOLUME:
            snprintf(log, 
                sizeof(log), 
                "Beginning dispense process with a target time: %.2f seconds, timeout: %ld miliseconds", 
                command->target, 
                command->timeoutMs
            );
            break;

        default:
            dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Invalid value of VdgDispenseProcessTargetType_e.");
            return;
    }

    dataContainer_.logInfo(ESP_OK, TAG, log);

    state_ = VDG_MAIN_FSM_DISPENSE;
    return;

err:
    valveManager_.endProcess(valveProcess);
    return;
}

/**
 * @brief Handles state change for a deactivation request.
 * 
 * @param[in] message MQTT received message.
 * @return esp_err_t Return code.
 */
void StateManager::handleDeactivateRequest(VdgMessage_t message) {
    return;
}

/**
 * @brief Handles state change for a restart request.
 * 
 * @param[in] message MQTT received message.
 */
void StateManager::handleRestartRequest(VdgMessage_t message) {
    return;
}

/**
 * @brief Handles state change for a flow calibrate request
 * when in the state VDG_MAIN_FSM_LISTEN.
 * 
 * @param[in] message MQTT received message.
 */
void StateManager::handleFlowCalibrateRequestListen(VdgMessage_t message) {
    esp_err_t err = ESP_OK;
    char log[128];
    VdgFlowCalibrationUpdateCommand_t *command = nullptr;
    VdgFlowSensorCalibrationState_e flowSensorState = VDG_FLOW_SENSOR_CALIBRATION_MIN;
    VdgValveProcess_e valveProcess = VDG_VALVES_MIN;
    VdgDispenseProcessTarget_t dispenseTarget = {};

    /** Validate state. */
    if (state_ != VDG_MAIN_FSM_LISTEN) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Flow calibration handler called in invalid state.");
        return;
    }

    /** Validate inputs. */
    if (message.payload == nullptr) {
        dataContainer_.logError(ESP_ERR_INVALID_ARG, TAG, "Flow calibration handler called with null message payload.");
        return;
    }
    
    /** Typecast the payload. */
    command = reinterpret_cast<VdgFlowCalibrationUpdateCommand_t*>(message.payload);

    /** Begin the calibration process. */
    //err = flowSensorManager_.beginCalibration(flowSensorState);
    if ( (err != ESP_OK) || (flowSensorState != VDG_FLOW_SENSOR_CALIBRATION_MEASURING) ) {
        dataContainer_.logError(err, TAG, "Failed to begin flow calibration process.");
        goto err;
    }

    /** Begin valve dispensation. */
    dispenseTarget = {};
    dispenseTarget.targetType = VDG_DISPENSE_PROCESS_TARGET_VOLUME;
    dispenseTarget.target = command->targetVolume;
    dispenseTarget.timeoutMs = command->timeoutMs;
    err = valveManager_.beginDispenseProcess(dispenseTarget, valveProcess);
    if ( (err != ESP_OK) || (valveProcess != VDG_VALVES_DISPENSE) ) {
        dataContainer_.logError(err, TAG, "Failed to begin flow calibration process - dispense.");
    }

    /** Log info. */    
    snprintf(log, 
        sizeof(log), 
        "Beginning calibration process with a target volume: %.2f liters, timeout: %ld ms", 
        command->targetVolume, 
        command->timeoutMs
    );
    dataContainer_.logInfo(ESP_OK, TAG, log);

    state_ = VDG_MAIN_FSM_FLOW_CALIBRATE;
    return;

err:
    valveManager_.endProcess(valveProcess);
    return;
}

/**
 * @brief Handles state change for a flow calibrate request
 * when in the state VDG_MAIN_FSM_FLOW_CALIBRATE.
 * 
 * @param[in] message MQTT received message.
 */
void StateManager::handleFlowCalibrateRequestFlowCalibration(VdgMessage_t message) {
    esp_err_t err = ESP_OK;
    char log[128];
    VdgFlowCalibrationUpdateCommand_t *command = nullptr;
    VdgValveProcess_e valveProcess = VDG_VALVES_MIN;
    VdgFlowSensorCalibrationState_e flowSensorState = VDG_FLOW_SENSOR_CALIBRATION_MIN; 

    /** Validate state. */
    if (state_ != VDG_MAIN_FSM_FLOW_CALIBRATE) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Flow calibration handler called in invalid state.");
        return;
    }

    /** Validate inputs. */
    if (message.payload == nullptr) {
        dataContainer_.logError(ESP_ERR_INVALID_ARG, TAG, "Flow calibration handler called with null message payload.");
        return;
    }
    
    /** Typecast the payload. */
    command = reinterpret_cast<VdgFlowCalibrationUpdateCommand_t*>(message.payload);

    valveManager_.getCurrentProcess(valveProcess);

    /** Retrieve valve status. */

    /** Handle state transition based on valve status. */
    //switch (valveProcess) {}


    return;

}

/**
 * @brief Handles state change for a pressure calibrate request.
 * 
 * @param message MQTT received message.
 * @return esp_err_t Return code.
 */
void StateManager::handlePressureCalibrateRequest(VdgMessage_t message) {
    return;
}

/**
 * @brief Handles state change for a drain request.
 * 
 * @param message MQTT received message.
 * @return esp_err_t Return code.
 */
void StateManager::handleDrainRequest(VdgMessage_t message) {
    return;
}

/**
 * @brief Handles state change for a pressure poll request.
 * 
 * @param message MQTT received message.
 * @return esp_err_t Return code.
 */
void StateManager::handlePressurePollRequest(VdgMessage_t message) {
    return;
}

/**
 * @brief Handles state change for a config change request.
 * 
 * @param message MQTT received message.
 * @return esp_err_t Return code.
 */
void StateManager::handleConfigChangeRequest(VdgMessage_t message) {
    return;
}

/**
 * @brief Initializes and mounts the LittleFS filesystem.
 * 
 * @details Taken from https://github.com/Ariif0/ESPIDF-WiFi-Configuration/blob/main/include/Application.h
 *
 * Registers the LittleFS driver with the Virtual File System (VFS) using the
 * base path defined in config.h, enabling file operations on the storage partition.
 */
esp_err_t StateManager::initializeFilesystem() {
    ESP_LOGI(TAG, "Initializing LittleFS...");
    esp_vfs_littlefs_conf_t conf = {
        .base_path = LFS_BASE_PATH,            
        .partition_label = LFS_PARTITION_LABEL, 
        .partition = NULL,                     
        .format_if_mount_failed = true,        
        .read_only = false,                    
        .dont_mount = false,                   
        .grow_on_mount = false,                
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find LittleFS partition. Ensure '%s' exists in partition_custom.csv", LFS_PARTITION_LABEL);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to retrieve LittleFS partition info (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    ESP_LOGI(TAG, "LittleFS successfully mounted at path: %s", LFS_BASE_PATH);

    return ESP_OK;
}