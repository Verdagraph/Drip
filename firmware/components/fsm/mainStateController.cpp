#include "cstdio"
#include "array"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "etl/const_map.h"
#include "etl/span.h"

#include "utils.h"
#include "dataContainer.h"
#include "configManager.h"
#include "mqttManager.h"
#include "wifiManager.h"
#include "valveManager.h"
#include "flowManager.h"

#include "mainStateController.h"

#define ETL_NO_STL

static const char* TAG = "MainStateController";

/**
 * @brief State handler map.
 */
constexpr StateHandlerMapPair<DripMainFsmState_e, MainStateController> stateToHandlerMapValues[] {
    etl::pair{
        DripMainFsmState_e::Uninitialized, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::uninitializedEntry, 
            &MainStateController::uninitializedUpdate, 
            &MainStateController::uninitializedExit, 
            1U /** handlingIntervalMs. N/A, one-shot state. */
        ),
    },
    etl::pair{
        DripMainFsmState_e::Boot, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::bootEntry, 
            &MainStateController::bootUpdate, 
            &MainStateController::bootExit, 
            1U /** handlingIntervalMs. N/A, one-shot state. */
        ),
    },
    etl::pair{
        DripMainFsmState_e::FatalError, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::fatalErrorEntry, 
            &MainStateController::fatalErrorUpdate, 
            &MainStateController::fatalErrorExit, 
            1000U /** handlingIntervalMs. Idles without much need for response. */
        ),
    },
    etl::pair{
        DripMainFsmState_e::Connect, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::connectEntry, 
            &MainStateController::connectUpdate, 
            &MainStateController::connectExit, 
            10U /** handlingIntervalMs. Quick handling of connection logic. */
        ),
    },
    etl::pair{
        DripMainFsmState_e::Provisioning, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::provisioningEntry, 
            &MainStateController::provisioningUpdate, 
            &MainStateController::provisioningExit, 
            10U /** handlingIntervalMs. Quick handling of connection logic. */
        ),
    },
    etl::pair{
        DripMainFsmState_e::Restart, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::restartEntry, 
            &MainStateController::restartUpdate, 
            &MainStateController::restartExit, 
            1U /** handlingIntervalMs. N/A, one-shot state. */
        ),
    },
    etl::pair{
        DripMainFsmState_e::Listen, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::listenEntry, 
            &MainStateController::listenUpdate, 
            &MainStateController::listenExit, 
            250U /** handlingIntervalMs. Reasonably quick response time to messages. */
        ),
    },
    etl::pair{
        DripMainFsmState_e::Dispense, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::dispenseEntry, 
            &MainStateController::dispenseUpdate, 
            &MainStateController::dispenseExit, 
            100U /** handlingIntervalMs. Reasonably quick response time to dispense events. */
        ),
    },
    etl::pair{
        DripMainFsmState_e::FlowSensorCalibrate, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::flowSensorCalibrateEntry, 
            &MainStateController::flowSensorCalibrateUpdate, 
            &MainStateController::flowSensorCalibrateExit, 
            100U /** handlingIntervalMs. Reasonably quick response time to dispense events. */
        ),
    },
    etl::pair{
        DripMainFsmState_e::Drain, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::drainEntry, 
            &MainStateController::drainUpdate, 
            &MainStateController::drainExit, 
            100U /** handlingIntervalMs. Reasonably quick response time to dispense events. */
        ),
    }
};
using FsmStateMap = StateHandlerMap<DripMainFsmState_e, MainStateController>;
constexpr FsmStateMap stateToHandlerMap { stateToHandlerMapValues };

/**
 * @brief Event handler map.
 */
constexpr EventHandlerMapPair<DripMainFsmState_e, DripRxMessageId_e, DripRxMessage, MainStateController> eventToHandlerMapValues[] {
    etl::pair{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::Listen, 
            DripRxMessageId_e::DispenseActivate
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage, MainStateController>(
            
            &MainStateController::handleDispenseRequestStateListen
        )
    }
};
using FsmEventMap = EventHandlerMap<DripMainFsmState_e, DripRxMessageId_e, DripRxMessage, MainStateController>;
constexpr FsmEventMap eventToHandlerMap { eventToHandlerMapValues };




/**
 * @brief Constructor
 */
MainStateController::MainStateController(const DataContainer &dataContainer) :
    StateController<DripMainFsmState_e, DripRxMessageId_e, DripRxMessage, MainStateController>(
        DripMainFsmState_e::Uninitialized,
        "MainFsm",
        stateToHandlerMap,
        eventToHandlerMap,
        this,
        dataContainer,
    ),
    eventTimers_(),
    configManager_(),
    dataContainer_(),
    mqttManager_(),
    wifiManager_(),
    valveManager_(configManager_, dataContainer_),
    flowSensorManager_() {}


/**
 * @brief Handlers for state uninitialized.
 */
void MainStateController::uninitializedEntry() {
    esp_err_t err = ESP_OK;

    /** Initialize managers. */
    err = configManager_.initialize();
    if (err != ESP_OK) goto err;

    err = mqttManager_.initialize();
    if (err != ESP_OK) goto err;


    state_ = DripMainFsmState_e::Connect;
    return;

err:
    state_ = DripMainFsmState_e::FatalError;
    return;
}
void MainStateController::uninitializedUpdate() {}
void MainStateController::uninitializedExit() {}

/**
 * @brief Handlers for state boot.
 */
void MainStateController::bootEntry() {
    return;
}
void MainStateController::bootUpdate() {
    return;
}
void MainStateController::bootExit() {
    return;
}

/**
 * @brief Handlers for state fatal error.
 */
void MainStateController::fatalErrorEntry() {
    return;
}
void MainStateController::fatalErrorUpdate() {
    return;
}
void MainStateController::fatalErrorExit() {
    return;
}

/**
 * @brief Handlers for state connect.
 */
void MainStateController::connectEntry() {
    return;
}
void MainStateController::connectUpdate() {
    return;
}
void MainStateController::connectExit() {
    return;
}

/**
 * @brief Handlers for state provisioning.
 */
void MainStateController::provisioningEntry() {
    return;
}
void MainStateController::provisioningUpdate() {
    return;
}
void MainStateController::provisioningExit() {
    return;
}

/**
 * @brief Handlers for state restart.
 */
void MainStateController::restartEntry() {
    return;
}
void MainStateController::restartUpdate() {
    return;
}
void MainStateController::restartExit() {
    return;
}

/**
 * @brief Handlers for state listen.
 */
void MainStateController::listenEntry() {
    return;
}
void MainStateController::listenUpdate() {
    return;
}
void MainStateController::listenExit() {
    return;
}

/**
 * @brief Handlers for state dispense.
 */
void MainStateController::dispenseEntry() {
    return;
}
void MainStateController::dispenseUpdate() {
    return;
}
void MainStateController::dispenseExit() {
    return;
}

/**
 * @brief Handlers for state flow calibrate.
 */
void MainStateController::flowSensorCalibrateEntry() {
    return;
}
void MainStateController::flowSensorCalibrateUpdate() {
    return;
}
void MainStateController::flowSensorCalibrateExit() {
    return;
}

/**
 * @brief Handlers for state drain.
 */
void MainStateController::drainEntry() {
    return;
}
void MainStateController::drainUpdate() {
    return;
}
void MainStateController::drainExit() {
    return;
}


/**
 * @brief Handler for state DripMainFsmState_e::Boot.
 */
void StateManager::boot() {

}

/**
 * @brief Handler for state DripMainFsmState_e::FatalError.
 */
void StateManager::fatalError() {
    // Placeholder for fatal error state logic
}

/**
 * @brief Handler for state DripMainFsmState_e::Connect.
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
        state_ = DripMainFsmState_e::Listen;
        return;
    } else {
        state_ = DripMainFsmState_e::Provisioning;
        return;
    }
}

/**
 * @brief Handler for state DripMainFsmState_e::Provisioning.
 */
void StateManager::accessPoint() {
    /** Move forward once connected. */
    if (wifiManager_.isConnected() == true) {
        state_ = DripMainFsmState_e::Listen;
        return;
    }
}

/**
 * @brief Handler for state DripMainFsmState_e::Restart.
 */
void StateManager::restart() {
    esp_err_t err = ESP_OK;
    
    /** Report reset to MQTT. */
    dataContainer_.logInfo(ESP_OK, TAG, "Device reset requested.");
    mqttManager_.uploadLogs();

    /** Reset device. */
    esp_restart();

    /** Should not reach here. */
    state_ = DripMainFsmState_e::FatalError;
    return;
}

/**
 * @brief Handler for state DripMainFsmState_e::Listen.
 */
void StateManager::listen() {
    esp_err_t err = ESP_OK;
    
    /** Handle received messages. */
    err = handleReceivedMessages(vdgMessageHandleFuncTableListenState);
    if (err != ESP_OK) {
        return;
    }

    /** TODO Implement sleep interval. */
}

/**
 * @brief Handler for state DripMainFsmState_e::Dispense.
 */
void StateManager::dispense() {
    esp_err_t err = ESP_OK;
    TickType_t currentTicks = 0U;
    VdgValveProcess_e valveProcess = DRIP_VALVES_MIN;
    DripFlowSensorCalibrationState_e flowSensorState = DRIP_FLOW_SENSOR_CALIBRATION_MIN;

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
        case DRIP_VALVES_MIN:
        case DRIP_VALVES_MAX:
        case DRIP_VALVES_DRAIN:
            dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "ValveManager in an invalid state.");
            goto exit;
            break;
        
        /** Continuing to dispense. */
        case DRIP_VALVES_DISPENSE:

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
        case DRIP_VALVES_IDLE:
            goto exit;
            break;
    }

exit:
    /** End the process. */
    err = valveManager_.endProcess(valveProcess);
    if ( (err != ESP_OK) || (valveProcess != DRIP_VALVES_IDLE) ) {
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
    state_ = DripMainFsmState_e::Listen;
    return;
}

/**
 * @brief Handler for state DripMainFsmState_e::FlowSensorCalibrate.
 */
void StateManager::flowCalibrate() {
    esp_err_t err = ESP_OK;
    VdgValveProcess_e valveProcess = DRIP_VALVES_MIN;
    DripFlowSensorCalibrationState_e calibrationState = DRIP_FLOW_SENSOR_CALIBRATION_MIN;
    DripFlowCalibrationProcessData_t processData = {};
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
        case DRIP_VALVES_MIN:
        case DRIP_VALVES_DRAIN:
        case DRIP_VALVES_MAX:
            dataContainer_.logError(err, TAG, "ValveManager in an invalid state.");
            goto exit;
            break;
        
        /** Continuing to dispense. */
        case DRIP_VALVES_DISPENSE:
            break;

        /** Finished dispensing. */
        case DRIP_VALVES_IDLE:
            break;
            
    }

exit:
    /** End the process. */
    err = valveManager_.endProcess(valveProcess);
    if ( (err != ESP_OK) || (valveProcess != DRIP_VALVES_IDLE) ) {
        dataContainer_.logError(err, TAG, "Failed to deactivate dispensation.");
    }

    if (saveConfig) {
        dataContainer_.logError(err, TAG, "Saved flow sensor calibration data:...");

        /** TODO: Set and persist the config. */
    }

    dataContainer_.logInfo(err, TAG, "Concluded calibration process.");
    state_ = DripMainFsmState_e::Listen;
    return;
}

/**
 * @brief Handler for state DripMainFsmState_e::PressureSensorCalibrate.
 */
void StateManager::pressureCalibrate() {
    // Placeholder for pressure calibrate state logic
}

/**
 * @brief Handler for state DripMainFsmState_e::Drain.
 */
void StateManager::drain() {
    // Placeholder for drain state logic
}

/**
 * @brief Handles state change for a dispense request.
 * 
 * @param[in] message MQTT received message.
 */
void StateManager::handleDispenseRequest(DripMessage_t message) {
    esp_err_t err = ESP_OK;
    char log[DRIP_LOG_MESSAGE_BUFFER_BYTES];
    DripDispenseActivateCommand_t *command = nullptr;
    DripFlowSensorCalibrationState_e flowSensorState = DRIP_FLOW_SENSOR_CALIBRATION_MIN;
    VdgValveProcess_e valveProcess = DRIP_VALVES_MIN;

    /** Validate state. */
    if (state_ != DripMainFsmState_e::Listen) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Dispense handler called in invalid state.");
        return;
    }

    /** Validate inputs. */
    if (message.payload == nullptr) {
        dataContainer_.logError(ESP_ERR_INVALID_ARG, TAG, "Dispense handler called with null message payload.");
        return;
    }
    
    /** Typecast the payload. */
    command = reinterpret_cast<DripDispenseActivateCommand_t*>(message.payload);

    /** Begin the dispensation process. */
    err = valveManager_.beginDispenseProcess(*command, valveProcess);
    if ( (err != ESP_OK) || (valveProcess != DRIP_VALVES_DISPENSE) ) {
        dataContainer_.logError(err, TAG, "Failed to begin dispense process.");
        goto err;
    }

    /** Log info. */
    switch (command->targetType) {
        case DRIP_DISPENSE_PROCESS_TARGET_TIME:
            snprintf(log, 
                sizeof(log), 
                "Beginning dispense process with a target volume: %.2f liters, timeout: %ld miliseconds", 
                command->target, 
                command->timeoutMs
            );
            break;

        case DRIP_DISPENSE_PROCESS_TARGET_VOLUME:
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

    state_ = DripMainFsmState_e::Dispense;
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
void StateManager::handleDeactivateRequest(DripMessage_t message) {
    return;
}

/**
 * @brief Handles state change for a restart request.
 * 
 * @param[in] message MQTT received message.
 */
void StateManager::handleRestartRequest(DripMessage_t message) {
    return;
}

/**
 * @brief Handles state change for a flow calibrate request
 * when in the state DripMainFsmState_e::Listen.
 * 
 * @param[in] message MQTT received message.
 */
void StateManager::handleFlowCalibrateRequestListen(DripMessage_t message) {
    esp_err_t err = ESP_OK;
    char log[128];
    DripFlowCalibrationUpdateCommand_t *command = nullptr;
    DripFlowSensorCalibrationState_e flowSensorState = DRIP_FLOW_SENSOR_CALIBRATION_MIN;
    VdgValveProcess_e valveProcess = DRIP_VALVES_MIN;
    VdgDispenseProcessTarget_t dispenseTarget = {};

    /** Validate state. */
    if (state_ != DripMainFsmState_e::Listen) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Flow calibration handler called in invalid state.");
        return;
    }

    /** Validate inputs. */
    if (message.payload == nullptr) {
        dataContainer_.logError(ESP_ERR_INVALID_ARG, TAG, "Flow calibration handler called with null message payload.");
        return;
    }
    
    /** Typecast the payload. */
    command = reinterpret_cast<DripFlowCalibrationUpdateCommand_t*>(message.payload);

    /** Begin the calibration process. */
    //err = flowSensorManager_.beginCalibration(flowSensorState);
    if ( (err != ESP_OK) || (flowSensorState != DRIP_FLOW_SENSOR_CALIBRATION_MEASURING) ) {
        dataContainer_.logError(err, TAG, "Failed to begin flow calibration process.");
        goto err;
    }

    /** Begin valve dispensation. */
    dispenseTarget = {};
    dispenseTarget.targetType = DRIP_DISPENSE_PROCESS_TARGET_VOLUME;
    dispenseTarget.target = command->targetVolume;
    dispenseTarget.timeoutMs = command->timeoutMs;
    err = valveManager_.beginDispenseProcess(dispenseTarget, valveProcess);
    if ( (err != ESP_OK) || (valveProcess != DRIP_VALVES_DISPENSE) ) {
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

    state_ = DripMainFsmState_e::FlowSensorCalibrate;
    return;

err:
    valveManager_.endProcess(valveProcess);
    return;
}

/**
 * @brief Handles state change for a flow calibrate request
 * when in the state DripMainFsmState_e::FlowSensorCalibrate.
 * 
 * @param[in] message MQTT received message.
 */
void StateManager::handleFlowCalibrateRequestFlowCalibration(DripMessage_t message) {
    esp_err_t err = ESP_OK;
    char log[128];
    DripFlowCalibrationUpdateCommand_t *command = nullptr;
    VdgValveProcess_e valveProcess = DRIP_VALVES_MIN;
    DripFlowSensorCalibrationState_e flowSensorState = DRIP_FLOW_SENSOR_CALIBRATION_MIN; 

    /** Validate state. */
    if (state_ != DripMainFsmState_e::FlowSensorCalibrate) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Flow calibration handler called in invalid state.");
        return;
    }

    /** Validate inputs. */
    if (message.payload == nullptr) {
        dataContainer_.logError(ESP_ERR_INVALID_ARG, TAG, "Flow calibration handler called with null message payload.");
        return;
    }
    
    /** Typecast the payload. */
    command = reinterpret_cast<DripFlowCalibrationUpdateCommand_t*>(message.payload);

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
void StateManager::handlePressureCalibrateRequest(DripMessage_t message) {
    return;
}

/**
 * @brief Handles state change for a drain request.
 * 
 * @param message MQTT received message.
 * @return esp_err_t Return code.
 */
void StateManager::handleDrainRequest(DripMessage_t message) {
    return;
}

/**
 * @brief Handles state change for a pressure poll request.
 * 
 * @param message MQTT received message.
 * @return esp_err_t Return code.
 */
void StateManager::handlePressurePollRequest(DripMessage_t message) {
    return;
}

/**
 * @brief Handles state change for a config change request.
 * 
 * @param message MQTT received message.
 * @return esp_err_t Return code.
 */
void StateManager::handleConfigChangeRequest(DripMessage_t message) {
    return;
}



void StateManager::update() {
    esp_err_t err = ESP_OK;

    handleCurrentState();

    /** Upload all logs to MQTT. */
    err = mqttManager_.uploadLogs();
    if (err != ESP_OK) {
        dataContainer_.logError(err, TAG, "Failed to upload logs.");
    }
}


esp_err_t StateManager::handleReceivedMessages(const DripMessageHandleFuncTable_t *table, size_t tableLen) {
    DripRxMessage message = DripRxMessage(0);
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