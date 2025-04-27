#include "esp_err.h"

#include "mqttManager.h"

static const char* TAG = "MqttManager";

/**
 * @brief Constructor.
 */
MqttManager::MqttManager() {
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

/**
 * @brief Get the checkedForMessages flagged.
 * 
 * @returns True if the first scan for messages has happened. 
 */
bool MqttManager::checkedForMessages() {
    return true;
}

/**
 * @brief Returns the number of received MQTT messages in
 * the queue.
 */
uint8_t MqttManager::numMessagesInQueue() {
    return 0;
}

/**
 * @brief Pull the next incoming MQTT message from the queue.
 * 
 * @param Pointer to the message object.
 * @return esp_err_t Return code.
 */
esp_err_t MqttManager::getNextMessage(MqttRxMessage_t* message) {
    return ESP_OK;
}

/**
 * @brief Transmit an info log.
 * 
 * @param Log message.
 * @return esp_err_t Return code.
 */
esp_err_t info(char *message[]) {
    return ESP_OK;
}
/**
 * @brief Transmit a warning log.
 * 
 * @param Log message.
 * @return esp_err_t Return code.
 */
esp_err_t warning(char *message) {
    return ESP_OK;
}

/**
 * @brief Transmit an error log.
 * 
 * @param Log message.
 * @return esp_err_t Return code.
 */
esp_err_t error(char *message[]) {
    return ESP_OK;
}