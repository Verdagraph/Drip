#include "message.h"

/**
 * @defgroup DripRxMessage Incoming Message Object
 */

DripRxMessage::DripRxMessage(DripRxMessageId_e id) : id_(id), isValid_(false) {}

DripRxMessageId_e DripRxMessage::id() const {
    return id_;
}

bool DripRxMessage::isValid() const {
    return isValid_;
}

esp_err_t DripRxMessage::structure(
    uint8_t *payload, 
    size_t payloadLen, 
    const DataContainer &container, 
    const DripConfig_t &config
) {
    esp_err_t err = ESP_OK;

    if ( (payload == nullptr) || (payloadLen > DRIP_MSG_MAX_PAYLOAD_BYTES) ) {
        return ESP_ERR_INVALID_ARG;
    }
    
    err = fromJson(payload, payloadLen);
    if (err != ESP_OK) {
        return ESP_FAIL;
    }

    err = validate(container, config);
    if (err != ESP_OK) {
        return ESP_FAIL;
    }

    isValid_ = true;

    return ESP_OK;
}

/** @} */


/**
 * @defgroup DripTxMessageContainer Outgoing Message Object
 */

template <typename TData>
DripTxMessageContainer<TData>::DripTxMessageContainer(DripTxMessageId_e id) : id(id), payload_{}, payloadLen_(0), isValid_(false) {}

template <typename TData>
bool DripTxMessageContainer<TData>::isValid() const {
    return isValid_;
}

template <typename TData>
esp_err_t DripTxMessageContainer<TData>::copyPayload(const uint8_t *buf, size_t bufLen) {
    if (isValid_ == false) {
        return ESP_ERR_INVALID_STATE;
    }
    if ( (buf == nullptr) || (bufLen < payloadLen_) ) {
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(buf, payload_, payloadLen_);

    return ESP_OK;
}

template <typename TData>
esp_err_t DripTxMessageContainer<TData>::unstructure(const TData data) {
    esp_err_t err = ESP_OK;
    
    err = toJson(data);
    if (err != ESP_OK) {
        return ESP_FAIL;
    } 

    isValid_ = true;

    return ESP_OK;
}

/** @} */
