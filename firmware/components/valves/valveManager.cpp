#include "esp_err.h"

#include "valveManager.h"

static const char* TAG = "ValveManager";

/**
 * @brief Constructor.
 */
ValveManager::ValveManager(ConfigManager &configManager, DataContainer &dataContainer) {
    configManager_ = configManager;
    dataContainer_ = dataContainer;

    openValve_ = VDG_VALVE_OPEN_NONE;
    currentProcess_ = VDG_VALVES_MIN;
}

/**
 * @brief Begin the ValveManager.
 * 
 * @return esp_err_t Return code. 
 */
esp_err_t ValveManager::initialize() {
    openValve_ = VDG_VALVE_OPEN_NONE;
    currentProcess_ = VDG_VALVES_IDLE;
    return ESP_OK;
}

/**
 * @brief Begins a dispensation process.
 * 
 * @param[in] target Target for the process.
 * @param[out] process Output parameter for the current process after this function.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::beginDispenseProcess(VdgDispenseProcessTarget_t &target, VdgValveProcess_e &process) {
    Config_t config = {};
    VdgDispenseProcessData_t processData = {};
    esp_err_t err = ESP_OK; 

    err = configManager_.getConfig(config);
    if (err != ESP_OK) {
        // log error
        return err;
    }
    
    /** Validate config. */

    /** Validate parameters. */
    

    
    processData.state = VDG_DISPENSE_FSM_INIT;
    processData.target = target;


    switch (config.valves.preferredDispenseValve) {
        case SOURCE_DISPENSE:
            err = openSourceDispenseValve();
            if (err != ESP_OK) {
                return err;
            }
            processData.state = VDG_DISPENSE_FSM_SOURCE_DISPENSE;
            break;
        case TANK_DISPENSE:
            err = openTankDispenseValve();
            if (err != ESP_OK) {
                return err;
            }
            processData.state = VDG_DISPENSE_FSM_TANK_DISPENSE;
            break;
        default:
            return ESP_ERR_INVALID_STATE;
    }


    


    
    currentProcess_ = VDG_VALVES_DISPENSE;
    process = currentProcess_;
    return ESP_OK;
}

/**
 * @brief Begins a drain process.
 * 
 * @param[in] target Target for the process.
 * @param[out] process Output parameter for the current process after this function.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::beginDrainProcess(VdgDrainProcessTarget_t &target, VdgValveProcess_e &process){
    return ESP_OK;
}

/**
 * @brief Updates the current process.
 * 
 * @param[out] process Output parameter for the current process after this function.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::updateProcess(VdgValveProcess_e &process){
    switch(currentProcess_) {
        case VDG_VALVES_IDLE:
            return ESP_OK;
            break;
        case VDG_VALVES_DISPENSE:

            /** Handle timeout. */

            /** Handle low flow. */

            break;
        case VDG_VALVES_DRAIN:
            break;
        default:
            break;
    }

    return ESP_OK;
}

/**
 * @brief Ends the current process.
 * 
 * @param[out] process Output parameter for the current process after this function.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::endProcess(VdgValveProcess_e &process){
    return ESP_OK;
}

esp_err_t ValveManager::getCurrentProcess(VdgValveProcess_e &process){
    process = currentProcess_;
    return ESP_OK;
}

esp_err_t ValveManager::setValveState(Valves_e valve, VdgGpioToggleState_e state) {
    Config_t config = {};
    Relays_e relay = RELAYS_MIN;
    esp_err_t err = ESP_OK;

    err = configManager_.getConfig(config);
    if (err != ESP_OK) {
        // log error
        return err;
    }

    switch (valve) {
        case SOURCE_DISPENSE:
            relay = config.valves.sourceDispenseRelay;
            break;
        case TANK_DISPENSE:
            relay = config.valves.tankDispenseRelay;
            break;
        case TANK_DRAIN:
            relay = config.valves.tankDrainRelay;
            break;
        default:
            return ESP_ERR_INVALID_STATE;
    }

    switch (relay) {
        case RELAY1:
            err = gpioDriver.setRelay1State(state);
            break;
        case RELAY2:
            err = gpioDriver.setRelay2State(state);
            break;
        case RELAY3:
            err = gpioDriver.setRelay3State(state);
            break;
        default:
            return ESP_ERR_INVALID_STATE;
    }

    return err;
}

/**
 * @brief Closes the currently open valve.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::closeValves() {
    Config_t config = {};
    esp_err_t err = ESP_OK;

    err = configManager_.getConfig(config);
    if (err != ESP_OK) {
        // log error
        return err;
    }

    if (config.valves.sourceDispenseEnabled) {
        err = setValveState(SOURCE_DISPENSE, VDG_GPIO_TOGGLE_OFF);
        if (err != ESP_OK) {
            //log error
        }
    }
    if (config.valves.tankDispenseEnabled) {
        err = setValveState(TANK_DISPENSE, VDG_GPIO_TOGGLE_OFF);
        if (err != ESP_OK) {
            //log error
        }
    }
    if (config.valves.tankDrainEnabled) {
        err = setValveState(TANK_DRAIN, VDG_GPIO_TOGGLE_OFF);
        if (err != ESP_OK) {
            //log error
        }
    }

    return ESP_OK;
}

/**
 * @brief Open the source dispense valve.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::openSourceDispenseValve() {
    Config_t config = {};
    esp_err_t err = ESP_OK;

    err = configManager_.getConfig(config);
    if (err != ESP_OK) {
        // log error
        return err;
    }
    
    if (config.valves.sourceDispenseEnabled) {
        err = setValveState(SOURCE_DISPENSE, VDG_GPIO_TOGGLE_ON);
        if (err != ESP_OK) {
            //log error
        }
    }

    return ESP_OK;
}

/**
 * @brief Open the tank dispense valve.
 * 
 * @return esp_err_t Return code. 
 */
esp_err_t ValveManager::openTankDispenseValve() {
    Config_t config = {};
    esp_err_t err = ESP_OK;

    if (config.valves.tankDispenseEnabled) {
        err = setValveState(TANK_DISPENSE, VDG_GPIO_TOGGLE_ON);
        if (err != ESP_OK) {
            //log error
        }
    }

    return ESP_OK;
}

/**
 * @brief Open the tank drain valve.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t ValveManager::openTankDrainValve() {
    Config_t config = {};
    esp_err_t err = ESP_OK;

    if (config.valves.tankDrainEnabled) {
        err = setValveState(TANK_DRAIN, VDG_GPIO_TOGGLE_ON);
        if (err != ESP_OK) {
            //log error
        }
    }

    return ESP_OK;
}
