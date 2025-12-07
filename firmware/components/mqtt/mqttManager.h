#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include "message.h"
#include "valveManager.h"

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
     * @brief Begin the MqttManager.
     * 
     * @return esp_err_t Return code. 
     */
    esp_err_t initialize();

    bool connected() const;

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
     * @brief Begins an attempt to connect to MQTT.
     * Reinitializes any exising connection.
     * 
     * @return esp_err_t 
     */
    esp_err_t connect();
    
    /**
     * @brief Pull the next incoming MQTT message from the queue.
     * 
     * @param message Output parameter for the message object.
     * @return esp_err_t Return code.
     */
    esp_err_t getNextMessage(DripRxMessage *&message);

    esp_err_t freeMessage(DripRxMessage *&message);

    esp_err_t uploadLogs();

    /**
     * @brief Transmits a time slice of the dispense process realtime variables.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t uploadDispenseSlice();

    /**
     * @brief Transmits a summary of the dispense process variables.
     * 
     * @return esp_err_t Return code. 
     */
    esp_err_t uploadDispenseSummary();

    
private:
    bool connected_;
    /** If true, the manager has checked for messages at least once. */
    bool checkedForMessages_;
    //MqttRxMessage_t rxMessage;
    //char rxPayload[RX_PAYLOAD_MAX_BYTES]; 

};

#endif