#include "cstdio"
#include "array"

//#include <etlcpp/span.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"

//#include "utils.h"
//#include "dataContainer.h"
//#include "configManager.h"
//#include "mqttManager.h"
//#include "wifiManager.h"
//#include "valveManager.h"
//#include "flowManager.h"

#include "mainStateController.h"

#define ETL_NO_STL

static const char* TAG = "MainStateController";

/**
 * @brief State handler map.
 */
//constexpr StateHandlerMapPair<DripMainFsmState_e, MainStateController> stateToHandlerMapValues[] {

//};
using FsmStateMap = StateHandlerMap<DripMainFsmState_e, MainStateController>;
constexpr FsmStateMap stateToHandlerMap { 
    FsmStateMap::value_type{
        DripMainFsmState_e::Uninitialized, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::uninitializedEntry, 
            &MainStateController::uninitializedUpdate, 
            &MainStateController::uninitializedExit, 
            1U /** handlingIntervalMs. N/A, one-shot state. */
        ),
    },
    FsmStateMap::value_type{
        DripMainFsmState_e::Boot, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::bootEntry, 
            &MainStateController::bootUpdate, 
            &MainStateController::bootExit, 
            1U /** handlingIntervalMs. N/A, one-shot state. */
        ),
    },
    FsmStateMap::value_type{
        DripMainFsmState_e::FatalError, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::fatalErrorEntry, 
            &MainStateController::fatalErrorUpdate, 
            &MainStateController::fatalErrorExit, 
            1000U /** handlingIntervalMs. Idles without much need for response. */
        ),
    },
    FsmStateMap::value_type{
        DripMainFsmState_e::ConnectWifi, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::connectWifiEntry, 
            &MainStateController::connectWifiUpdate, 
            &MainStateController::connectWifiExit, 
            10U /** handlingIntervalMs. Quick handling of connection logic. */
        ),
    },
    FsmStateMap::value_type{
        DripMainFsmState_e::ConnectMqtt, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::connectMqttEntry, 
            &MainStateController::connectMqttUpdate, 
            &MainStateController::connectMqttExit, 
            10U /** handlingIntervalMs. Quick handling of connection logic. */
        ),
    },
    FsmStateMap::value_type{
        DripMainFsmState_e::Provisioning, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::provisioningEntry, 
            &MainStateController::provisioningUpdate, 
            &MainStateController::provisioningExit, 
            10U /** handlingIntervalMs. Quick handling of connection logic. */
        ),
    },
    FsmStateMap::value_type{
        DripMainFsmState_e::Restart, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::restartEntry, 
            &MainStateController::restartUpdate, 
            &MainStateController::restartExit, 
            1U /** handlingIntervalMs. N/A, one-shot state. */
        ),
    },
    FsmStateMap::value_type{
        DripMainFsmState_e::Listen, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::listenEntry, 
            &MainStateController::listenUpdate, 
            &MainStateController::listenExit, 
            250U /** handlingIntervalMs. Reasonably quick response time to messages. */
        ),
    },
    FsmStateMap::value_type{
        DripMainFsmState_e::Dispense, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::dispenseEntry, 
            &MainStateController::dispenseUpdate, 
            &MainStateController::dispenseExit, 
            100U /** handlingIntervalMs. Reasonably quick response time to dispense events. */
        ),
    },
    FsmStateMap::value_type{
        DripMainFsmState_e::FlowSensorCalibrate, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::flowSensorCalibrateEntry, 
            &MainStateController::flowSensorCalibrateUpdate, 
            &MainStateController::flowSensorCalibrateExit, 
            100U /** handlingIntervalMs. Reasonably quick response time to dispense events. */
        ),
    },
    FsmStateMap::value_type{
        DripMainFsmState_e::Drain, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::drainEntry, 
            &MainStateController::drainUpdate, 
            &MainStateController::drainExit, 
            100U /** handlingIntervalMs. Reasonably quick response time to dispense events. */
        ),
    }
};

/**
 * @brief Event handler map.
 */
constexpr EventHandlerMapPair<DripMainFsmState_e, DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController> eventToHandlerMapValues[] {

};
using FsmEventMap = EventHandlerMap<DripMainFsmState_e, DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>;
constexpr FsmEventMap eventToHandlerMap { 
    /**
    FsmEventMap::value_type{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::Listen, 
            DripRxMessageId_e::DispenseActivate
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
            &MainStateController::handleDispenseRequestStateListen
        )
    },
    */
    FsmEventMap::value_type{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::Dispense, 
            DripRxMessageId_e::Deactivate
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
            &MainStateController::handleDeactivateRequestStateDispenseDrainOrFlowCalibration
        )
    },
    /**
    FsmEventMap::value_type{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::Drain, 
            DripRxMessageId_e::Deactivate
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
            &MainStateController::handleDeactivateRequestStateDispenseDrainOrFlowCalibration
        )
    },
    FsmEventMap::value_type{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::FlowSensorCalibrate, 
            DripRxMessageId_e::Deactivate
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
            &MainStateController::handleDeactivateRequestStateDispenseDrainOrFlowCalibration
        )
    },
    etl::pair{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::Listen, 
            DripRxMessageId_e::Restart
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
            &MainStateController::handleRestartRequestStateListen
        )
    },
    FsmEventMap::value_type{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::Listen, 
            DripRxMessageId_e::ConfigUpdate
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
            &MainStateController::handleConfigUpdateRequestStateListen
        )
    },
    FsmEventMap::value_type{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::Listen, 
            DripRxMessageId_e::FlowCalibrateBegin
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
            &MainStateController::handleFlowCalibrateBeginRequestStateListen
        )
    },
    FsmEventMap::value_type{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::FlowSensorCalibrate, 
            DripRxMessageId_e::FlowCalibrateDispense
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
            &MainStateController::handleFlowCalibrateDispenseRequestStateFlowCalibration
        )
    },
    FsmEventMap::value_type{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::FlowSensorCalibrate, 
            DripRxMessageId_e::FlowCalibrateMeasure
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
            &MainStateController::handleFlowCalibrateMeasureRequestStateFlowCalibration
        )
    },
    FsmEventMap::value_type{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::FlowSensorCalibrate, 
            DripRxMessageId_e::FlowCalibrateEnd
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
            &MainStateController::handleFlowCalibrateEndRequestStateFlowCalibration
        )
    },
    FsmEventMap::value_type{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::Listen, 
            DripRxMessageId_e::PressureCalibrateUpdate
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
            &MainStateController::handlePressureCalibrateUpdateRequestStateListen
        )
    },
    FsmEventMap::value_type{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::Listen, 
            DripRxMessageId_e::DrainActivate
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
            &MainStateController::handleDrainRequestStateListen
        )
    },
    FsmEventMap::value_type{
        EventHandlerMapKey_t<DripMainFsmState_e, DripRxMessageId_e>(
            DripMainFsmState_e::Listen, 
            DripRxMessageId_e::PressurePoll
        ),
        EventHandlerMapEntry_t<DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
            &MainStateController::handlePressurePollRequestStateListen
        )
    }
    */
};


/**
* @brief Constructor
*/
MainStateController::MainStateController(
    DataContainer &dataContainer,
    ConfigManager &configManager
    //const WifiManager &wifiManager, 
    //const MqttManager &mqttManager,
    //const ValveManager &valveManager,
    //const FlowSensorManager &flowSensorManager
) :
    StateController<DripMainFsmState_e, DripRxMessageId_e, DripRxMessage<DripMainFsmState_e>, MainStateController>(
        DripMainFsmState_e::Uninitialized,
        "MainFsm",
        stateToHandlerMap,
        eventToHandlerMap,
        *this,
        dataContainer
    ),

    eventTimers_(),
    
    configManager_(configManager) {}
    //dataContainer_(dataContainer),
    //mqttManager_(mqttManager),
    //wifiManager_(wifiManager),
    //valveManager_(valveManager),
    //flowSensorManager_(flowSensorManager) {}


/**
 * @brief Handlers for state uninitialized.
 */
void MainStateController::uninitializedEntry() {
    esp_err_t err = ESP_OK;

    /** Initialize managers. */
    err = configManager_.initialize();
    if (err != ESP_OK) machine_.transition(DripMainFsmState_e::FatalError);

    //err = mqttManager_.initialize();
    //if (err != ESP_OK) machine_.transition(DripMainFsmState_e::FatalError);

    /** Enter boot. */
    //err = machine_.transition(DripMainFsmState_e::Boot);
    //if (err != ESP_OK) machine_.transition(DripMainFsmState_e::FatalError);

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
}
void MainStateController::connectWifiUpdate() {
}
void MainStateController::connectWifiExit() {
    return;
}
/**
 * @brief Handlers for state connect mqtt.
 */
void MainStateController::connectMqttEntry() {
}
void MainStateController::connectMqttUpdate() {
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
}
void MainStateController::provisioningExit() {
    return;
}

/**
 * @brief Handlers for state restart.
 */
void MainStateController::restartEntry() {
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
}
void MainStateController::dispenseExit() {
}

/**
 * @brief Handlers for state flow calibrate.
 */
void MainStateController::flowSensorCalibrateEntry() {
    return;
}
void MainStateController::flowSensorCalibrateUpdate() {

}
void MainStateController::flowSensorCalibrateExit() {
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

void MainStateController::handleDeactivateRequestStateDispenseDrainOrFlowCalibration(DripRxMessageId_e id, const DripRxMessage<DripMainFsmState_e> *message) {
    machine_.transition(DripMainFsmState_e::Listen);
    return;
}

void MainStateController::preUpdate() {
    /** Update  */
    handleReceivedMessages();

}
void MainStateController::postUpdate() {
    esp_err_t err = ESP_OK;

    /** Upload all logs to MQTT. */
    //err = mqttManager_.uploadLogs();
    //if (err != ESP_OK) {
        //dataContainer_.logError(err, TAG, "Failed to upload logs.");
    //}
}



esp_err_t MainStateController::handleReceivedMessages() {
    return ESP_OK;
}


esp_err_t MainStateController::initializeFilesystem() {
    return ESP_OK;
}