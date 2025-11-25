#ifndef MESSAGE_H
#define MESSAGE_H

#include "config.h"
#include "dataContainer.h"
#include "valveManager.h"


/** @brief The maximum supported size for the message payload. */
constexpr size_t DRIP_MSG_MAX_PAYLOAD_BYTES = 512U;

/**
 * @brief Specifies data structure and conversion to a JSON payload
 * for a structured application message data object.
 * 
 * @tparam TData The type of struct represented. 
 */
template <typename TData>
class DripRxMessageContainer {
public:

    DripRxMessageContainer();

    ~DripRxMessageContainer() = default;

    /**
     * @brief Retrieve the structured data.
     * 
     * @warning The isValid flag should be checked before using this.
     * 
     * @return TData The structured data.
     */
    TData data() const;
    
    /**
     * @brief Get the isValid flag.
     * 
     * @return true If the message contains valid structured data.
     */
    bool isValid() const;

    /**
     * @brief Unpack the JSON data into the structured
     * form and validate.
     * 
     * @param payload The unstructured data to set. 
     * @param payloadLen The length of payload.
     * @param container DataContainer context.
     * @param config Config context.
     * 
     * @return ESP_OK If the data was stored and validated successfully.
     * @return ESP_ERR_INVALID_ARG If payload is null or
     * newPayloadLen is greater than the maximum supported length.
     * @return ESP_FAIL If parsing or validation fails.
     */
    esp_err_t structure(
        uint8_t *payload, 
        size_t payloadLen, 
        const DataContainer &container, 
        const DripConfig_t &config
    );

    /**
     * @brief Unpack the JSON data into the structured form.
     * 
     * @param payload The unstructured data to set. 
     * @param payloadLen The length of payload.
     * 
     * @return ESP_OK If the data was stored successfully.
     * @return ESP_ERR_INVALID_ARG If payload is null or
     * newPayloadLen is greater than the maximum supported length.
     * @return ESP_FAIL If parsing fails.
     */
    virtual esp_err_t fromJson(uint8_t *payload, size_t payloadLen) = 0;

    /**
     * @brief Validate the structured data.
     * 
     * @param container DataContainer context.
     * @param config Config context.
     * @return ESP_FAIL If validation fails.
     * 
     * @return ESP_OK If the struct is valid. 
     */
    virtual esp_err_t validate(const DataContainer &container, const DripConfig_t &config) = 0;

private:
    /** @brief The structured data. */
    TData data_;
    /** @brief True if the struct data has been populated and validated. */
    bool isValid_;
};


/**
 * @brief Specifies data structure and conversion from a JSON payload
 * for a structured application message data object.
 * 
 * @tparam TData The type of struct represented. 
 */
template <typename TData>
class DripTxMessageContainer {
public:

    DripTxMessageContainer();

    ~DripTxMessageContainer() = default;
    
    /**
     * @brief Get the isValid flag.
     * 
     * @return true If the message contains valid unstructured JSON data.
     */
    bool isValid() const;
    
    /**
     * @brief Get the unstructured message JSON payload.
     * 
     * @param buf The buffer to copy the payload into.
     * @param bufLen The length of buf.
     * 
     * @return ESP_OK If the data was output successfully.
     * @return ESP_ERR_INVALID_STATE If the data was not previously unstructured.
     * @return ESP_ERR_INVALID_ARG If buf is null or bufLen is less than the
     * length of the unstructured data.
     */
    esp_err_t copyPayload(const uint8_t *buf, size_t bufLen);

    /**
     * @brief Packs the struct data into the unstructured
     * JSON form.
     * 
     * @param data The data to pack.
     * 
     * @return ESP_OK If the data was packed successfully.
     * @return ESP_FAIL If a failure occurs while packing.  
     */
    esp_err_t unstructure(const TData data);

    /**
     * @brief Dumps the struct data to JSON.
     * 
     * @param data The data to contvert.
     * 
     * @return ESP_OK If the data was packed successfully.
     * @return ESP_FAIL If a failure occurs while packing.  
     */
    virtual esp_err_t toJson(const TData data) = 0;

private:
    /** @brief A pointer to the unstructured JSON payload. */
    uint8_t payload_[DRIP_MSG_MAX_PAYLOAD_BYTES];
    /** @brief The number of bytes in the payload. */
    size_t payloadLen_;
    /** @brief True if the payload has populated data. */
    bool isValid_;
};

#endif
