#include "cstdio"

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
            mqttManager->txError(TAG, "State machine set to invalid state.");
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
        mqttManager->txWarning(TAG, "Failed to transmit reset notification.");
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
        mqttManager->txError(TAG, "Failed to retrieve device config.");
        return;
    }
    
    /** Check for new MQTT messages. */
    while(mqttManager->numMessagesInQueue() > 0) {

        /** Get the next message from the queue. */
        err = mqttManager->getNextMessage(message);
        if (err != ESP_OK) {
            mqttManager->txWarning(TAG, "Failed to retrieve MQTT message.");
            return;
        }
        if (message == nullptr) {
            mqttManager->txWarning(TAG, "Non-zero queue count returned null reference.");
            break;
        }
        
        /** Handle message. */
        switch (message->messageCode) {
            case MQTT_RX_DISPENSE_ACTIVATE:
                handleDispenseRequest(message);
                break;
        
            case MQTT_RX_RESTART:
                state = STATE_RESTART;
                break;

            case MQTT_RX_CHANGE_CONFIG:
                handleConfigChangeRequest(message);
                break;
        
            case MQTT_RX_FLOW_CALIBRATE;
                handleFlowCalibrateRequest(message);
                break;

            case MQTT_RX_PRESSURE_CALIBRATE;
                handlePressureCalibrateRequest(message);
                break;
        
            case MQTT_RX_DRAIN:
                handleDrainRequest(message);
                break;
        
            case MQTT_RX_PRESSURE_POLL:
                handlePressurePollRequest(message);
                break;
        
            default:
                mqttManager->txWarning(TAG, "Message not valid in idle state.");
                break;

        }
        
    }

}

/**
 * @brief Handler for state STATE_DISPENSE.
 */
void StateManager::dispense() {
    esp_err_t err = ESP_OK;
    MqttRxMessage_t* message = nullptr;
    ValveStates_e valveState = VALVES_UNKNOWN;
    DispenseProcess_t dispenseProcess = {};
    DispenseSummary_t dispenseSummary = {};

    /** Check for new MQTT messages. */
    while(mqttManager->numMessagesInQueue() > 0) {

        /** Get the next message from the queue. */
        err = mqttManager->getNextMessage(message);
        if (err != ESP_OK) {
            mqttManager->txWarning(TAG, "Failed to retrieve MQTT message.");
            return;
        }
        if (message == nullptr) {
            mqttManager->txWarning(TAG, "Non-zero queue count returned null reference.");
            break;
        }
        
        /** Handle message. */
        switch (message->messageCode) {

            /** Handle deactivation. */
            case MQTT_RX_DEACTIVATE:
                goto exit;
                break;
        
            default:
                mqttManager->txWarning(TAG, "Only DEACTIVATE commands are accepted during dispensation.")
                break;

        }
        
    }

    /** Update dispense state. */
    err = valveManager->loopDispensation(valveState, dispenseProcess, dispenseSummary);
    if (err != ESP_OK) {
        mqttManager->txError(TAG, "Error detected. Ending dispense process.");
        goto exit;
    }
    
    /** Handle state transition based on dispensation status. */
    switch (valveState) {

        /** Error state. */
        default:
        case VALVES_UNKNOWN:
        case VALVES_TANK_DRAIN:
            mqttManager->txError(TAG, "ValveManager in an invalid state.");
            goto exit;
            break;
        
        /** Continuing to dispense. */
        case VALVES_TANK_DISPENSE:
        case VALVES_SOURCE_DISPENSE:
            
            err = mqttManager->txDispenseSlice(dispenseProcess);
            if (err != ESP_OK) {
                mqttManager->txError(TAG, "Failed to transmit dispense slice.");
            }
            break;
            
        /** Dispense has concluded. */
        case VALVES_IDLE:
            goto exit;
            break;
    }

exit:
    /** End the process. */
    err = valveManager->endDispenstaion(valveState, dispenseProcess, dispenseSummary);
    if ( (err != ESP_OK) || (valveState != VALVES_IDLE) ) {
        mqttManager->txError(TAG, "Failed to deactivate dispensation.");
    }
    
    /** Report the final variables, or none if none has been collected. */
    err = mqttManager->txDispenseSlice(dispenseProcess);
    if (err != ESP_OK) {
        mqttManager->txError(TAG, "Failed to transmit dispense slice.");
    }
    err = mqttManager->txDispenseSummary(dispenseSummary);
    if (err != ESP_OK) {
        mqttManager->txError(TAG, "Failed to transmit dispense summary.");
    }

    mqttManager->txInfo(TAG, "Concluded dispense process.");
    state = STATE_LISTEN;
    return;
}

/**
 * @brief Handler for state STATE_FLOW_CALIBRATE.
 */
void StateManager::flowCalibrate() {
    esp_err_t err = ESP_OK;
    MqttRxMessage_t* message = nullptr;
    MqttRxFlowCalibrate_t *calibrateMessagePayload = nullptr;
    FlowSensorStates_e flowState = FLOW_SENSOR_UNKNOWN;
    FlowCalibrateProcess_t calibrationProcess = {};
    FlowCalibrateSummary_t calibrationSummary = {};
    bool saveConfig = false;

    /** Check for new MQTT messages. */
    while(mqttManager->numMessagesInQueue() > 0) {

        /** Get the next message from the queue. */
        err = mqttManager->getNextMessage(message);
        if (err != ESP_OK) {
            mqttManager->txWarning(TAG, "Failed to retrieve MQTT message.");
            return;
        }
        if (message == nullptr) {
            mqttManager->txWarning(TAG, "Non-zero queue count returned null reference.");
            break;
        }
        
        /** Handle message. */
        switch (message->messageCode) {

            /** Handle deactivation. */
            case MQTT_RX_DEACTIVATE:
                goto exit;
                break;

            case MQTT_RX_FLOW_CALIBRATE:
                calibrateMessagePayload = reinterpret_cast<MqttRxFlowCalibrate_t*>(message->payload);
                break;
        
            default:
                mqttManager->txWarning(TAG, "Only DEACTIVATE and FLOW_CALIBRATE commands are accepted during flow sensor calibration.")
                break;

        }
        
    }

    /** Update calibration state. */
    err = flowManager->loopCalibration(flowState, calibrationProcess, calibrationSummary);
    if (err != ESP_OK) {
        mqttManager->txError(TAG, "Error detected. Ending calibration process.");
        goto exit;
    }

    /** Process calibration message. */
    if (calibrateMessagePayload != nullptr) {
        err = flowManager->inputCalibration(flowState, calibrateMessagePayload->targetVolume, calibrateMessagePayload->conclude);
        if (err != ESP_OK) {
            mqttManager->txError(TAG, "Error detected. Ending calibration process.");
            goto exit;
        }
    } 
    
    /** Handle state transition based on dispensation status. */
    switch (flowState) {

        /** Error state. */
        default:
        case FLOW_SENSOR_UNKNOWN:
        case FLOW_SENSOR_DISPENSING:
            mqttManager->txError(TAG, "FlowManager in an invalid state.");
            goto exit;
            break;
        
        /** Continuing to dispense. */
        case FLOW_SENSOR_CALIBRATION_DISPENSING:
            break

        /** Waiting for measurement. */
        case FLOW_SENSOR_CALIBRATION_WAITING_FOR_MEASUREMENT:
            break;
            
        /** Calibration has concluded. */
        case FLOW_SENSOR_IDLE:
            goto exit;
            break;
    }

exit:
    /** End the process. */
    err = flowManager->endCalibration(flowState, calibrationProcess, calibrationSummary);
    if ( (err != ESP_OK) || (flowState != FLOW_SENSOR_IDLE) ) {
        mqttManager->txError(TAG, "Failed to deactivate dispensation.");
    }

    if (saveConfig) {
        mqttManager->txInfo(TAG, "Saved flow sensor calibration data:...");
    }

    mqttManager->txInfo(TAG, "Concluded calibration process.");
    state = STATE_LISTEN;
    return;
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
    esp_err_t err = ESP_OK;
    char message[128];
    MqttRxDispenseActivateMessage_t *payload = nullptr;
    ValveStates_e valveState = VALVES_UNKNOWN; 
    DispenseProcess_t dispenseProcess = {};

    /** Reject null input. */
    if (message == nullptr) {
        mqttManager->txError(TAG, "Mqtt handler received null message.");
        return ESP_ERR_INVALID_INPUT;
    }
    
    /** Typecast the payload. */
    payload = reinterpret_cast<MqttRxDispenseActivateMessage_t*>(message->payload);

    /** Begin the dispensation process. */
    err = valveManager->beginDispensation(payload*, valveState, dispenseProcess);
    if (err != ESP_OK) {
        mqttManager->txError(TAG, "Valve manager failure.");
    }

    /** Handle state transition based on dispensation status. */
    switch (valveState) {
        case VALVES_UNKNOWN:
        case VALVES_IDLE:
        case VALVES_TANK_DRAIN:
            mqttManager->txError(TAG, "Failed to begin dispensation.");
            break;

        case VALVES_TANK_DISPENSE:
        case VALVES_SOURCE_DISPENSE:

            snprintf(message, 
                sizeof(message), 
                "Beginning dispense process with a target volume: %.2f liters, time: %d min, timeout: %d min", 
                payload->targetVolume, 
                payload->targetTime / 1000, 
                payload->timeout / 1000
            );
            mqttManager->txInfo(TAG, message);
            state = STATE_DISPENSE;
            break;

        default:
            mqttManager->txError(TAG, "Unrecognized value of ValveStates_e.");
            break;
    }

    return ESP_OK;
}


/**
 * @brief Handles state change for a config change request.
 * 
 * @param message MQTT received message.
 * @return esp_err_t Return code.
 */
esp_err_t handleConfigChangeRequest(MqttRxMessage_t *message) {
    return ESP_OK;
}

/**
 * @brief Handles state change for a flow calibrate request.
 * 
 * @param message MQTT received message.
 * @return esp_err_t Return code.
 */
esp_err_t handleFlowCalibrateRequest(MqttRxMessage_t *message) {
    esp_err_t err = ESP_OK;
    char message[128];
    MqttRxFlowCalibrate_t *payload = nullptr;
    FlowSensorStates_e flowState = FLOW_SENSOR_UNKNOWN; 
    FlowCalibrateProcess_t calibrateProcess = {};

    /** Reject null input. */
    if (message == nullptr) {
        mqttManager->txError(TAG, "Mqtt handler received null message.");
        return ESP_ERR_INVALID_INPUT;
    }
    
    /** Typecast the payload. */
    payload = reinterpret_cast<MqttRxFlowCalibrate_t*>(message->payload);

    /** Begin the calibration process. */
    err = flowManager->beginCalibration(payload*, flowState, calibrateProcess);
    if (err != ESP_OK) {
        mqttManager->txError(TAG, "Flow manager failure.");
    }

    /** Handle state transition based on dispensation status. */
    switch (flowState) {
        case FLOW_SENSOR_UNKNOWN:
        case FLOW_SENSOR_IDLE:
        case FLOW_SENSOR_DISPENSING:
            mqttManager->txError(TAG, "Failed to begin dispensation.");
            break;

        case SENSOR_CALIBRATING:

            snprintf(message, 
                sizeof(message), 
                "Beginning calibration process with a target volume: %.2f liters, timeout: %d min", 
                payload->targetVolume, 
                payload->timeout / (1000 * 60)
            );
            mqttManager->txInfo(TAG, message);
            state = STATE_FLOW_CALIBRATE;
            break;

        default:
            mqttManager->txError(TAG, "Unrecognized value of FlowSensorStates_e.");
            break;
    }

    return ESP_OK;
}

/**
 * @brief Handles state change for a pressure calibrate request.
 * 
 * @param message MQTT received message.
 * @return esp_err_t Return code.
 */
esp_err_t handlePressureCalibrateRequest(MqttRxMessage_t *message) {
    return ESP_OK;
}

/**
 * @brief Handles state change for a drain request.
 * 
 * @param message MQTT received message.
 * @return esp_err_t Return code.
 */
esp_err_t handleDrainRequest(MqttRxMessage_t *message) {
    return ESP_OK
}

/**
 * @brief Handles state change for a pressure poll request.
 * 
 * @param message MQTT received message.
 * @return esp_err_t Return code.
 */
esp_err_t handlePressurePollRequest(MqttRxMessage_t *message) {
    return ESP_OK
}