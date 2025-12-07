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
        DripMainFsmState_e::ConnectWifi, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::connectWifiEntry, 
            &MainStateController::connectWifiUpdate, 
            &MainStateController::connectWifiExit, 
            10U /** handlingIntervalMs. Quick handling of connection logic. */
        ),
    },
    etl::pair{
        DripMainFsmState_e::ConnectMqtt, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::connectMqttEntry, 
            &MainStateController::connectMqttUpdate, 
            &MainStateController::connectMqttExit, 
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
MainStateController::MainStateController(
    const ConfigManager &configManager,
    const DataContainer &dataContainer,
    const WifiManager &wifiManager, 
    const MqttManager &mqttManager,
    const ValveManager &valveManager,
    const FlowSensorManager &flowSensorManager
) :
    StateController<DripMainFsmState_e, DripRxMessageId_e, DripRxMessage, MainStateController>(
        DripMainFsmState_e::Uninitialized,
        "MainFsm",
        stateToHandlerMap,
        eventToHandlerMap,
        *this,
        dataContainer
    ),

    eventTimers_(),
    
    configManager_(configManager),
    dataContainer_(dataContainer),
    mqttManager_(mqttManager),
    wifiManager_(wifiManager),
    valveManager_(valveManager),
    flowSensorManager_(flowSensorManager) {}


/**
 * @brief Handlers for state uninitialized.
 */
void MainStateController::uninitializedEntry() {
    esp_err_t err = ESP_OK;

    /** Initialize managers. */
    err = configManager_.initialize();
    if (err != ESP_OK) machine_.transition(DripMainFsmState_e::FatalError);

    err = mqttManager_.initialize();
    if (err != ESP_OK) machine_.transition(DripMainFsmState_e::FatalError);

    /** Enter boot. */
    err = machine_.transition(DripMainFsmState_e::Boot);
    if (err != ESP_OK) machine_.transition(DripMainFsmState_e::FatalError);

    return;
}
void MainStateController::uninitializedUpdate() {}
void MainStateController::uninitializedExit() {}

/**
 * @brief Handlers for state boot.
 */
void MainStateController::bootEntry() {
    esp_err_t err = ESP_OK;

    /** Enter connection. */
    err = machine_.transition(DripMainFsmState_e::ConnectWifi);
    if (err != ESP_OK) goto err;

    return;

err:
    machine_.transition(DripMainFsmState_e::FatalError);
    return;
}
void MainStateController::bootUpdate() {}
void MainStateController::bootExit() {}

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
 * @brief Handlers for state connect wifi.
 */
void MainStateController::connectWifiEntry() {
    esp_err_t err = ESP_OK;
    
    if (wifiManager_.isConnected() ) {
        machine_.transition(DripMainFsmState_e::ConnectMqtt);
    }

    /** 
     * Attempt connection. 
     * The WifiManager handles all wifi provisioning events in the background.
     */
    wifiManager_.start();

    eventTimers_.wifiConnectionBeganTicks = xTaskGetTickCount();

    return;
}
void MainStateController::connectWifiUpdate() {
    esp_err_t err = ESP_OK;
    
    if (true == wifiManager_.isConnected()) {
        err = machine_.transition(DripMainFsmState_e::ConnectMqtt);
        if (err != ESP_OK) goto err;
        
    } else if ( (xTaskGetTickCount() - eventTimers_.wifiConnectionBeganTicks) > pdMS_TO_TICKS(DRIP_MAIN_FSM_WIFI_CONNECTION_WAIT_MS) ) {
        err = machine_.transition(DripMainFsmState_e::Provisioning);
        if (err != ESP_OK) goto err;
    }

    return;
    
err:
    machine_.transition(DripMainFsmState_e::FatalError);
    return;
}
void MainStateController::connectWifiExit() {
    return;
}
/**
 * @brief Handlers for state connect mqtt.
 */
void MainStateController::connectMqttEntry() {
    esp_err_t err = ESP_OK;
    
    if (mqttManager_.connected() ) {
        machine_.transition(DripMainFsmState_e::Listen);
    }

    mqttManager_.connect();

    eventTimers_.mqttConnectionBeganTicks = xTaskGetTickCount();

    return;
}
void MainStateController::connectMqttUpdate() {
    esp_err_t err = ESP_OK;
    
    if (true == mqttManager_.connected()) {
        err = machine_.transition(DripMainFsmState_e::Listen);
        if (err != ESP_OK) goto err;
        
    } else if ( (xTaskGetTickCount() - eventTimers_.mqttConnectionBeganTicks) > pdMS_TO_TICKS(DRIP_MAIN_FSM_WIFI_CONNECTION_WAIT_MS) ) {
        err = machine_.transition(DripMainFsmState_e::Provisioning);
        if (err != ESP_OK) goto err;
    }

    return;
    
err:
    machine_.transition(DripMainFsmState_e::FatalError);
    return;
}
void MainStateController::connectMqttExit() {
    return;
}

/**
 * @brief Handlers for state provisioning.
 */
void MainStateController::provisioningEntry() {
    return;
}
void MainStateController::provisioningUpdate() {
    esp_err_t err = ESP_OK;

    /** Move forward once connected. */
    if (wifiManager_.isConnected() == true && mqttManager_.connected()) {

        err = machine_.transition(DripMainFsmState_e::Listen);
        if (err != ESP_OK) goto err;
        return;
    }

err:
    machine_.transition(DripMainFsmState_e::FatalError);
    return;
}
void MainStateController::provisioningExit() {
    return;
}

/**
 * @brief Handlers for state restart.
 */
void MainStateController::restartEntry() {
    esp_err_t err = ESP_OK;
    
    /** Report reset to MQTT. */
    dataContainer_.logInfo(ESP_OK, TAG, "Device reset requested.");
    mqttManager_.uploadLogs();

    /** Reset device. */
    esp_restart();

    /** Should not reach here. */
    machine_.transition(DripMainFsmState_e::FatalError);
    return;
}
void MainStateController::restartUpdate() {}
void MainStateController::restartExit() {}

/**
 * @brief Handlers for state listen.
 */
void MainStateController::listenEntry() {
    return;
}
void MainStateController::listenUpdate() {
    esp_err_t err = ESP_OK;

    if (false == mqttManager_.connected()) {
        machine_.transition(DripMainFsmState_e::ConnectWifi);
    }

    /** TODO Implement sleep interval. */
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
    esp_err_t err = ESP_OK;
    DripConfig_t config = {};
    TickType_t currentTicks = 0U;
    VdgValveProcess_e valveProcess = DRIP_VALVES_MIN;
    DripFlowSensorCalibrationState_e flowSensorState = DRIP_FLOW_SENSOR_CALIBRATION_MIN;

    /** Retrieve config. */
    err = dataContainer_.getConfig(config);
    if (ESP_OK != err) {
        machine_.transition(DripMainFsmState_e::FatalError);
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
                pdMS_TO_TICKS(config.system.processSliceUploadIntervalMs) ) {
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

/** todo remove */
exit:
    return;
}
void MainStateController::dispenseExit() {
    esp_err_t err = ESP_OK;
    VdgValveProcess_e valveProcess = DRIP_VALVES_MIN;

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
    machine_.transition(DripMainFsmState_e::Listen);
    return;
}

/**
 * @brief Handlers for state flow calibrate.
 */
void MainStateController::flowSensorCalibrateEntry() {
    return;
}
void MainStateController::flowSensorCalibrateUpdate() {
    esp_err_t err = ESP_OK;
    VdgValveProcess_e valveProcess = DRIP_VALVES_MIN;
    DripFlowSensorCalibrationState_e calibrationState = DRIP_FLOW_SENSOR_CALIBRATION_MIN;
    DripFlowCalibrationProcessData_t processData = {};
    bool saveConfig = false;

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

}
void MainStateController::flowSensorCalibrateExit() {
    esp_err_t err = ESP_OK;
    VdgValveProcess_e valveProcess = DRIP_VALVES_MIN;

    /** End the process. */
    err = valveManager_.endProcess(valveProcess);
    if ( (err != ESP_OK) || (valveProcess != DRIP_VALVES_IDLE) ) {
        dataContainer_.logError(err, TAG, "Failed to deactivate dispensation.");
    }

    //if (saveConfig) {
        //dataContainer_.logError(err, TAG, "Saved flow sensor calibration data:...");

        /** TODO: Set and persist the config. */
    //}

    //dataContainer_.logInfo(err, TAG, "Concluded calibration process.");
    //state_ = DripMainFsmState_e::Listen;
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


void MainStateController::handleDispenseRequestStateListen(DripRxMessageId_e id, const DripRxMessage *message) {
    esp_err_t err = ESP_OK;
    const DispenseActivateRxMessage *command = nullptr;
    DripFlowSensorCalibrationState_e flowSensorState = DRIP_FLOW_SENSOR_CALIBRATION_MIN;
    VdgValveProcess_e valveProcess = DRIP_VALVES_MIN;

    /** Validate state. */
    if (DripMainFsmState_e::Listen != machine_.getCurrentState()) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "DispenseActivateRxMessage handler called in invalid state.");
        return;
    }
    
    /** Validate payload. */
    if (nullptr == message){
        dataContainer_.logError(ESP_ERR_INVALID_ARG, TAG, "Null message passed to handleDispenseRequestStateListen.");
        return;
    }
    if (message->id() != DripRxMessageId_e::DispenseActivate) {
        dataContainer_.logError(ESP_ERR_INVALID_ARG, TAG, "Incorrect message type: %d passed to handleDispenseRequestStateListen.", message->id());
        return; 
    }

    /** Typecast the payload. */
    command = reinterpret_cast<const DispenseActivateRxMessage*>(message);
    VdgDispenseProcessTarget_t target = command->data();

    /** Begin the dispensation process. */
    err = valveManager_.beginDispenseProcess(target, valveProcess);
    if ( (err != ESP_OK) || (valveProcess != DRIP_VALVES_DISPENSE) ) {
        dataContainer_.logError(err, TAG, "Failed to begin dispense process.");
        goto err;
    }

    /** Log info. */
    switch (target.targetType) {
        case DRIP_DISPENSE_PROCESS_TARGET_TIME:
            dataContainer_.logInfo(ESP_OK, TAG, "Beginning dispense process with a target: %.2f seconds, timeout: %ld miliseconds", target.target, target.timeoutMs);
            break;

        case DRIP_DISPENSE_PROCESS_TARGET_VOLUME:
            dataContainer_.logInfo(ESP_OK, TAG, "Beginning dispense process with a target: %.2f liters, timeout: %ld miliseconds", target.target, target.timeoutMs);
            break;

        default:
            dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Invalid value of VdgDispenseProcessTargetType_e.");
            return;
    }

    machine_.transition(DripMainFsmState_e::Dispense);
    return;

err:
    valveManager_.endProcess(valveProcess);
    return;
}

void MainStateController::handleDeactivateRequestStateListen(DripRxMessageId_e id, const DripRxMessage *message) {
    return;
}

void MainStateController::handleRestartRequestStateListen(DripRxMessageId_e id, const DripRxMessage *message) {
    return;
}

void MainStateController::handleConfigChangeRequestStateListen(DripRxMessageId_e id, const DripRxMessage *message) {
    return;
}

void MainStateController::handleFlowCalibrateBeginRequestStateListen(DripRxMessageId_e id, const DripRxMessage *message) {
    esp_err_t err = ESP_OK;
    const FlowCalibrateBeginRxMessage *command = nullptr;
    DripFlowSensorCalibrationState_e flowSensorState = DRIP_FLOW_SENSOR_CALIBRATION_MIN;
    VdgValveProcess_e valveProcess = DRIP_VALVES_MIN;
    VdgDispenseProcessTarget_t dispenseTarget = {};

    /** Validate state. */
    if (DripMainFsmState_e::Listen != machine_.getCurrentState()) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "FlowCalibrateBeginRxMessage handler called in invalid state.");
        return;
    }
    
    /** Validate payload. */
    if (nullptr == message){
        dataContainer_.logError(ESP_ERR_INVALID_ARG, TAG, "Null message passed to handleFlowCalibrateBeginRequestStateListen.");
        return;
    }
    if (message->id() != DripRxMessageId_e::FlowCalibrateBegin) {
        dataContainer_.logError(ESP_ERR_INVALID_ARG, TAG, "Incorrect message type: %d passed to handleFlowCalibrateBeginRequestStateListen.", message->id());
        return; 
    }



    /** Begin the calibration process. */
    //err = flowSensorManager_.beginCalibration(flowSensorState);
    /*
    if ( (err != ESP_OK) || (flowSensorState != DRIP_FLOW_SENSOR_CALIBRATION_MEASURING) ) {
        dataContainer_.logError(err, TAG, "Failed to begin flow calibration process.");
        goto err;
    }
    */

    /** Begin valve dispensation. */
    /*
    dispenseTarget = {};
    dispenseTarget.targetType = DRIP_DISPENSE_PROCESS_TARGET_VOLUME;
    dispenseTarget.target = command->targetVolume;
    dispenseTarget.timeoutMs = command->timeoutMs;
    err = valveManager_.beginDispenseProcess(dispenseTarget, valveProcess);
    if ( (err != ESP_OK) || (valveProcess != DRIP_VALVES_DISPENSE) ) {
        dataContainer_.logError(err, TAG, "Failed to begin flow calibration process - dispense.");
    }
    */

    /** Log info. */    
    /*
    snprintf(log, 
        sizeof(log), 
        "Beginning calibration process with a target volume: %.2f liters, timeout: %ld ms", 
        command->targetVolume, 
        command->timeoutMs
    );
    dataContainer_.logInfo(ESP_OK, TAG, log);
    
    state_ = DripMainFsmState_e::FlowSensorCalibrate;
    */
    return;

err:
    valveManager_.endProcess(valveProcess);
    return;
}

void MainStateController::handleFlowCalibrateDispenseRequestStateFlowCalibration(DripRxMessageId_e id, const DripRxMessage *message) {
    /*
    esp_err_t err = ESP_OK;
    char log[128];
    DripFlowCalibrationUpdateCommand_t *command = nullptr;
    VdgValveProcess_e valveProcess = DRIP_VALVES_MIN;
    DripFlowSensorCalibrationState_e flowSensorState = DRIP_FLOW_SENSOR_CALIBRATION_MIN; 

    */
    /** Validate state. 
     * 
     if (state_ != DripMainFsmState_e::FlowSensorCalibrate) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Flow calibration handler called in invalid state.");
        return;
    }
    */
    
    /** Validate inputs. 
     * 
     if (message.payload == nullptr) {
        dataContainer_.logError(ESP_ERR_INVALID_ARG, TAG, "Flow calibration handler called with null message payload.");
        return;
    }
    command = reinterpret_cast<DripFlowCalibrationUpdateCommand_t*>(message.payload);
    valveManager_.getCurrentProcess(valveProcess);
    */
    
    /** Typecast the payload. */


    /** Retrieve valve status. */

    /** Handle state transition based on valve status. */
    //switch (valveProcess) {}


    return;
}

void MainStateController::handleFlowCalibrateMeasureRequestStateFlowCalibration(DripRxMessageId_e id, const DripRxMessage *message) {
    return;
}

void MainStateController::handleFlowCalibrateEndRequestStateFlowCalibration(DripRxMessageId_e id, const DripRxMessage *message) {
    return;
}

void MainStateController::handlePressureCalibrateUpdateRequestStateListen(DripRxMessageId_e id, const DripRxMessage *message) {
    return;
}

void MainStateController::handleDrainRequestStateListen(DripRxMessageId_e id, const DripRxMessage *message) {
    return;
}

void MainStateController::handlePressurePollRequestStateListen(DripRxMessageId_e id, const DripRxMessage *message) {
    return;
}



void MainStateController::preUpdate() {
    /** Update  */
    handleReceivedMessages();

}
void MainStateController::postUpdate() {
    esp_err_t err = ESP_OK;

    /** Upload all logs to MQTT. */
    err = mqttManager_.uploadLogs();
    if (err != ESP_OK) {
        dataContainer_.logError(err, TAG, "Failed to upload logs.");
    }
}



esp_err_t MainStateController::handleReceivedMessages() {
    /** TODO: Smart pointer */
    DripRxMessage *message = nullptr;
    esp_err_t err = ESP_OK;

    /** Check for new MQTT messages. */
    while(mqttManager_.numMessagesInQueue() > 0) {

        /** Get the next message from the queue. */
        err = mqttManager_.getNextMessage(message);
        if ( (err != ESP_OK) || (message == nullptr) ) {
            dataContainer_.logError(err, TAG, "Failed to retrieve MQTT message.");
            return ESP_FAIL;
        }

        /** Handle the message. */
        err = machine_.handleEvent((*message).id(), message);
        if (err != ESP_OK) {
            /* Invalid firmware*/
        }

        err = mqttManager_.freeMessage(message);
        if (err != ESP_OK) {
            return ESP_FAIL;
        }
    }

    return ESP_OK;
}


esp_err_t MainStateController::initializeFilesystem() {
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