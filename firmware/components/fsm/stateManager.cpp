#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"

#include "configManager.h"
#include "mqttManager.h"
#include "connectionManager.h"
#include "valveManager.h"
#include "messages.h"

#include "stateManager.h"

static const char* TAG = "StateManager";

/**
 * @brief Constructor
 */
StateManager::StateManager(ConfigManager *configManager, MqttManager *mqttManager, ConnectionManager *connectionManager, ValveManager *valveManager) {
    state = STATE_MIN;
    configManager = configManager;
    mqttManager = mqttManager;
    connectionManager = connectionManager;
    valveManager = valveManager;
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
        case STATE_DISPENSE:
            dispense();
            break;
        case STATE_FLOW_CALIBRATE:
            flowCalibrate();
            break;
        case STATE_PRESSURE_CALIBRATE:
            pressureCalibrate();
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
    Config_t config = {}; 
    
    /** In case MQTT has not checked for any messages yet, wait until it has. */
    if (mqttManager->numMessagesInQueue() == 0) {
        return;
    }

    /** Retrieve config. */
    err = configManager->getConfig(config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to retrieve device config.");
        return;
    }
    
    /** Check for new MQTT messages. */
    while(mqttManager->numMessagesInQueue() > 0) {

        /** Get the next message from the queue. */
        err = mqttManager->getNextMessage(message);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Failed to retrieve MQTT message.");
            return;
        }
        if (message == nullptr) {
            ESP_LOGW(TAG, "Non-zero queue count returned null reference.");
            break;
        }
        
        /** Handle message. */
        switch (message->messageCode) {
            case MQTT_RX_DISPENSE_ACTIVATE:
                handleDispenseRequest(message)
                break;
        
            case MQTT_RX_RESTART:
                state = STATE_RESTART;
                break;
        
            case MQTT_RX_CHANGE_CONFIG:
                // Handle change config
                break;
        
            case MQTT_RX_FLOW_CALIBRATE
                // Handle flow calibrate begin
                break;

            case MQTT_RX_PRESSURE_CALIBRATE
                // Handle pressure calibrate begin
                break;
        
            case MQTT_RX_DRAIN:
                // Handle drain
                break;
        
            case MQTT_RX_PRESSURE_POLL:
                // Handle pressure poll
                break;
        
            default:
                // Handle unknown message type
                break;

        }
        
    }

}

/**
 * @brief Handler for state STATE_DISPENSE.
 */
void StateManager::dispense() {
    // Placeholder for dispense start state logic
}

/**
 * @brief Handler for state STATE_FLOW_CALIBRATE.
 */
void StateManager::flowCalibrate() {
    // Placeholder for flow calibrate dispense state logic
}

/**
 * @brief Handler for state STATE_PRESSURE_CALIBRATE.
 */
void StateManager::pressureCalibrate() {
    // Placeholder for pressure calibrate state logic
}

/**
 * @brief Handler for state STATE_DRAIN.
 */
void StateManager::drain() {
    // Placeholder for drain state logic
}

/**
 * @brief Handles state change for a dispense request.
 * 
 * @param message MQTT received message.
 * @return esp_err_t Return code.
 */
esp_err_t StateManager::handleDispenseRequest(MqttRxMessage_t *message) {
    MqttRxDispenseActivateMessage_t *payload = nullptr;
    DispensationStage_e dispenseStage = NOT_DISPENSING; 

    /** Reject null input. */
    if (message == nullptr) {
        ESP_LOGE(TAG, "Mqtt handler received null message.")
        return ESP_ERR_INVALID_INPUT;
    }
    
    /** Typecast the payload. */
    payload = reinterpret_cast<MqttRxDispenseActivateMessage_t*>(message->payload);

    /** Begin the dispensation process. */
    err = valveManager.beginDispensation(payload->targetVolume, payload->targetTime, payload->timeout, dispenseStage);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Valve manager failure.")
    }

    /** Handle state transition based on dispensation status. */
    switch (dispenseStage) {
        case NOT_DISPENSING:
            ESP_LOGW(TAG, "Failed to begin dispensation.")
            break;
        case DISPENSING_TANK:
            state = STATE_DISPENSE;
            break;
        case DISPENSING_SOURCE:
            state = STATE_DISPENSE;
            break;
        case CONCLUDED:
            ESP_LOGW(TAG, "Dispensation concluded immediately.")
            break;
        default:
            ESP_LOGE(TAG, "Unrecognized value of DispensationStage_e.")
            break;
    }

    return ESP_OK;
}