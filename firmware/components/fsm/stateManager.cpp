#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"

#include "configManager.h"
#include "mqttManager.h"
#include "connectionManager.h"
#include "messages.h"

#include "stateManager.h"

static const char* TAG = "StateManager";

/**
 * @brief Constructor
 */
StateManager::StateManager(ConfigManager *configManager, MqttManager *mqttManager, ConnectionManager *connectionManager) {
    state = STATE_MIN;
    configManager = configManager;
    mqttManager = mqttManager;
    connectionManager = connectionManager;
}

/**
 * @brief Initializes the finite state machine.
 */
void StateManager::initialize() {
    state = STATE_BOOT;
}

/**
 * @brief Executes the current state.
 */
void StateManager::handle_current_state() {
    switch (state) {
        case STATE_BOOT:
            boot();
            break;
        case STATE_FATAL_ERROR:
            fatalError();
            break;
        case STATE_CONNECT:
            connect();
            break;
        case STATE_PROVISIONING:
            accessPoint();
            break;
        case STATE_RESTART:
            restart();
            break;
        case STATE_LISTEN:
            listen();
            break;
        case STATE_SLEEP:
            sleep();
            break;
        case STATE_CONFIG:
            config();
            break;
        case STATE_DISPENSE_START:
            dispense_start();
            break;
        case STATE_DISPENSE_SOURCE:
            dispense_source();
            break;
        case STATE_DISPENSE_TANK:
            dispense_tank();
            break;
        case STATE_FLOW_CALIBRATE_DISPENSE:
            flow_calibrate_dispense();
            break;
        case STATE_FLOW_CALIBRATE_MEASURE:
            flow_calibrate_measure();
            break;
        case STATE_PRESSURE_CALIBRATE:
            pressure_calibrate();
            break;
        case STATE_PRESSURE_POLL:
            pressure_poll();
            break;
        case STATE_DRAIN:
            drain();
            break;
        default:
            ESP_LOGW(TAG, "State machine set to invalid state: {%d}", state);
            state = STATE_FATAL_ERROR;
            break;
    }
}

/**
 * @brief Handler for state STATE_BOOT.
 */
void StateManager::boot() {
    esp_err_t err = ESP_OK;

    /** Initialize managers. */
    err = configManager->initialize();
    if (err != ESP_OK) goto err;

    err = connectionManager->initialize();
    if (err != ESP_OK) goto err;

    err = mqttManager->initialize();
    if (err != ESP_OK) goto err;


    state = STATE_CONNECT;
    return;

err:
    state = STATE_FATAL_ERROR;
    return;
}

/**
 * @brief Handler for state STATE_FATAL_ERROR.
 */
void StateManager::fatalError() {
    // Placeholder for fatal error state logic
}

/**
 * @brief Handler for state STATE_CONNECT.
 */
void StateManager::connect() {
    esp_err_t err = ESP_OK;
    bool connected = false;
    
    /** Attempt connection. */
    err = connectionManager->connect(connected);
    if (err != ESP_OK) goto err;

    /** Continue if connected or provision WiFi & MQTT if not. */
    if (connected == true) {
        state = STATE_LISTEN;
        return;

    } else {
        state = STATE_PROVISIONING;
        return;
    }

err:
    state = STATE_FATAL_ERROR;
    return;
}

/**
 * @brief Handler for state STATE_PROVISIONING.
 */
void StateManager::accessPoint() {
    esp_err_t err = ESP_OK;

    /** Begin provisioning. */
    if ( (connectionManager->isProvisioning() == false) && (connectionManager->isConnected() == false) ) {
        err = connectionManager->beginProvisioning();
        if (err != ESP_OK) goto err;
    }

    /** Move forward once connected. */
    if (connectionManager->isConnected() == true) {
        state = STATE_LISTEN;
        return;
    }

err:
    state = STATE_FATAL_ERROR;
    return;
}

/**
 * @brief Handler for state STATE_RESTART.
 */
void StateManager::restart() {
    esp_err_t err = ESP_OK;
    
    /** Report reset to MQTT. */
    //err = mqttManager->info("Resetting the device.");
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to transmit reset notification.");
    }

    /** Reset device. */
    esp_restart();

    /** Should not reach here. */
    state = STATE_FATAL_ERROR;
    return;
}

/**
 * @brief Handler for state STATE_LISTEN.
 */
void StateManager::listen() {
    esp_err_t err = ESP_OK;
    MqttRxMessage_t* message = nullptr;
    Config_t* config = nullptr; 

    /** In case MQTT has not checked for any messages yet, wait until it has. */
    if (mqttManager->checkedForMessages() == false) {
        return;
    }
    
    /** Check for new MQTT messages. */
    err = mqttManager->getNextMessage(message);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to retrieve MQTT message.");
        return;
    }

    /** Handle message. */
    if (message != nullptr) {

    }

    /** Sleep. */
}

/**
 * @brief Handler for state STATE_SLEEP.
 */
void StateManager::sleep() {
    // Placeholder for sleep state logic
}

/**
 * @brief Handler for state STATE_CONFIG.
 */
void StateManager::config() { 
    // Placeholder for config state logic
}

/**
 * @brief Handler for state STATE_DISPENSE_START.
 */
void StateManager::dispense_start() {
    // Placeholder for dispense start state logic
}

/**
 * @brief Handler for state STATE_DISPENSE_SOURCE.
 */
void StateManager::dispense_source() {
    // Placeholder for dispense source state logic
}

/**
 * @brief Handler for state STATE_DISPENSE_TANK.
 */
void StateManager::dispense_tank() {
    // Placeholder for dispense tank state logic
}

/**
 * @brief Handler for state STATE_FLOW_CALIBRATE_DISPENSE.
 */
void StateManager::flow_calibrate_dispense() {
    // Placeholder for flow calibrate dispense state logic
}

/**
 * @brief Handler for state STATE_FLOW_CALIBRATE_MEASURE.
 */
void StateManager::flow_calibrate_measure() {
    // Placeholder for flow calibrate measure state logic
}

/**
 * @brief Handler for state STATE_PRESSURE_CALIBRATE.
 */
void StateManager::pressure_calibrate() {
    // Placeholder for pressure calibrate state logic
}

/**
 * @brief Handler for state STATE_PRESSURE_POLL.
 */
void StateManager::pressure_poll() {
    // Placeholder for pressure poll state logic
}

/**
 * @brief Handler for state STATE_DRAIN.
 */
void StateManager::drain() {
    // Placeholder for drain state logic
}
