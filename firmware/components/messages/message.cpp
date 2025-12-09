#include "message.h"

/**
 * @defgroup DripRxMessage Incoming Message Object
 */
template <typename TRxMessageId_e>
DripRxMessage<TRxMessageId_e>::DripRxMessage(TRxMessageId_e id) : id_(id), isValid_(false) {}

template <typename TRxMessageId_e>
TRxMessageId_e DripRxMessage<TRxMessageId_e>::id() const {
    return id_;
}

template <typename TRxMessageId_e>
bool DripRxMessage<TRxMessageId_e>::isValid() const {
    return isValid_;
}

template <typename TRxMessageId_e>
esp_err_t DripRxMessage<TRxMessageId_e>::structure(
    uint8_t *payload, 
    size_t payloadLen, 
    const DataContainer &container
) {
    esp_err_t err = ESP_OK;

    if ( (payload == nullptr) || (payloadLen > DRIP_MSG_MAX_PAYLOAD_BYTES) ) {
        return ESP_ERR_INVALID_ARG;
    }
    
    err = fromJson(payload, payloadLen);
    if (err != ESP_OK) {
        return ESP_FAIL;
    }

    err = validate(container);
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

template <typename TTxMessageId_e, typename TData>
DripTxMessageContainer<TTxMessageId_e, TData>::DripTxMessageContainer(TTxMessageId_e id) : id(id), payload_{}, payloadLen_(0), isValid_(false) {}

template <typename TData, typename TTxMessageId_e>
bool DripTxMessageContainer<TData, TTxMessageId_e>::isValid() const {
    return isValid_;
}

template <typename TTxMessageId_e, typename TData>
esp_err_t DripTxMessageContainer<TTxMessageId_e, TData>::copyPayload(const uint8_t *buf, size_t bufLen) {
    if (isValid_ == false) {
        return ESP_ERR_INVALID_STATE;
    }
    if ( (buf == nullptr) || (bufLen < payloadLen_) ) {
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(buf, payload_, payloadLen_);

    return ESP_OK;
}

template <typename TTxMessageId_e, typename TData>
esp_err_t DripTxMessageContainer<TTxMessageId_e, TData>::unstructure(const TData data, const DataContainer &container) {
    esp_err_t err = ESP_OK;
    
    err = toJson(data, container);
    if (err != ESP_OK) {
        return ESP_FAIL;
    } 

    isValid_ = true;

    return ESP_OK;
}

/** @} */
