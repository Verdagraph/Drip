/**
 * @file WifiManager.h
 * @brief Declaration of the WifiManager class for handling Wi-Fi connectivity.
 * @details Borrowed from https://github.com/Ariif0/ESPIDF-WiFi-Configuration
 */

#pragma once
#include "sdk_compat.h"

#include "dataContainer.h"

/**
 * @class WifiManager
 * @brief Manages all aspects of Wi-Fi connectivity, including Station (STA), Access Point (AP), and provisioning via web server.
 */
class WifiManager {
public:
    /**
     * @brief Constructs a new WifiManager object.
     *
     * Initializes the event group for Wi-Fi event handling.
     */
    WifiManager(DataContainer &dataContainer);

    /**
     * @brief Destroys the WifiManager object.
     *
     * Cleans up resources such as the web server and event group.
     */
    ~WifiManager();

    /**
     * @brief Starts the Wi-Fi management process.
     *
     * Attempts to connect using stored credentials. If connection fails or no credentials are available,
     * it switches to provisioning mode (AP).
     */
    void start();

    /**
     * @brief Checks if the device is currently connected to Wi-Fi in Station mode.
     *
     * @return true if connected, false otherwise.
     */
    bool isConnected() const;

    /**
     * @brief Retrieves the current IP address of the device in Station mode.
     *
     * @return std::string containing the IP address, or an empty string if not connected.
     */
    std::string getIpAddress() const;

private:
    /**
     * @brief Initializes the TCP/IP stack and default event loop.
     */
    void initialize();

    /**
     * @brief Attempts to connect to a Wi-Fi Access Point with the provided credentials.
     *
     * @param ssid The SSID of the Wi-Fi network.
     * @param password The password of the Wi-Fi network.
     * @return esp_err_t ESP_OK if connection attempt is successful, ESP_FAIL otherwise.
     */
    esp_err_t connectToWifi(const std::string& ssid, const std::string& password);

    /**
     * @brief Starts Access Point mode for provisioning.
     *
     * Runs a simple web server to receive new Wi-Fi credentials.
     */
    void startProvisioning();

    /**
     * @brief Stops Wi-Fi in AP and/or STA mode.
     */
    void stopWifi();

    /**
     * @brief Starts the HTTP web server.
     *
     * @param is_provisioning_mode True if the server is for AP provisioning mode, false for STA mode (reset endpoint only).
     */
    void startWebServer(bool is_provisioning_mode);

    /**
     * @brief Stops the HTTP web server.
     */
    void stopWebServer();

    /**
     * @brief Static event handler for Wi-Fi events.
     *
     * @param arg Pointer to the WifiManager instance.
     * @param event_base Event base identifier.
     * @param event_id Event ID.
     * @param event_data Event-specific data.
     */
    static void eventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

    /**
     * @brief HTTP GET handler for provisioning page.
     *
     * @param req HTTP request handle.
     * @return esp_err_t ESP_OK on success, error code otherwise.
     */
    static esp_err_t provisioningGetHandler(httpd_req_t *req);

    /**
     * @brief HTTP POST handler for receiving Wi-Fi credentials.
     *
     * @param req HTTP request handle.
     * @return esp_err_t ESP_OK on success, error code otherwise.
     */
    static esp_err_t connectPostHandler(httpd_req_t *req);

    /**
     * @brief HTTP GET handler for reset endpoint.
     *
     * @param req HTTP request handle.
     * @return esp_err_t ESP_OK on success, error code otherwise.
     */
    static esp_err_t resetGetHandler(httpd_req_t* req);

    /**
     * @brief HTTP GET handler for favicon.
     *
     * @param req HTTP request handle.
     * @return esp_err_t ESP_OK on success, error code otherwise.
     */
    static esp_err_t faviconGetHandler(httpd_req_t *req);

    /**
     * @brief Saves Wi-Fi credentials to Non-Volatile Storage (NVS).
     *
     * @param ssid The SSID to save.
     * @param password The password to save.
     * @return esp_err_t ESP_OK on success, error code otherwise.
     */
    esp_err_t saveCredentials(const std::string& ssid, const std::string& password);

    /**
     * @brief Loads Wi-Fi credentials from Non-Volatile Storage (NVS).
     *
     * @param ssid Output parameter for the loaded SSID.
     * @param password Output parameter for the loaded password.
     * @return esp_err_t ESP_OK on success, error code otherwise.
     */
    esp_err_t loadCredentials(std::string& ssid, std::string& password);

    /**
     * @brief Clears stored credentials and restarts the device.
     */
    void clearCredentialsAndRestart();

    /** @brief Event group handle for managing Wi-Fi connection events (connected/failed). */
    EventGroupHandle_t m_wifi_event_group;

    /** @brief Handle for the HTTP web server. */
    httpd_handle_t m_server;

    /** @brief Current connection status. */
    bool m_is_connected;

    /** @brief Number of reconnection attempts. */
    uint8_t m_retry_num;

    /** @brief Current IP address in Station mode. */
    std::string m_current_ip;

    /** @brief Maximum number of reconnection attempts before giving up. */
    static constexpr uint8_t MAX_RETRY = 5;
};