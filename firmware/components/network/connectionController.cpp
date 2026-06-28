//#include "cstdio"
//#include "array"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"


#include "connectionController.h"

static const char* TAG = "ConnectionController";

/**
 * @brief State handler map.
 */
using FsmStateMap = StateHandlerMap<DripConnectionControllerState_e, ConnectionController>;
constexpr FsmStateMap stateToHandlerMap { 
    FsmStateMap::value_type{
        DripMainFsmState_e::Uninitialized, 
        StateHandlerMapEntry_t<MainStateController>(
            &MainStateController::uninitializedEntry, 
            &MainStateController::uninitializedUpdate, 
            &MainStateController::uninitializedExit, 
            1U /** handlingIntervalMs. N/A, one-shot state. */
        ),
    }
};

/**
 * @brief Event handler map.
 */
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
ConnectionController::ConnectionController(DataContainer &dataContainer) :
    StateController<DripConnectionControllerState_e, NULL, NULL, ConnectionController>(
        DripMainFsmState_e::Uninitialized,
        "ConnectionController",
        stateToHandlerMap,
        eventToHandlerMap,
        *this,
        dataContainer
    ),

    dataContainer_(dataContainer) {}

/**
 * @brief Handlers for state uninitialized.
 */
void ConnectionController::uninitializedEntry() {}
void ConnectionController::uninitializedUpdate() {}
void ConnectionController::uninitializedExit() {}

/**
 * @brief Handlers for state fatal error.
 */
void ConnectionController::fatalErrorEntry() {}
void ConnectionController::fatalErrorUpdate() {}
void ConnectionController::fatalErrorExit() {}

/**
 * @brief Handlers for state provisioning.
 */
void ConnectionController::provisioningEntry() {}
void ConnectionController::provisioningUpdate() {}
void ConnectionController::provisioningExit() {}

/**
 * @brief Handlers for state disconnected.
 */
void ConnectionController::disconnectedBeginEntry() {}
void ConnectionController::disconnectedBeginUpdate() {}
void ConnectionController::disconnectedBeginExit() {}

/**
 * @brief Handlers for state connect begin.
 */
void ConnectionController::connectBeginEntry() {}
void ConnectionController::connectBeginUpdate() {}
void ConnectionController::connectBeginExit() {}

/**
 * @brief Handlers for state connect Wifi.
 */
void ConnectionController::connectWifiEntry() {}
void ConnectionController::connectWifiUpdate() {}
void ConnectionController::connectWifiExit() {}

/**
 * @brief Handlers for state connect MQTT.
 */
void ConnectionController::connectMqttEntry() {}
void ConnectionController::connectMqttUpdate() {}
void ConnectionController::connectMqttExit() {}

/**
 * @brief Handlers for state connected.
 */
void ConnectionController::connectedEntry() {}
void ConnectionController::connectedUpdate() {}
void ConnectionController::connectedExit() {}