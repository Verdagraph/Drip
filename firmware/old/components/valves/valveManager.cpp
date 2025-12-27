#include "esp_err.h"
#include "freertos/task.h"

#include "valveManager.h"
#include "rxMessages.h"

static const char* TAG = "ValveManager";

/**
 * @brief State handler map.
 */
constexpr StateHandlerMapPair<DripValveManagerStateId_e, ValveManager> stateToHandlerMapValues[] {
    etl::pair{
        DripValveManagerStateId_e::Idle, 
        StateHandlerMapEntry_t<ValveManager>{
            &ValveManager::idleEntry, 
            &ValveManager::idleUpdate, 
            &ValveManager::idleUpdate,
        }
    },
    etl::pair{
        DripValveManagerStateId_e::DispenseStart, 
        StateHandlerMapEntry_t<ValveManager>{
            &ValveManager::dispenseStartEntry, 
            &ValveManager::dispenseStartUpdate, 
            &ValveManager::dispenseStartExit, 
        },
    },
    etl::pair{
        DripValveManagerStateId_e::DispenseSource, 
        StateHandlerMapEntry_t<ValveManager>{
            &ValveManager::dispenseSourceEntry, 
            &ValveManager::dispenseSourceUpdate, 
            &ValveManager::dispenseSourceExit, 
        },
    },
    etl::pair{
        DripValveManagerStateId_e::DispenseTank, 
        StateHandlerMapEntry_t<ValveManager>{
            &ValveManager::dispenseTankEntry, 
            &ValveManager::dispenseTankUpdate, 
            &ValveManager::dispenseTankExit, 
        },
    },
    etl::pair{
        DripValveManagerStateId_e::DispenseExit, 
        StateHandlerMapEntry_t<ValveManager>{
            &ValveManager::dispenseExitEntry, 
            &ValveManager::dispenseExitUpdate, 
            &ValveManager::dispenseExitExit, 
        },
    },
    etl::pair{
        DripValveManagerStateId_e::DrainTank, 
        StateHandlerMapEntry_t<ValveManager>{
            &ValveManager::drainTankEntry, 
            &ValveManager::drainTankUpdate, 
            &ValveManager::drainTankExit, 
        },
    },
    etl::pair{
        DripValveManagerStateId_e::Deactivate, 
        StateHandlerMapEntry_t<ValveManager>{
            &ValveManager::deactivateEntry, 
            &ValveManager::deactivateUpdate, 
            &ValveManager::deactivateExit, 
        },
    },
};
using FsmStateMap = StateHandlerMap<DripValveManagerStateId_e, ValveManager>;
constexpr FsmStateMap stateToHandlerMap { stateToHandlerMapValues };

/**
 * @brief Event handler map.
 */
constexpr EventHandlerMapPair<DripValveManagerStateId_e, DripValveManagerEventId_e, DripRxMessage<DripValveManagerEventId_e>, ValveManager> eventToHandlerMapValues[] {
    etl::pair{
        EventHandlerMapKey_t<DripValveManagerStateId_e, DripValveManagerEventId_e>(
            DripValveManagerStateId_e::Idle, 
            DripValveManagerEventId_e::DispenseStart
        ),
        EventHandlerMapEntry_t<DripValveManagerEventId_e, DripRxMessage<DripValveManagerEventId_e>, ValveManager>(
            &ValveManager::handleDispenseRequestStateIdle
        )
    },
    etl::pair{
        EventHandlerMapKey_t<DripValveManagerStateId_e, DripValveManagerEventId_e>(
            DripValveManagerStateId_e::DispenseStart, 
            DripValveManagerEventId_e::Deactivate
        ),
        EventHandlerMapEntry_t<DripValveManagerEventId_e, DripRxMessage<DripValveManagerEventId_e>, ValveManager>(
            &ValveManager::handleDeactivateRequestStateDispenseOrDrain
        )
    },
    etl::pair{
        EventHandlerMapKey_t<DripValveManagerStateId_e, DripValveManagerEventId_e>(
            DripValveManagerStateId_e::DispenseSource, 
            DripValveManagerEventId_e::Deactivate
        ),
        EventHandlerMapEntry_t<DripValveManagerEventId_e, DripRxMessage<DripValveManagerEventId_e>, ValveManager>(
            &ValveManager::handleDeactivateRequestStateDispenseOrDrain
        )
    },
    etl::pair{
        EventHandlerMapKey_t<DripValveManagerStateId_e, DripValveManagerEventId_e>(
            DripValveManagerStateId_e::DispenseTank, 
            DripValveManagerEventId_e::Deactivate
        ),
        EventHandlerMapEntry_t<DripValveManagerEventId_e, DripRxMessage<DripValveManagerEventId_e>, ValveManager>(
            &ValveManager::handleDeactivateRequestStateDispenseOrDrain
        )
    },
    etl::pair{
        EventHandlerMapKey_t<DripValveManagerStateId_e, DripValveManagerEventId_e>(
            DripValveManagerStateId_e::DrainTank, 
            DripValveManagerEventId_e::Deactivate
        ),
        EventHandlerMapEntry_t<DripValveManagerEventId_e, DripRxMessage<DripValveManagerEventId_e>, ValveManager>(
            &ValveManager::handleDeactivateRequestStateDispenseOrDrain
        )
    },
    etl::pair{
        EventHandlerMapKey_t<DripValveManagerStateId_e, DripValveManagerEventId_e>(
            DripValveManagerStateId_e::Idle, 
            DripValveManagerEventId_e::DrainStart
        ),
        EventHandlerMapEntry_t<DripValveManagerEventId_e, DripRxMessage<DripValveManagerEventId_e>, ValveManager>(
            &ValveManager::handleDrainRequestStateIdle
        )
    },
};
using FsmEventMap = EventHandlerMap<DripValveManagerStateId_e, DripValveManagerEventId_e, DripRxMessage<DripValveManagerEventId_e>, ValveManager>;
constexpr FsmEventMap eventToHandlerMap { eventToHandlerMapValues };

ValveManager::ValveManager(
    DataContainer &dataContainer
) : 
    StateController<DripValveManagerStateId_e, DripValveManagerEventId_e, DripRxMessage<DripValveManagerEventId_e>, ValveManager>(
        DripValveManagerStateId_e::Idle,
        "MainFsm",
        stateToHandlerMap,
        eventToHandlerMap,
        *this,
        dataContainer
    ),

    eventTimers_{},
    dataContainer_(dataContainer),
    openValve_(DripValves_e::Null) {}

esp_err_t ValveManager::initialize() {
    return ESP_OK;
}

bool ValveManager::isIdle() const {
    if (DripValveManagerStateId_e::Idle == machine_.getCurrentState() ) {
        return true;
    } else {
        return false;
    }
}
bool ValveManager::isDispensing() const {
    if ( 
        (DripValveManagerStateId_e::DispenseStart == machine_.getCurrentState() ) || 
        (DripValveManagerStateId_e::DispenseSource == machine_.getCurrentState() ) || 
        (DripValveManagerStateId_e::DispenseTank == machine_.getCurrentState() ) ) {
        return true;
    } else {
        return false;
    }
}
bool ValveManager::isDraining() const {
    if (DripValveManagerStateId_e::DrainTank == machine_.getCurrentState() ) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief Handlers for state idle.
 */
void idleEntry();
void idleUpdate();
void idleExit();

/**
 * @brief Handlers for state dispense start.
 */
void ValveManager::dispenseStartEntry() {
    DripConfig_t config = {};
    DripDriverStatus_t driverStatus = {};
    VdgDispenseProcessData_t processData = {};
    DripMeasurementData_t measurementData = {};
    DripDerivedData_t derivedData = {};
    DripValves_e valve = DripValves_e::Null;
    esp_err_t err = ESP_OK; 

    /** Retrieve container data. */
    err = dataContainer_.getConfig(config);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve config.");
        goto exit;
    }
    err = dataContainer_.getDriverStatus(driverStatus);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve driver status.");
        goto exit;
    }
    err = dataContainer_.getDispenseProcessData(processData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve dispense process data.");
        goto exit;
    }
    err = dataContainer_.getMeasurementData(measurementData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve measurement data.");
        goto exit;
    }
    err = dataContainer_.getDerivedData(derivedData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve derived data.");
        goto exit;
    }

    /** Select valve. */
    if (driverStatus.sourceDispenseValveOnline && driverStatus.tankDispenseValveOnline) {
        if (DripValves_e::SourceDispense == config.valves.preferredDispenseValve) {
            valve = DripValves_e::SourceDispense;
        } else if (DripValves_e::TankDispense == config.valves.preferredDispenseValve) {
            valve = DripValves_e::TankDispense;
        } else {
            dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Invalid value of config.valves.preferredDispenseValve: %d", config.valves.preferredDispenseValve);
            goto exit;

        }
    } else if (driverStatus.sourceDispenseValveOnline) {
        valve = DripValves_e::SourceDispense;
    } else if (driverStatus.tankDispenseValveOnline) {
        valve = DripValves_e::TankDispense;
    } else {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Attempted to enter dispensation with no valves online.");
        goto exit;
    }
    
    /** Update event timers. */
    eventTimers_ = {};
    eventTimers_.dispenseBeganTicks = xTaskGetTickCount();

    /** Reset flow measurement data. */
    measurementData.flowSensorPulses = 0U;
    err = dataContainer_.setMeasurementData(measurementData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to set measurement data.");
        goto exit;
    }

    /** Update process data. */
    processData.summary.initialTankLevel = derivedData.tankLevel;
    processData.summary.initialTankVolume = derivedData.tankVolume;
    err = dataContainer_.setDispenseProcessData(processData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to set dispense process data.");
        goto exit;
    }

    /** Transistion to next state. */
    switch (valve) {
    case DripValves_e::SourceDispense:
        machine_.transition(DripValveManagerStateId_e::DispenseSource);
        return;
    case DripValves_e::TankDispense:
        machine_.transition(DripValveManagerStateId_e::DispenseTank);
        return;
    default:
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Invalid valve state.");
        goto exit;
    }

    return;

exit:
    machine_.transition(DripValveManagerStateId_e::DispenseExit);
    return;
}
void ValveManager::dispenseStartUpdate() {}
void ValveManager::dispenseStartExit() {}

/**
 * @brief Handlers for state dispense source.
 */
void ValveManager::dispenseSourceEntry() {
    esp_err_t err = ESP_OK;

    err = openSourceDispenseValve();
    if (ESP_OK != err) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to open source dispense valve.");
        goto exit;
    }

    /** Update event timers. */
    eventTimers_.dispenseSourceBeganTicks = xTaskGetTickCount();

    return;

exit:
    machine_.transition(DripValveManagerStateId_e::DispenseExit);
    return;
}
void ValveManager::dispenseSourceUpdate() {
    DripConfig_t config = {};
    DripDriverStatus_t driverStatus = {};
    VdgDispenseProcessData_t processData = {};
    VdgDispenseProcessSlice_t previousSlice = {};
    DripMeasurementData_t measurementData = {};
    DripDerivedData_t derivedData = {};
    DripValves_e valve = DripValves_e::Null;
    TickType_t currentTick = 0U;
    esp_err_t err = ESP_OK; 

    /** Update process slice data. */
    err = updateDispenseProcessSlice();
    if (ESP_OK != err) {
        goto exit;
    }

    /** Retrieve container data. */
    err = dataContainer_.getConfig(config);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve config.");
        goto exit;
    }
    err = dataContainer_.getDriverStatus(driverStatus);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve driver status.");
        goto exit;
    }
    err = dataContainer_.getDispenseProcessData(processData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve dispense process data.");
        goto exit;
    }
    err = dataContainer_.getMeasurementData(measurementData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve measurement data.");
        goto exit;
    }
    err = dataContainer_.getDerivedData(derivedData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve derived data.");
        goto exit;
    }


    /** Check target. */
    if ( (DripDispenseProcessTargetType_e::Seconds == processData.target.targetType) && ( (processData.slice.timeMs / 1000.0f) >= processData.target.target) ) {

        dataContainer_.logInfo(ESP_OK, TAG, "Dispense process hit target of %.2f seconds.", processData.target.target);
        goto exit;

    } else if ( (DripDispenseProcessTargetType_e::Liters == processData.target.targetType) && ( (processData.slice.outputVolume) >= processData.target.target) ) {

        dataContainer_.logInfo(ESP_OK, TAG, "Dispense process hit target of %.2f liters.", processData.target.target);
        goto exit;

    }

    /** Enforce timeout condition. */
    if (processData.slice.timeMs > processData.target.timeoutMs) {
        dataContainer_.logInfo(ESP_OK, TAG, "Dispense process timed out at %d miliseconds.", processData.slice.timeMs);
        goto exit;
    }

    /** Determine if the source is empty. */
    currentTick = xTaskGetTickCount();
    if (processData.slice.flowRate >= config.flowSensor.minFlowRateLps) {
        eventTimers_.lastFlowSensorAboveMinimumFlowTicks = currentTick;
    } else if ( (currentTick - eventTimers_.lastFlowSensorAboveMinimumFlowTicks) > pdMS_TO_TICKS(config.flowSensor.minFlowRateTimeoutMs) ) {

        if ()

    }

exit:
    machine_.transition(DripValveManagerStateId_e::DispenseExit);
    return;
}
void ValveManager::dispenseSourceExit() {
    esp_err_t err = ESP_OK;

    
    return;

exit:
    machine_.transition(DripValveManagerStateId_e::DispenseExit);
    return;
}

/**
 * @brief Handlers for state dispense tank.
 */
void ValveManager::dispenseTankEntry() {
    esp_err_t err = ESP_OK;

    err = openSourceDispenseValve();
    if (ESP_OK != err) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to open source dispense valve.");
        goto exit;
    }

    /** Update event timers. */
    eventTimers_.dispenseTankBeganTicks = xTaskGetTickCount();
    
    return;

exit:
    machine_.transition(DripValveManagerStateId_e::DispenseExit);
    return;
}
void ValveManager::dispenseTankUpdate() {
    return;
}
void ValveManager::dispenseTankExit() {
    DripConfig_t config = {};
    DripDriverStatus_t driverStatus = {};
    VdgDispenseProcessData_t processData = {};
    VdgDispenseProcessSlice_t previousSlice = {};
    DripMeasurementData_t measurementData = {};
    DripDerivedData_t derivedData = {};
    DripValves_e valve = DripValves_e::Null;
    esp_err_t err = ESP_OK; 

    /** Retrieve container data. */
    err = dataContainer_.getConfig(config);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve config.");
        goto exit;
    }
    err = dataContainer_.getDriverStatus(driverStatus);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve driver status.");
        goto exit;
    }
    err = dataContainer_.getDispenseProcessData(processData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve dispense process data.");
        goto exit;
    }
    err = dataContainer_.getMeasurementData(measurementData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve measurement data.");
        goto exit;
    }
    err = dataContainer_.getDerivedData(derivedData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve derived data.");
        goto exit;
    }

    /** Update process summary data. */
    /** can't assume tank is being used first - track source and tank outputs seperately */
    //processData.summary.outputTankVolume = processData.slice.outputVolume;
    err = dataContainer_.setDispenseProcessData(processData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to set dispense process data.");
        goto exit;
    }

    return;

exit:
    machine_.transition(DripValveManagerStateId_e::DispenseExit);
    return;
}

/**
 * @brief Handlers for state dispense exit.
 */
void ValveManager::dispenseExitEntry() {
    DripConfig_t config = {};
    DripDriverStatus_t driverStatus = {};
    VdgDispenseProcessData_t processData = {};
    VdgDispenseProcessSlice_t previousSlice = {};
    DripMeasurementData_t measurementData = {};
    DripDerivedData_t derivedData = {};
    DripValves_e valve = DripValves_e::Null;
    esp_err_t err = ESP_OK; 

    /** Update process slice data. */
    err = updateDispenseProcessSlice();
    if (ESP_OK != err) {
        goto exit;
    }

    /** Retrieve container data. */
    err = dataContainer_.getConfig(config);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve config.");
        goto exit;
    }
    err = dataContainer_.getDriverStatus(driverStatus);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve driver status.");
        goto exit;
    }
    err = dataContainer_.getDispenseProcessData(processData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve dispense process data.");
        goto exit;
    }
    err = dataContainer_.getMeasurementData(measurementData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve measurement data.");
        goto exit;
    }
    err = dataContainer_.getDerivedData(derivedData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve derived data.");
        goto exit;
    }

    /** Update process summary data. */
    processData.summary.durationMs = processData.slice.timeMs;
    processData.summary.outputVolume = processData.slice.outputVolume;
    processData.summary.finalTankLevel = processData.slice.tankLevel;
    processData.summary.finalTankVolume = processData.slice.tankVolume;
    err = dataContainer_.setDispenseProcessData(processData);
    if (ESP_OK != err) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to set dispense process data.");
        goto exit;
    }

    machine_.transition(DripValveManagerStateId_e::Deactivate);
    return;

exit:
    machine_.transition(DripValveManagerStateId_e::Deactivate);
    return;
}
void ValveManager::dispenseExitUpdate() {}
void ValveManager::dispenseExitExit() {}

/**
 * @brief Handlers for state drain tank.
 */
void ValveManager::drainTankEntry() {
    return;
}
void ValveManager::drainTankUpdate() {
    return;
}
void ValveManager::drainTankExit() {
    return;
}

/**
 * @brief Handlers for state deactivate.
 */
void ValveManager::deactivateEntry() {
    esp_err_t err = ESP_OK;

    err = closeValves();
    if (ESP_OK != err) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to deactivate valves.");
        goto exit;
    }

exit:
    machine_.transition(DripValveManagerStateId_e::Idle);
    return;
}
void ValveManager::deactivateUpdate() {}
void ValveManager::deactivateExit() {}
    
void ValveManager::handleDispenseRequestStateIdle(DripValveManagerEventId_e id, const DripRxMessage<DripValveManagerEventId_e> *message) {
    DripConfig_t config = {};
    DripDriverStatus_t driverStatus = {};
    const DispenseActivateRxMessage *command = nullptr;
    VdgDispenseProcessData_t processData = {};
    esp_err_t err = ESP_OK; 

    /** Validate state. */
    if (DripValveManagerStateId_e::Idle != machine_.getCurrentState()) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "DispenseActivateRxMessage handler called in invalid state.");
        return;
    }
    
    /** Validate payload. */
    if (nullptr == message){
        dataContainer_.logError(ESP_ERR_INVALID_ARG, TAG, "Null message passed to handleDispenseRequestStateIdle.");
        return;
    }
    if (message->id() != DripValveManagerEventId_e::DispenseStart) {
        dataContainer_.logError(ESP_ERR_INVALID_ARG, TAG, "Incorrect message type: %d passed to handleDispenseRequestStateIdle.", message->id());
        return; 
    }

    /** Typecast the payload. */
    command = reinterpret_cast<const DispenseActivateRxMessage*>(message);
    VdgDispenseProcessTarget_t target = command->data();
    
    /** Retrieve container data. */
    err = dataContainer_.getConfig(config);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve config.");
        return;
    }
    err = dataContainer_.getDriverStatus(driverStatus);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve driver status.");
        return;
    }
    
    /** Validate config. */
    if ( (false == driverStatus.sourceDispenseValveOnline) && 
         (false == driverStatus.tankDispenseValveOnline) ) {
        dataContainer_.logWarning(ESP_ERR_INVALID_STATE, TAG, "Dispense attempted while both dispense valves are disabled.");
        return;
    }
    
    if ( (false == driverStatus.sourceDispenseValveOnline) && 
         (config.valves.preferredDispenseValve == DripValves_e::SourceDispense) ) {
        dataContainer_.logWarning(ESP_ERR_INVALID_STATE, TAG, "Source dispense valve is preferred but disabled.");
    } else if ( (false == driverStatus.tankDispenseValveOnline) && 
                (config.valves.preferredDispenseValve == DripValves_e::SourceDispense) ) {
        dataContainer_.logWarning(ESP_ERR_INVALID_STATE, TAG, "Tank dispense valve is preferred but disabled.");
    }
        
    /** Validate parameters. */
    if ( (DripDispenseProcessTargetType_e::Liters == target.targetType) && 
         (false == driverStatus.flowSensorOnline) ) {
        dataContainer_.logWarning(ESP_ERR_INVALID_ARG, TAG, "Dispense requested with a volume target but the flow sensor is disabled.");
        return;
    }

    if (target.target <= 0.0f) {
        dataContainer_.logWarning(ESP_ERR_INVALID_ARG, TAG, "Dispense requested with null target.");
        return;
    } else if ( (target.targetType == DripDispenseProcessTargetType_e::Seconds) && 
                (target.target > config.valves.maxDispenseTargetSeconds) ) {
        dataContainer_.logWarning(ESP_ERR_INVALID_ARG, TAG, "Dispense requested with target greater than the maximum of %.2f seconds.", config.valves.maxDispenseTargetSeconds);
        return;
    } else if ( (target.targetType == DripDispenseProcessTargetType_e::Liters) && 
                (target.target > config.valves.maxDispenseTargetLiters) ) {
        dataContainer_.logWarning(ESP_ERR_INVALID_ARG, TAG, "Dispense requested with target greater than the maximum of %.2f liters.", config.valves.maxDispenseTargetLiters);
        return;
    }

    if (target.timeoutMs <= 0) {
        dataContainer_.logWarning(ESP_OK, TAG, "Dispense requested with null timeout, using default of %d minutes.", config.valves.defaultDispenseTimeoutMin);
        target.timeoutMs = config.valves.defaultDispenseTimeoutMin * 60U * 1000U;
    }

    /* Update the process data. */
    processData.target = target;
    err = dataContainer_.setDispenseProcessData(processData);
    if (ESP_OK != err) {
        dataContainer_.logError(err, TAG, "Failed to set dispense process data.");
        return;
    }

    machine_.transition(DripValveManagerStateId_e::DispenseStart);
    return;
}

void ValveManager::handleDrainRequestStateIdle(DripValveManagerEventId_e id, const DripRxMessage<DripValveManagerEventId_e> *message) {
    return;
}

void ValveManager::handleDeactivateRequestStateDispenseOrDrain(DripValveManagerEventId_e id, const DripRxMessage<DripValveManagerEventId_e> *message) {
    return;
}


esp_err_t ValveManager::closeValves() {
    DripConfig_t config = {};
    DripDriverStatus_t driverStatus = {};
    bool failed = false;
    esp_err_t err = ESP_OK;

    err = dataContainer_.getDriverStatus(driverStatus);
    if (err != ESP_OK) {
        return err;
    }

    err = setValveState(DripValves_e::SourceDispense, DRIP_GPIO_TOGGLE_OFF);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_FAIL, TAG, "Failed to close source dispense valve");
        failed = true;
    }
    err = setValveState(DripValves_e::TankDispense, DRIP_GPIO_TOGGLE_OFF);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_FAIL, TAG, "Failed to close tank dispense valve");
        failed = true;
    }
    err = setValveState(DripValves_e::TankDrain, DRIP_GPIO_TOGGLE_OFF);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_FAIL, TAG, "Failed to close tank drain valve");
        failed = true;
    }
    if (failed) {
        return ESP_FAIL;
    }

    openValve_ = DripValves_e::Null;
    
    return ESP_OK;
}

esp_err_t ValveManager::openSourceDispenseValve() {
    DripDriverStatus_t driverStatus = {};
    esp_err_t err = ESP_OK;

    err = dataContainer_.getDriverStatus(driverStatus);
    if (err != ESP_OK) {
        return err;
    }

    /** Close valves. */
    err = closeValves();
    if (ESP_OK != err) {
        dataContainer_.logError(ESP_FAIL, TAG, "Failed to close all valves prior to opening source dispense valve.");
        return ESP_FAIL;
    }
    
    /** Open valve. */
    if (driverStatus.sourceDispenseValveOnline) {
        err = setValveState(DripValves_e::SourceDispense, DRIP_GPIO_TOGGLE_ON);
        if (err != ESP_OK) {
            dataContainer_.logError(ESP_FAIL, TAG, "Failed to open source dispense valve");
            return ESP_FAIL;
        }
    } else {
        dataContainer_.logWarning(ESP_ERR_INVALID_STATE, TAG, "Source dispense valve attempted to be closed while disabled.");
        return ESP_ERR_INVALID_STATE;
    }
    
    return ESP_OK;
}

esp_err_t ValveManager::openTankDispenseValve() {
    DripDriverStatus_t driverStatus = {};
    esp_err_t err = ESP_OK;

    err = dataContainer_.getDriverStatus(driverStatus);
    if (err != ESP_OK) {
        return err;
    }

    /** Close valves. */
    err = closeValves();
    if (ESP_OK != err) {
        dataContainer_.logError(ESP_FAIL, TAG, "Failed to close all valves prior to opening tank dispense valve.");
        return ESP_FAIL;
    }
    
    /** Open valve. */
    if (driverStatus.tankDispenseValveOnline) {
        err = setValveState(DripValves_e::TankDispense, DRIP_GPIO_TOGGLE_ON);
        if (err != ESP_OK) {
            dataContainer_.logError(ESP_FAIL, TAG, "Failed to open tank dispense valve");
            return ESP_FAIL;
        }
    } else {
        dataContainer_.logWarning(ESP_ERR_INVALID_STATE, TAG, "Tank dispense valve attempted to be closed while disabled.");
        return ESP_ERR_INVALID_STATE;
    }
    
    return ESP_OK;
}

esp_err_t ValveManager::openTankDrainValve() {
    DripDriverStatus_t driverStatus = {};
    esp_err_t err = ESP_OK;

    err = dataContainer_.getDriverStatus(driverStatus);
    if (err != ESP_OK) {
        return err;
    }

    /** Close valves. */
    err = closeValves();
    if (ESP_OK != err) {
        dataContainer_.logError(ESP_FAIL, TAG, "Failed to close all valves prior to opening tank drain valve.");
        return ESP_FAIL;
    }
    
    /** Open valve. */
    if (driverStatus.tankDrainValveOnline) {
        err = setValveState(DripValves_e::TankDrain, DRIP_GPIO_TOGGLE_ON);
        if (err != ESP_OK) {
            dataContainer_.logError(ESP_FAIL, TAG, "Failed to open tank drain valve");
            return ESP_FAIL;
        }
    } else {
        dataContainer_.logWarning(ESP_ERR_INVALID_STATE, TAG, "Tank drain valve attempted to be closed while disabled.");
        return ESP_ERR_INVALID_STATE;
    }
    
    return ESP_OK;
}

esp_err_t ValveManager::setValveState(DripValves_e valve, DripGpioToggleState_e state) {
    DripConfig_t config = {};
    DripDriverStatus_t driverStatus = {};
    DripRelays_e relay = DripRelays_e::Relay1;
    esp_err_t err = ESP_OK;

    if (DripValves_e::Null == valve) {
        return ESP_ERR_INVALID_ARG;
    }

    err = dataContainer_.getConfig(config);
    if (err != ESP_OK) {
        return err;
    }
    err = dataContainer_.getDriverStatus(driverStatus);
    if (err != ESP_OK) {
        return err;
    }

    /** Validate status. */
    switch (valve) {
        case DripValves_e::SourceDispense:
            if (false == driverStatus.sourceDispenseValveOnline) {
                dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Source dispense valve attempted to be closed while disabled.");
            }
            return ESP_ERR_INVALID_STATE;
        case DripValves_e::TankDispense:
            if (false == driverStatus.tankDispenseValveOnline) {
                dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Tank dispense valve attempted to be closed while disabled.");
            }
            return ESP_ERR_INVALID_STATE;
        case DripValves_e::TankDrain:
            if (false == driverStatus.tankDrainValveOnline) {
                dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Tank drain valve attempted to be closed while disabled.");
            }
            return ESP_ERR_INVALID_STATE;
        default:
            return ESP_ERR_INVALID_STATE;
    }

    /** Retrieve relay. */
    switch (valve) {
        case DripValves_e::SourceDispense:
            relay = config.valves.sourceDispenseRelay;
            break;
        case DripValves_e::TankDispense:
            relay = config.valves.tankDispenseRelay;
            break;
        case DripValves_e::TankDrain:
            relay = config.valves.tankDrainRelay;
            break;
        default:
            return ESP_ERR_INVALID_STATE;
    }

    /** Set relay state. */
    err = gpioDriver.setRelayState(relay, state);
    if (ESP_OK != err) {
        return err;
    }
    
    if (DRIP_GPIO_TOGGLE_ON == state) {
        openValve_ = valve;
    }

    return ESP_OK;
}

esp_err_t ValveManager::updateDispenseProcessSlice() {
    VdgDispenseProcessData_t processData = {};
    VdgDispenseProcessSlice_t previousSlice = {};
    DripDerivedData_t derivedData = {};
    esp_err_t err = ESP_OK; 

    /** Retrieve container data. */
    err = dataContainer_.getDerivedData(derivedData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve derived data.");
        return err;
    }
    err = dataContainer_.getDispenseProcessData(processData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to retrieve dispense process data.");
        return err;
    }

    /** Update process variables. */
    previousSlice = processData.slice;

    processData.slice.openValve = openValve_;
    processData.slice.timeMs = pdTICKS_TO_MS(xTaskGetTickCount() - eventTimers_.dispenseBeganTicks);
    processData.slice.outputVolume = derivedData.volumeOutputLiters;
    processData.slice.flowRate = (processData.slice.outputVolume - previousSlice.outputVolume) / 
                                 ( (processData.slice.timeMs - previousSlice.timeMs) / 1000U);
    processData.slice.tankLevel = derivedData.tankLevel;
    processData.slice.tankVolume = derivedData.tankVolume;

    /** Set container data. */
    err = dataContainer_.setDispenseProcessData(processData);
    if (err != ESP_OK) {
        dataContainer_.logError(ESP_ERR_INVALID_STATE, TAG, "Failed to set process data.");
        return err;
    }

    return;
}