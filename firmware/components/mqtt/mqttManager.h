#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include "messages.h"
#include "valveManager.h"

#define RX_PAYLOAD_MAX_BYTES 512

/**
 * @brief Handles transmitting and receiving MQTT messages.
 */
class MqttManager {
public:
    /**
     * @brief Constructor.
     */
    MqttManager();

    /**
     * @brief Begin the MqttManager.s
     * 
     * @return esp_err_t Return code. 
     */
    esp_err_t initialize();

    /**
     * @brief Get the checkedForMessages flagged.
     * 
     * @returns True if the first scan for messages has happened. 
     */
    bool checkedForMessages();

    /**
     * @brief Returns the number of received MQTT messages in
     * the queue.
     */
    uint8_t numMessagesInQueue();
    
    /**
     * @brief Pull the next incoming MQTT message from the queue.
     * 
     * @param Pointer to the message object.
     * @return esp_err_t Return code.
     */
    esp_err_t getNextMessage(MqttRxMessage_t* message);

    /**
     * @brief Transmit an info log.
     * 
     * @param Log message.
     * @return esp_err_t Return code.
     */
    esp_err_t txInfo(const char* tag, const char *message);

    /**
     * @brief Transmit a warning log.
     * 
     * @param Log message.
     * @return esp_err_t Return code.
     */
    esp_err_t txWarning(const char* tag, const char *message);

    /**
     * @brief Transmit an error log.
     * 
     * @param Log message.
     * @return esp_err_t Return code.
     */
    esp_err_t txError(const char* tag, const char *message);

    /**
     * @brief Transmits a time slice of the dispense process realtime variables.
     * 
     * @param slice The variables.
     * @return esp_err_t Return code.
     */
    esp_err_t txDispenseSlice(DispenseProcess_t &slice);

    /**
     * @brief Transmits a summary of the dispense process variables.
     * 
     * @param summary The variables.
     * @return esp_err_t Return code. 
     */
    esp_err_t txDispenseSummary(DispenseSummary_t &summary);

    
private:
    /** If true, the manager has checked for messages at least once. */
    bool _checkedForMessages;
    MqttRxMessage_t rxMessage;
    char rxPayload[RX_PAYLOAD_MAX_BYTES]; 

};

#endif