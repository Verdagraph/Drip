#include "esp_err.h"

#include "valveManager.h"

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
            &ValveManager::deactivateTankEntry, 
            &ValveManager::deactivateTankUpdate, 
            &ValveManager::deactivateTankExit, 
        },
    },
};
using FsmStateMap = StateHandlerMap<DripValveManagerStateId_e, ValveManager>;
constexpr FsmStateMap stateToHandlerMap { stateToHandlerMapValues };

/**
 * @brief Event handler map.
 */
constexpr EventHandlerMapPair<DripValveManagerStateId_e, DripValveManagerEventId_e, DripRxMessage, ValveManager> eventToHandlerMapValues[] {
    etl::pair{
        EventHandlerMapKey_t<DripValveManagerStateId_e, DripValveManagerEventId_e>(
            DripValveManagerStateId_e::Idle, 
            DripValveManagerEventId_e::DispenseStart
        ),
        EventHandlerMapEntry_t<DripValveManagerEventId_e, DripRxMessage, ValveManager>(
            &ValveManager::handleDispenseRequestStateIdle
        )
    },
    etl::pair{
        EventHandlerMapKey_t<DripValveManagerStateId_e, DripValveManagerEventId_e>(
            DripValveManagerStateId_e::DispenseStart, 
            DripValveManagerEventId_e::Deactivate
        ),
        EventHandlerMapEntry_t<DripValveManagerEventId_e, DripRxMessage, ValveManager>(
            &ValveManager::handleDeactivateRequestStateDispenseOrDrain
        )
    },
    etl::pair{
        EventHandlerMapKey_t<DripValveManagerStateId_e, DripValveManagerEventId_e>(
            DripValveManagerStateId_e::DispenseSource, 
            DripValveManagerEventId_e::Deactivate
        ),
        EventHandlerMapEntry_t<DripValveManagerEventId_e, DripRxMessage, ValveManager>(
            &ValveManager::handleDeactivateRequestStateDispenseOrDrain
        )
    },
    etl::pair{
        EventHandlerMapKey_t<DripValveManagerStateId_e, DripValveManagerEventId_e>(
            DripValveManagerStateId_e::DispenseTank, 
            DripValveManagerEventId_e::Deactivate
        ),
        EventHandlerMapEntry_t<DripValveManagerEventId_e, DripRxMessage, ValveManager>(
            &ValveManager::handleDeactivateRequestStateDispenseOrDrain
        )
    },
    etl::pair{
        EventHandlerMapKey_t<DripValveManagerStateId_e, DripValveManagerEventId_e>(
            DripValveManagerStateId_e::DrainTank, 
            DripValveManagerEventId_e::Deactivate
        ),
        EventHandlerMapEntry_t<DripValveManagerEventId_e, DripRxMessage, ValveManager>(
            &ValveManager::handleDeactivateRequestStateDispenseOrDrain
        )
    },
    etl::pair{
        EventHandlerMapKey_t<DripValveManagerStateId_e, DripValveManagerEventId_e>(
            DripValveManagerStateId_e::Idle, 
            DripValveManagerEventId_e::DrainStart
        ),
        EventHandlerMapEntry_t<DripValveManagerEventId_e, DripRxMessage, ValveManager>(
            &ValveManager::handleDrainRequestStateIdle
        )
    },
};
using FsmEventMap = EventHandlerMap<DripValveManagerStateId_e, DripValveManagerEventId_e, DripRxMessage, ValveManager>;
constexpr FsmEventMap eventToHandlerMap { eventToHandlerMapValues };

ValveManager::ValveManager(
    DataContainer &dataContainer
) : 
    StateController<DripValveManagerStateId_e, DripValveManagerEventId_e, DripRxMessage, ValveManager>(
        DripValveManagerStateId_e::Idle,
        "MainFsm",
        stateToHandlerMap,
        eventToHandlerMap,
        *this,
        dataContainer
    ),

    dataContainer_(dataContainer),
    openValve_(DRIP_VALVE_OPEN_NONE) {}

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
    return;
}
void ValveManager::dispenseStartUpdate() {
    return;
}
void ValveManager::dispenseStartExit() {
    return;
}

/**
 * @brief Handlers for state dispense source.
 */
void ValveManager::dispenseSourceEntry() {
    return;
}
void ValveManager::dispenseSourceUpdate() {
    return;
}
void ValveManager::dispenseSourceExit() {
    return;
}

/**
 * @brief Handlers for state dispense tank.
 */
void ValveManager::dispenseTankEntry() {
    return;
}
void ValveManager::dispenseTankUpdate() {
    return;
}
void ValveManager::dispenseTankExit() {
    return;
}

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
void ValveManager::deactivateTankEntry() {
    return;
}
void ValveManager::deactivateTankUpdate() {
    return;
}
void ValveManager::deactivateTankExit() {
    return;
}
    
void ValveManager::handleDispenseRequestStateIdle(DripValveManagerEventId_e id, const DripRxMessage *message) {
    DripConfig_t config = {};
    VdgDispenseProcessData_t processData = {};
    esp_err_t err = ESP_OK; 
    
    err = dataContainer_.getConfig(config);
    if (err != ESP_OK) {
        // log error
        return err;
    }
    
    /** Validate config. */
    
    /** Validate parameters. */
    
    
    
    processData.state = DRIP_DISPENSE_FSM_INIT;
    processData.target = target;
    
    
    switch (config.valves.preferredDispenseValve) {
        case SOURCE_DISPENSE:
            err = openSourceDispenseValve();
            if (err != ESP_OK) {
                return err;
            }
            processData.state = DRIP_DISPENSE_FSM_SOURCE_DISPENSE;
            break;
        case TANK_DISPENSE:
            err = openTankDispenseValve();
            if (err != ESP_OK) {
                return err;
            }
            processData.state = DRIP_DISPENSE_FSM_TANK_DISPENSE;
            break;
        default:
            return ESP_ERR_INVALID_STATE;
    }
    
    
    
    
    
    
    currentProcess_ = DRIP_VALVES_DISPENSE;
    process = currentProcess_;
    return;
}

void ValveManager::handleDrainRequestStateIdle(DripValveManagerEventId_e id, const DripRxMessage *message) {
    return;
}

void ValveManager::handleDeactivateRequestStateDispenseOrDrain(DripValveManagerEventId_e id, const DripRxMessage *message) {
    return;
}


esp_err_t ValveManager::closeValves() {
    DripConfig_t config = {};
    DripDriverStatus_t driverStatus = {};
    esp_err_t err = ESP_OK;

    err = dataContainer_.getDriverStatus(driverStatus);
    if (err != ESP_OK) {
        return err;
    }

    if (driverStatus.sourceDispenseValveOnline) {
        err = setValveState(DripValves_e::SourceDispense, DRIP_GPIO_TOGGLE_OFF);
        if (err != ESP_OK) {
            dataContainer_.logError(ESP_FAIL, TAG, "Failed to close source dispense valve");
            return ESP_FAIL;
        }
    }
    if (driverStatus.tankDispenseValveOnline) {
        err = setValveState(DripValves_e::TankDispense, DRIP_GPIO_TOGGLE_OFF);
        if (err != ESP_OK) {
            dataContainer_.logError(ESP_FAIL, TAG, "Failed to close tank dispense valve");
            return ESP_FAIL;
        }
    }
    if (driverStatus.tankDrainValveOnline) {
        err = setValveState(DripValves_e::TankDrain, DRIP_GPIO_TOGGLE_OFF);
        if (err != ESP_OK) {
            dataContainer_.logError(ESP_FAIL, TAG, "Failed to close tank drain valve");
            return ESP_FAIL;
        }
    }
    
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
                dataContainer_.logWarning(ESP_ERR_INVALID_STATE, TAG, "Source dispense valve attempted to be closed while disabled.");
            }
            return ESP_ERR_INVALID_STATE;
        case DripValves_e::TankDispense:
            if (false == driverStatus.tankDispenseValveOnline) {
                dataContainer_.logWarning(ESP_ERR_INVALID_STATE, TAG, "Tank dispense valve attempted to be closed while disabled.");
            }
            return ESP_ERR_INVALID_STATE;
        case DripValves_e::TankDrain:
            if (false == driverStatus.tankDrainValveOnline) {
                dataContainer_.logWarning(ESP_ERR_INVALID_STATE, TAG, "Tank drain valve attempted to be closed while disabled.");
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

    return ESP_OK;
}