#include "esp_err.h"

#include "mqttManager.h"

static const char* TAG = "MqttManager";

/**
 * @brief Constructor.
 */
MqttManager::MqttManager(DataContainer &dataContainer) {
    return;
}

/**
 * @brief Begin the MqttManager.s
 * 
 * @return esp_err_t Return code. 
 */
esp_err_t MqttManager::initialize() {
    return ESP_OK;
}

bool MqttManager::connected() const {
    return connected_;
}


bool MqttManager::checkedForMessages() {
    return true;
}

uint8_t MqttManager::numMessagesInQueue() {
    return 0;
}

esp_err_t MqttManager::connect() {
    return ESP_OK;
}

/**
 * @brief Pull the next incoming MQTT message from the queue.
 * 
 * @param message Output parameter for the message object.
 * @return esp_err_t Return code.
 */
esp_err_t MqttManager::getNextMessage(DripRxMessage<DripRxMessageId_e> *&message) {
    return ESP_OK;
}

esp_err_t MqttManager::freeMessage(DripRxMessage<DripRxMessageId_e> *&message) {
    return ESP_OK;
}


esp_err_t MqttManager::uploadLogs() {
    return ESP_OK;
}

/**
 * @brief Transmits a time slice of the dispense process realtime variables.
 * 
 * @return esp_err_t Return code.
 */
esp_err_t MqttManager::uploadDispenseSlice() {
    return ESP_OK;
}

/**
 * @brief Transmits a summary of the dispense process variables.
 * 
 * @return esp_err_t Return code. 
 */
esp_err_t MqttManager::uploadDispenseSummary() {
    return ESP_OK;
}